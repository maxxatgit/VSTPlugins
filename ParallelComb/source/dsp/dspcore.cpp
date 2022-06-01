// (c) 2022 Takamitsu Endo
//
// This file is part of ParallelComb.
//
// ParallelComb is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ParallelComb is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ParallelComb.  If not, see <https://www.gnu.org/licenses/>.

#include "dspcore.hpp"

#include <algorithm>
#include <limits>
#include <numeric>

#if INSTRSET >= 10
#define DSPCORE_NAME DSPCore_AVX512
#elif INSTRSET >= 8
#define DSPCORE_NAME DSPCore_AVX2
#elif INSTRSET >= 7
#define DSPCORE_NAME DSPCore_AVX
#else
#error Unsupported instruction set
#endif

template<typename T> T lerp(T a, T b, T t) { return a + t * (b - a); }

void DSPCORE_NAME::setup(double sampleRate)
{
  this->sampleRate = float(sampleRate);
  auto upRate = float(sampleRate) * OverSampler::fold;

  SmootherCommon<float>::setSampleRate(upRate);
  SmootherCommon<float>::setTime(0.2f);

  for (auto &cmb : comb) cmb.setup(upRate, maxDelayTime);

  reset();
  startup();
}

size_t DSPCORE_NAME::getLatency() { return 0; }

#define ASSIGN_PARAMETER(METHOD)                                                         \
  using ID = ParameterID::ID;                                                            \
  const auto &pv = param.value;                                                          \
                                                                                         \
  auto upfold = pv[ID::overSampling]->getInt() ? OverSampler::fold : float(1);           \
  auto upRate = upfold * sampleRate;                                                     \
                                                                                         \
  auto stLean = pv[ID::stereoLean]->getFloat();                                          \
  auto lean0 = stLean < 0 ? float(1) + stLean : float(1);                                \
  auto lean1 = stLean < 0 ? float(1) : float(1) - stLean;                                \
  auto timeMul = pv[ID::timeMultiplier]->getFloat();                                     \
  for (size_t idx = 0; idx < nCombTaps; ++idx) {                                         \
    auto time = timeMul * upRate * pv[ID::delayTime0 + idx]->getFloat();                 \
    comb[0].time.METHOD##At(idx, time *lean0);                                           \
    comb[1].time.METHOD##At(idx, time *lean1);                                           \
  }                                                                                      \
  interpCombInterpRate.METHOD(pv[ID::delayTimeInterpRate]->getFloat());                  \
  auto combLowpassSeconds = pv[ID::delayTimeInterpLowpassSeconds]->getFloat();           \
  interpCombInterpCutoffKp.METHOD(                                                       \
    combLowpassSeconds == 0                                                              \
      ? float(1)                                                                         \
      : float(EMAFilter<double>::cutoffToP(upRate, float(1) / combLowpassSeconds)));     \
  interpFeedback.METHOD(pv[ID::feedback]->getFloat());                                   \
  interpFeedbackHighpassCutoffKp.METHOD(float(EMAFilter<double>::cutoffToP(              \
    upRate, pv[ID::feedbackHighpassCutoffHz]->getFloat())));                             \
  interpStereoCross.METHOD(pv[ID::stereoCross]->getFloat());                             \
  interpFeedbackToDelayTime.METHOD(pv[ID::feedbackToDelayTime]->getFloat());             \
  interpDry.METHOD(pv[ID::dry]->getFloat());                                             \
  interpWet.METHOD(pv[ID::wet]->getFloat());

void DSPCORE_NAME::reset()
{
  ASSIGN_PARAMETER(reset);

  delayOut.fill(0);

  for (auto &os : overSampler) os.reset();
  for (auto &cmb : comb) cmb.reset();
  for (auto &hp : feedbackHighpass) hp.reset();
  for (auto &lm : feedbackLimiter) lm.reset();

  startup();
}

void DSPCORE_NAME::startup() {}

void DSPCORE_NAME::setParameters()
{
  ASSIGN_PARAMETER(push);

  for (auto &lm : feedbackLimiter) {
    lm.prepare(upRate, pv[ID::feedbackLimiterRelease]->getFloat(), 1.0f);
  }
}

inline void convertToMidSide(float &left, float &right)
{
  auto mid = left + right;
  auto side = left - right;
  left = mid;
  right = side;
}

inline void convertToLeftRight(float &mid, float &side)
{
  auto left = (mid + side);
  auto right = (mid - side);
  mid = left;
  side = right;
}

std::array<float, 2> DSPCORE_NAME::processInternal(float ch0, float ch1)
{
  auto combRate = interpCombInterpRate.process();
  auto combCutoffKp = interpCombInterpCutoffKp.process();
  auto feedback = interpFeedback.process();
  auto feedbackHighpassKp = interpFeedbackHighpassCutoffKp.process();
  auto stereoCross = interpStereoCross.process();
  auto toDelayTime = interpFeedbackToDelayTime.process();
  auto dry = interpDry.process();
  auto wet = interpWet.process();

  auto cross0 = lerp(delayOut[0], delayOut[1], stereoCross);
  auto cross1 = lerp(delayOut[1], delayOut[0], stereoCross);

  auto combOut0 = comb[0].process(
    ch0 + feedback * cross0, combRate + toDelayTime * cross0, combCutoffKp);
  auto combOut1 = comb[1].process(
    ch1 + feedback * cross1, combRate + toDelayTime * cross1, combCutoffKp);

  delayOut[0] = feedbackLimiter[0].process(
    feedbackHighpass[0].process(combOut0, feedbackHighpassKp));
  delayOut[1] = feedbackLimiter[1].process(
    feedbackHighpass[1].process(combOut1, feedbackHighpassKp));

  return {dry * ch0 + wet * delayOut[0], dry * ch1 + wet * delayOut[1]};
}

void DSPCORE_NAME::process(
  const size_t length, const float *in0, const float *in1, float *out0, float *out1)
{
  using ID = ParameterID::ID;
  const auto &pv = param.value;

  bool overSampling = pv[ID::overSampling]->getInt();
  float upfold = overSampling ? OverSampler::fold : float(1);
  float upRate = upfold * sampleRate;

  SmootherCommon<float>::setBufferSize(float(length));
  SmootherCommon<float>::setSampleRate(upRate);

  bool enableMidSide = pv[ID::channelType]->getInt();

  if (overSampling) {
    for (size_t i = 0; i < length; ++i) {
      float sig0 = in0[i];
      float sig1 = in1[i];
      if (enableMidSide) convertToMidSide(sig0, sig1);

      overSampler[0].push(sig0);
      overSampler[1].push(sig1);
      for (size_t idx = 0; idx < OverSampler::fold; ++idx) {
        auto buf = processInternal(overSampler[0].at(idx), overSampler[1].at(idx));
        overSampler[0].inputBuffer[idx] = buf[0];
        overSampler[1].inputBuffer[idx] = buf[1];
      }
      sig0 = overSampler[0].process();
      sig1 = overSampler[1].process();
      if (enableMidSide) convertToLeftRight(sig0, sig1);

      out0[i] = sig0;
      out1[i] = sig1;
    }
  } else {
    for (size_t i = 0; i < length; ++i) {
      auto buf = processInternal(in0[i], in1[i]);
      out0[i] = buf[0];
      out1[i] = buf[1];
    }
  }
}

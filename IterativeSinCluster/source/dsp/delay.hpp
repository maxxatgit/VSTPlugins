// (c) 2019-2020 Takamitsu Endo
//
// This file is part of IterativeSinCluster.
//
// IterativeSinCluster is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// IterativeSinCluster is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with IterativeSinCluster.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/smoother.hpp"

#include <vector>

namespace SomeDSP {

// 2x oversampled, linear interpolated delay.
template<typename Sample> class Delay {
public:
  Sample sampleRate = 44100.0;
  Sample rFraction = 0.0;
  Sample w1 = 0.0;
  size_t wptr = 0;
  size_t rptr = 0;
  std::vector<Sample> buf;

  void setup(Sample sampleRate, Sample time, Sample maxTime)
  {
    this->sampleRate = 2 * sampleRate;

    auto size = size_t(this->sampleRate * maxTime);
    buf.resize(size >= INT32_MAX ? INT32_MAX : size + 1, 0.0);

    setTime(time);
    reset();
  }

  void setTime(Sample seconds)
  {
    auto timeInSample
      = std::max<Sample>(0, std::min<Sample>(sampleRate * seconds, (Sample)buf.size()));

    size_t timeInt = size_t(timeInSample);
    rFraction = timeInSample - Sample(timeInt);

    // Possible negative overflow. rptr must be in [0, buf.size()).
    rptr = wptr - timeInt;
    while (rptr >= buf.size()) rptr += buf.size();
  }

  Sample getTime()
  {
    auto diff = rptr < wptr ? wptr - rptr : wptr + buf.size() - rptr;
    return diff / sampleRate;
  }

  void reset()
  {
    w1 = 0;
    std::fill(buf.begin(), buf.end(), Sample(0));
  }

  Sample process(const Sample input)
  {
    // Write to buffer.
    buf[wptr] = input - Sample(0.5) * (input - w1);
    wptr += 1;
    if (wptr >= buf.size()) wptr -= buf.size();

    buf[wptr] = input;
    wptr += 1;
    if (wptr >= buf.size()) wptr -= buf.size();

    w1 = input;

    // Read from buffer.
    const size_t i1 = rptr;
    rptr += 1;
    if (rptr >= buf.size()) rptr -= buf.size();

    const size_t i0 = rptr;
    rptr += 1;
    if (rptr >= buf.size()) rptr -= buf.size();

    return buf[i0] - rFraction * (buf[i0] - buf[i1]);
  }
};

template<typename Sample> class Chorus {
public:
  Delay<Sample> delay;
  Sample phase = 0;
  Sample feedbackBuffer = 0;
  LinearSmoother<Sample> interpTick;
  RotarySmoother<Sample> interpPhase;
  LinearSmoother<Sample> interpFeedback;
  LinearSmoother<Sample> interpDepth;
  LinearSmoother<Sample> interpDelayTimeRange;
  LinearSmoother<Sample> interpMinDelayTime;
  EMAFilter<Sample> delayTimeLowpass;

  void setup(Sample sampleRate, Sample time, Sample maxTime)
  {
    delay.setup(sampleRate, time, maxTime);
    delayTimeLowpass.setP(0.1); // Fixed P.
    interpPhase.setRange(twopi);
  }

  // frequency can be negative.
  void setParam(
    Sample frequency,
    Sample phase,
    Sample feedback,
    Sample depth,
    Sample delayTimeRange,
    Sample minDelayTime)
  {
    interpTick.push(Sample(twopi) * frequency / delay.sampleRate);
    interpPhase.push(phase);
    interpFeedback.push(feedback);
    interpDepth.push(depth);
    interpDelayTimeRange.push(delayTimeRange);
    interpMinDelayTime.push(minDelayTime);
  }

  void reset(
    Sample frequency,
    Sample phase,
    Sample feedback,
    Sample depth,
    Sample delayTimeRange,
    Sample minDelayTime)
  {
    delay.reset();
    this->phase = 0;
    feedbackBuffer = 0;

    interpTick.reset(Sample(twopi) * frequency / delay.sampleRate);
    interpPhase.reset(phase);
    interpFeedback.reset(feedback);
    interpDepth.reset(depth);
    interpDelayTimeRange.reset(delayTimeRange);
    interpMinDelayTime.reset(minDelayTime);

    delayTimeLowpass.reset();
  }

  std::array<Sample, 2> process(Sample input)
  {
    phase += interpTick.process();
    if (phase >= Sample(twopi))
      phase -= Sample(twopi);
    else if (phase <= 0)
      phase += Sample(twopi);

    const auto phaseDelta = interpPhase.process();

    const Sample lfo = Sample(0.5) * (Sample(1) + std::sin(phase + phaseDelta));
    delay.setTime(delayTimeLowpass.process(
      interpMinDelayTime.process() + lfo * interpDelayTimeRange.process()));
    feedbackBuffer = delay.process(input + interpFeedback.process() * feedbackBuffer);

    const Sample lfoDepth
      = Sample(0.5) * (Sample(1) + std::sin(phase + phaseDelta + Sample(pi / 2)));
    const auto depth = interpDepth.process();
    return {
      feedbackBuffer * (Sample(1) - depth * lfoDepth),
      feedbackBuffer * (Sample(1) - depth * (Sample(1) - lfoDepth))};
  }
};

} // namespace SomeDSP

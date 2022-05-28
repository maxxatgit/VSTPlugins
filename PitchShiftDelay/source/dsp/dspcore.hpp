// (c) 2022 Takamitsu Endo
//
// This file is part of PitchShiftDelay.
//
// PitchShiftDelay is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// PitchShiftDelay is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with PitchShiftDelay.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/multirate.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../parameter.hpp"
#include "pitchshiftdelay.hpp"

#include <array>

using namespace SomeDSP;
using namespace Steinberg::Synth;

using OverSampler = OverSampler16<float>;

class DSPInterface {
public:
  virtual ~DSPInterface(){};

  GlobalParameter param;

  virtual void setup(double sampleRate) = 0;
  virtual void reset() = 0;   // Stop sounds.
  virtual void startup() = 0; // Reset phase, random seed etc.
  virtual size_t getLatency() = 0;
  virtual void setParameters() = 0;
  virtual void process(
    const size_t length, const float *in0, const float *in1, float *out0, float *out1)
    = 0;
};

#define DSPCORE_CLASS(INSTRSET)                                                          \
  class DSPCore_##INSTRSET final : public DSPInterface {                                 \
  public:                                                                                \
    void setup(double sampleRate) override;                                              \
    void reset() override;                                                               \
    void startup() override;                                                             \
    size_t getLatency() override;                                                        \
    void setParameters() override;                                                       \
    void process(                                                                        \
      const size_t length,                                                               \
      const float *in0,                                                                  \
      const float *in1,                                                                  \
      float *out0,                                                                       \
      float *out1) override;                                                             \
                                                                                         \
  private:                                                                               \
    float sampleRate = 44100.0f;                                                         \
                                                                                         \
    ExpSmoother<float> interpPitchMain;                                                  \
    ExpSmoother<float> interpPitchUnison;                                                \
    ExpSmoother<float> interpDelayTime;                                                  \
    ExpSmoother<float> interpFeedback;                                                   \
    ExpSmoother<float> interpCross;                                                      \
    ExpSmoother<float> interpUnisonMix;                                                  \
    ExpSmoother<float> interpDry;                                                        \
    ExpSmoother<float> interpWet;                                                        \
                                                                                         \
    std::array<float, 2> shifterMainOut{};                                               \
    std::array<float, 2> shifterUnisonOut{};                                             \
    std::array<OverSampler, 2> overSampler;                                              \
    std::array<PitchShiftDelay<float>, 2> shifterMain;                                   \
    std::array<PitchShiftDelay<float>, 2> shifterUnison;                                 \
  };

DSPCORE_CLASS(AVX512)
DSPCORE_CLASS(AVX2)
DSPCORE_CLASS(AVX)

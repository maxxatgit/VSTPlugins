// (c) 2019-2020 Takamitsu Endo
//
// This file is part of SevenDelay.
//
// SevenDelay is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SevenDelay is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SevenDelay.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "pluginterfaces/vst/ivstprocesscontext.h"
#include "public.sdk/source/vst/vstaudioeffect.h"

#include "../../../common/dsp/smoother.hpp"
#include "../parameter.hpp"
#include "delay.hpp"
#include "iir.hpp"

#include <array>
#include <memory>

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

// Lagrange delay is very slow at debug build. If that's the case set Order to 1.
using DelayTypeName = DelayLagrange<float, 7>;
using FilterTypeName = SomeDSP::SVF<float>;
using DCKillerTypeName = SomeDSP::BiquadHighPass<float>;

class DSPCore {
public:
  GlobalParameter param;

  void setup(double sampleRate);
  void reset();                     // Stop sounds.
  void startup();                   // Reset phase, random seed etc.
  void setParameters(double tempo); // tempo is beat per minutes.

  void process(const size_t length, float *in0, float *in1, float *out0, float *out1);

protected:
  const float pi = 3.14159265358979323846;

  std::array<LinearSmoother<float>, 2> interpTime{};
  LinearSmoother<float> interpWetMix;
  LinearSmoother<float> interpDryMix;
  LinearSmoother<float> interpFeedback;
  LinearSmoother<float> interpLfoTimeAmount;
  LinearSmoother<float> interpLfoToneAmount;
  LinearSmoother<float> interpLfoFrequency;
  LinearSmoother<float> interpLfoShape;
  LinearSmoother<float> interpPanIn;
  LinearSmoother<float> interpSpreadIn;
  LinearSmoother<float> interpPanOut;
  LinearSmoother<float> interpSpreadOut;
  LinearSmoother<float> interpToneCutoff;
  LinearSmoother<float> interpToneQ;
  LinearSmoother<float> interpToneMix;
  LinearSmoother<float> interpDCKill;
  LinearSmoother<float> interpDCKillMix;

  double lfoPhase;
  double lfoPhaseTick;
  std::array<float, 2> delayOut{};
  std::array<DelayTypeName, 2> delay;
  std::array<FilterTypeName, 2> filter;
  std::array<DCKillerTypeName, 2> dcKiller;
};

} // namespace Synth
} // namespace Steinberg

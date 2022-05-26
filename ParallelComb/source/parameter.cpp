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

#include "parameter.hpp"
#include "../../common/dsp/constants.hpp"

#include <limits>

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

UIntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);
LogScale<double> Scales::delayTime(0.0, maxDelayTime, 0.5, 0.1 * maxDelayTime);
DecibelScale<double> Scales::dry(-60.0, 24.0, true);
DecibelScale<double> Scales::wet(-60.0, 60.0, true);
LogScale<double> Scales::feedback(0.0, 1.0, 0.5, 0.75);
SemitoneScale<double> Scales::feedbackHighpassCutoffHz(-37.0, 117.0, true);
LogScale<double> Scales::feedbackLimiterRelease(0.0, 16.0, 0.5, 1.0);
LogScale<double> Scales::delayTimeInterpRate(0.0, 4.0, 0.5, 0.25);

} // namespace Synth
} // namespace Steinberg

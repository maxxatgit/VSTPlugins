// (c) 2020 Takamitsu Endo
//
// This file is part of FoldShaper.
//
// FoldShaper is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// FoldShaper is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with FoldShaper.  If not, see <https://www.gnu.org/licenses/>.

#include "parameter.hpp"
#include "../../common/dsp/constants.hpp"

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

UIntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);

LogScale<double> Scales::inputGain(0.0, 16.0, 0.5, 2.0);
LogScale<double> Scales::outputGain(0.0, 1.0, 0.5, 0.1);
LinearScale<double> Scales::mul(1e-5, 1.0);
LinearScale<double> Scales::moreMul(1.0, 4.0);

LogScale<double> Scales::smoothness(0.0, 0.5, 0.1, 0.04);

// Limiter attack time is based on the time of 4095 frames in 192 kHz samplerate.
// 0.021328 ~= 4095 / 192k.
LogScale<double> Scales::limiterThreshold(0.01, 2.0, 0.5, 0.5);
LogScale<double> Scales::limiterAttack(0.0001, 0.021328, 0.1, 0.002);
LogScale<double> Scales::limiterRelease(0.0001, 0.2, 0.2, 0.01);

} // namespace Synth
} // namespace Steinberg

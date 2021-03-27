// (c) 2019-2020 Takamitsu Endo
//
// This file is part of TrapezoidSynth.
//
// TrapezoidSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TrapezoidSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with TrapezoidSynth.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../lib/juce_FastMathApproximations.h"

#include <array>
#include <cmath>

namespace SomeDSP {

template<typename Sample> class AMPitchShiter {
public:
  void setup(Sample sampleRate) { this->sampleRate = sampleRate; }

  void reset()
  {
    phase = 0;
    im = 0;
    x0.fill(0);
    x1.fill(0);
    x2.fill(0);
    y0.fill(0);
    y1.fill(0);
    y2.fill(0);
  }

  void setShift(Sample hz)
  {
    hz *= Sample(twopi);
    if (hz < 0)
      hz = 0;
    else if (hz > sampleRate)
      hz = sampleRate;
    delta = hz / sampleRate;
  }

  // Note: output may exceed the amplitude of input.
  Sample process(Sample input)
  {
    x0[7] = x0[6];
    x0[6] = x0[5];
    x0[5] = x0[4];
    x0[4] = input;
    x0[3] = x0[2];
    x0[2] = x0[1];
    x0[1] = x0[0];
    x0[0] = input;

    y0[0] = a[0] * (x0[0] + y2[0]) - x2[0];
    y0[1] = a[1] * (x0[1] + y2[1]) - x2[1];
    y0[2] = a[2] * (x0[2] + y2[2]) - x2[2];
    y0[3] = a[3] * (x0[3] + y2[3]) - x2[3];
    y0[4] = a[4] * (x0[4] + y2[4]) - x2[4];
    y0[5] = a[5] * (x0[5] + y2[5]) - x2[5];
    y0[6] = a[6] * (x0[6] + y2[6]) - x2[6];
    y0[7] = a[7] * (x0[7] + y2[7]) - x2[7];

    x2 = x1;
    x1 = x0;
    y2 = y1;
    y1 = y0;

    const auto re = y0[3];
    const auto norm = std::sqrt(re * re + im * im);
    const auto theta = std::atan2(im, re);
    const auto output = norm * std::cos(theta + phase);

    // 1 sample delayed.
    im = y0[7];

    phase += delta;
    if (phase > Sample(twopi)) phase -= Sample(twopi);

    return output;
  }

private:
  const Sample a[8] = {Sample(0.16175849836770106), Sample(0.7330289323414905),
                       Sample(0.9453497003291133),  Sample(0.9905991566845292),
                       Sample(0.47940086558884),    Sample(0.8762184935393101),
                       Sample(0.9765975895081993),  Sample(0.9974992559355491)};

  Sample sampleRate = 44100;
  Sample phase = 0;
  Sample delta = 0;
  Sample im = 0;

  std::array<Sample, 8> x0{};
  std::array<Sample, 8> x1{};
  std::array<Sample, 8> x2{};
  std::array<Sample, 8> y0{};
  std::array<Sample, 8> y1{};
  std::array<Sample, 8> y2{};
};

template<typename Sample> class SerialZDF1Pole {
public:
  Sample sampleRate = 44100;
  Sample feedback = 0;
  Sample saturation = 1;

  void setup(Sample sampleRate) { this->sampleRate = sampleRate; }

  void reset()
  {
    feedback = 0;
    saturation = 1;

    order = 7;
    gLP = 0;
    xLP.fill(0);
    yLP.fill(0);
    sLP.fill(0);
    vLP.fill(0);
  }

  void setOrder(size_t order) { this->order = order < 8 ? order : 7; }

  void setCutoff(Sample cutoff)
  {
    Sample omega_c
      = juce::dsp::FastMathApproximations::tan<Sample>(Sample(pi) * cutoff / sampleRate);
    gLP = omega_c / (Sample(1) + omega_c);
  }

  Sample process(Sample input)
  {
    input = juce::dsp::FastMathApproximations::tanh<Sample>(
      saturation * (input - feedback * yLP[order]));

    xLP[0] = input;
    xLP[1] = yLP[0];
    xLP[2] = yLP[1];
    xLP[3] = yLP[2];
    xLP[4] = yLP[3];
    xLP[5] = yLP[4];
    xLP[6] = yLP[5];
    xLP[7] = yLP[6];

    vLP[0] = (xLP[0] - sLP[0]) * gLP;
    vLP[1] = (xLP[1] - sLP[1]) * gLP;
    vLP[2] = (xLP[2] - sLP[2]) * gLP;
    vLP[3] = (xLP[3] - sLP[3]) * gLP;
    vLP[4] = (xLP[4] - sLP[4]) * gLP;
    vLP[5] = (xLP[5] - sLP[5]) * gLP;
    vLP[6] = (xLP[6] - sLP[6]) * gLP;
    vLP[7] = (xLP[7] - sLP[7]) * gLP;

    yLP[0] = vLP[0] + sLP[0];
    yLP[1] = vLP[1] + sLP[1];
    yLP[2] = vLP[2] + sLP[2];
    yLP[3] = vLP[3] + sLP[3];
    yLP[4] = vLP[4] + sLP[4];
    yLP[5] = vLP[5] + sLP[5];
    yLP[6] = vLP[6] + sLP[6];
    yLP[7] = vLP[7] + sLP[7];

    sLP[0] = yLP[0] + vLP[0];
    sLP[1] = yLP[1] + vLP[1];
    sLP[2] = yLP[2] + vLP[2];
    sLP[3] = yLP[3] + vLP[3];
    sLP[4] = yLP[4] + vLP[4];
    sLP[5] = yLP[5] + vLP[5];
    sLP[6] = yLP[6] + vLP[6];
    sLP[7] = yLP[7] + vLP[7];

    return yLP[order];
  }

private:
  size_t order = 7;
  Sample gLP = 0;
  std::array<Sample, 8> xLP{};
  std::array<Sample, 8> yLP{};
  std::array<Sample, 8> sLP{};
  std::array<Sample, 8> vLP{};
};

/**
Lowpass filter specialized for 8x oversampling.

```python
import numpy
from scipy import signal
sos = signal.butter(16, 20500*2/44100/8, "low", output="sos")
```
*/
template<typename Sample> class DecimationLowpass {
public:
  DecimationLowpass() {}

  void reset()
  {
    x0.fill(0);
    x1.fill(0);
    x2.fill(0);
    y0.fill(0);
    y1.fill(0);
    y2.fill(0);
  }

  Sample process(Sample input)
  {
    x0[0] = input;
    x0[1] = y0[0];
    x0[2] = y0[1];
    x0[3] = y0[2];
    x0[4] = y0[3];
    x0[5] = y0[4];
    x0[6] = y0[5];
    x0[7] = y0[6];

    y0[0] = co[0][0] * x0[0] + co[0][1] * x1[0] + co[0][2] * x2[0] - co[0][3] * y1[0]
      - co[0][4] * y2[0];
    y0[1] = co[1][0] * x0[1] + co[1][1] * x1[1] + co[1][2] * x2[1] - co[1][3] * y1[1]
      - co[1][4] * y2[1];
    y0[2] = co[2][0] * x0[2] + co[2][1] * x1[2] + co[2][2] * x2[2] - co[2][3] * y1[2]
      - co[2][4] * y2[2];
    y0[3] = co[3][0] * x0[3] + co[3][1] * x1[3] + co[3][2] * x2[3] - co[3][3] * y1[3]
      - co[3][4] * y2[3];
    y0[4] = co[4][0] * x0[4] + co[4][1] * x1[4] + co[4][2] * x2[4] - co[4][3] * y1[4]
      - co[4][4] * y2[4];
    y0[5] = co[5][0] * x0[5] + co[5][1] * x1[5] + co[5][2] * x2[5] - co[5][3] * y1[5]
      - co[5][4] * y2[5];
    y0[6] = co[6][0] * x0[6] + co[6][1] * x1[6] + co[6][2] * x2[6] - co[6][3] * y1[6]
      - co[6][4] * y2[6];
    y0[7] = co[7][0] * x0[7] + co[7][1] * x1[7] + co[7][2] * x2[7] - co[7][3] * y1[7]
      - co[7][4] * y2[7];

    x2 = x1;
    x1 = x0;
    y2 = y1;
    y1 = y0;

    return y0[7];
  }

  std::array<Sample, 8> x0{};
  std::array<Sample, 8> x1{};
  std::array<Sample, 8> x2{};
  std::array<Sample, 8> y0{};
  std::array<Sample, 8> y1{};
  std::array<Sample, 8> y2{};
  const std::array<std::array<Sample, 5>, 8> co{
    {{Sample(2.78583757e-13), Sample(5.57167513e-13), Sample(2.78583757e-13),
      Sample(-1.37840700e+00), Sample(4.75668194e-01)},
     {Sample(1.00000000e+00), Sample(2.00000000e+00), Sample(1.00000000e+00),
      Sample(-1.39243564e+00), Sample(4.90686697e-01)},
     {Sample(1.00000000e+00), Sample(2.00000000e+00), Sample(1.00000000e+00),
      Sample(-1.42080012e+00), Sample(5.21052591e-01)},
     {Sample(1.00000000e+00), Sample(2.00000000e+00), Sample(1.00000000e+00),
      Sample(-1.46409826e+00), Sample(5.67405872e-01)},
     {Sample(1.00000000e+00), Sample(2.00000000e+00), Sample(1.00000000e+00),
      Sample(-1.52317720e+00), Sample(6.30653459e-01)},
     {Sample(1.00000000e+00), Sample(2.00000000e+00), Sample(1.00000000e+00),
      Sample(-1.59904996e+00), Sample(7.11879848e-01)},
     {Sample(1.00000000e+00), Sample(2.00000000e+00), Sample(1.00000000e+00),
      Sample(-1.69274029e+00), Sample(8.12181022e-01)},
     {Sample(1.00000000e+00), Sample(2.00000000e+00), Sample(1.00000000e+00),
      Sample(-1.80501230e+00), Sample(9.32375009e-01)}}};
};

} // namespace SomeDSP

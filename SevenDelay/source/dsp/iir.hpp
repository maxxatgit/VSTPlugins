// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../../common/dsp/constants.hpp"

#include <cmath>

namespace SomeDSP {

template<typename Sample> class SVF {
public:
  void setup(Sample sampleRate) { this->sampleRate = sampleRate; }

  void setCoefficient()
  {
    Sample omega_c = std::tan(Sample(pi) * cutoff / sampleRate);
    g = omega_c / (Sample(1.0) + omega_c);
    twoR = 2 * resonance;
    g1 = twoR + g;
    d = Sample(1.0) / (Sample(1.0) + Sample(2.0) * resonance + g * g);
  }

  // q in (0, 1].
  void setCutoffQ(Sample hz, Sample q)
  {
    cutoff = hz > 0 ? hz : 0;
    resonance = q < Sample(1e-5) ? Sample(1e-5) : q;
    setCoefficient();
  }

  void reset()
  {
    yLP = yBP = yHP = 0;
    s1 = s2 = 0;
  }

  Sample process(Sample input)
  {
    yHP = (input - g1 * s1 - s2) * d;

    Sample v1 = g * yHP;
    yBP = v1 + s1;
    s1 = yBP + v1;

    Sample v2 = g * yBP;
    yLP = v2 + s2;
    s2 = yLP + v2;

    Sample output = input - Sample(2) * twoR * yBP;
    if (std::isfinite(output)) return output;

    reset();
    return 0;
  }

protected:
  Sample sampleRate = 44100;
  Sample cutoff = Sample(0.5);
  Sample resonance = Sample(0.5);

  Sample yLP = 0;
  Sample yBP = 0;
  Sample yHP = 0;

  Sample s1 = 0;
  Sample s2 = 0;

  Sample g = 0;
  Sample g1 = 0;
  Sample d = 0;
  Sample twoR = 0;
};

// http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt
template<typename Sample> class BiquadHighPass {
public:
  // q in (0, 1].
  void setup(Sample sampleRate, Sample q)
  {
    fs = sampleRate;
    this->q = q < Sample(1e-5) ? Sample(1e-5) : q;
  }

  void reset()
  {
    b0 = b1 = b2 = 0;
    a0 = a1 = a2 = 0;
    x1 = x2 = 0;
    y1 = y2 = 0;
  }

  void setCutoff(Sample hz)
  {
    f0 = hz > 0 ? hz : 0;

    Sample w0 = Sample(twopi) * f0 / fs;
    Sample cos_w0 = std::cos(w0);
    Sample sin_w0 = std::sin(w0);

    Sample alpha = sin_w0 / (Sample(2) * q);
    b0 = (Sample(1) + cos_w0) / Sample(2);
    b1 = -(Sample(1) + cos_w0);
    b2 = (Sample(1) + cos_w0) / Sample(2);
    a0 = Sample(1) + alpha;
    a1 = -Sample(2) * cos_w0;
    a2 = Sample(1) - alpha;

    b0 /= a0;
    b1 /= a0;
    b2 /= a0;
    a1 /= a0;
    a2 /= a0;
  }

  Sample process(Sample input)
  {
    Sample output = b0 * input + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;

    x2 = x1;
    x1 = input;

    y2 = y1;
    y1 = output;

    return output;
  }

protected:
  Sample fs = 44100;
  Sample f0 = Sample(0.5);
  Sample q = Sample(0.5);

  Sample b0 = 0;
  Sample b1 = 0;
  Sample b2 = 0;
  Sample a0 = 0;
  Sample a1 = 0;
  Sample a2 = 0;
  Sample x1 = 0;
  Sample x2 = 0;
  Sample y1 = 0;
  Sample y2 = 0;
};

} // namespace SomeDSP

// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include <cmath>
#include <cstdint>

namespace SomeDSP {

// Numerical Recipes In C p.284.
template<typename Sample> class White {
public:
  int32_t seed;

  White(int32_t seed) : seed(seed) {}

  Sample process()
  {
    seed = 1664525L * seed + 1013904223L;
    return (Sample)seed / ((Sample)INT32_MAX + (Sample)1); // Normalize to [-1, 1).
  }
};

// Paul Kellet's refined method in Allan's analysis.
// http://www.firstpr.com.au/dsp/pink-noise/
template<typename Sample> class Pink {
public:
  Pink(int32_t seed) : rng(seed) {}

  Sample process()
  {
    const Sample gain = 0.125;
    auto white = rng.process();
    b0 = Sample(0.99886) * b0 + white * Sample(0.0555179) * gain;
    b1 = Sample(0.99332) * b1 + white * Sample(0.0750759) * gain;
    b2 = Sample(0.96900) * b2 + white * Sample(0.1538520) * gain;
    b3 = Sample(0.86650) * b3 + white * Sample(0.3104856) * gain;
    b4 = Sample(0.55000) * b4 + white * Sample(0.5329522) * gain;
    b5 = Sample(-0.7616) * b5 - white * Sample(0.0168980) * gain;
    auto pink = b0 + b1 + b2 + b3 + b4 + b5 + b6 + white * Sample(0.5362) * gain;
    b6 = white * Sample(0.115926);
    return pink;
  }

private:
  White<Sample> rng;
  Sample b0 = 0.0;
  Sample b1 = 0.0;
  Sample b2 = 0.0;
  Sample b3 = 0.0;
  Sample b4 = 0.0;
  Sample b5 = 0.0;
  Sample b6 = 0.0;
};

} // namespace SomeDSP

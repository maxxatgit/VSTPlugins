// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include <algorithm>
#include <vector>

template<typename Sample, unsigned char Order> class FractionalDelayLagrange {
public:
  void reset()
  {
    xd.fill(0);
    diff.fill(0);
  }

  void push(Sample input)
  {
    diff[0] = input - xd[0];
    for (size_t i = 1; i < Order; ++i) diff[i] = diff[i - 1] - xd[i];

    xd[0] = input;
    for (size_t i = 1; i < Order; ++i) xd[i] = diff[i - 1];
  }

  Sample at(Sample fraction)
  {
    Sample delta = fraction + (Order - 1) / 2;
    Sample sig = 0.0;

    size_t i = Order;
    while (i > 0) {
      size_t next = i - 1;
      sig = (next - delta) / i * (diff[next] + sig);
      i = next;
    }

    return sig + xd[0];
  }

  Sample process(Sample input, Sample fraction)
  {
    push(input);
    return at(fraction);
  }

private:
  std::array<Sample, Order> xd{};
  std::array<Sample, Order> diff{};
};

template<typename Sample, unsigned char Order> class DelayLagrange {
public:
  void setup(Sample sampleRate, Sample time, Sample maxTime)
  {
    this->sampleRate = overSample * sampleRate;

    auto size = (size_t)(maxTime * this->sampleRate);
    if (size >= INT32_MAX)
      size = INT32_MAX;
    else if (size == 0)
      size += 1;
    buf.resize(size, 0.0);

    setTime(time);
  }

  void setTime(Sample value)
  {
    auto timeInSample
      = std::max<Sample>(fix, std::min<Sample>(sampleRate * value, (Sample)buf.size()));

    int32_t time_int = (int32_t)timeInSample;
    rFraction = timeInSample - (Sample)time_int;

    rptr = wptr - time_int + fix;
    while (rptr < 0) rptr += (int32_t)buf.size();
  }

  void reset()
  {
    std::fill(buf.begin(), buf.end(), Sample(0));
    wInterp.reset();
  }

  Sample process(Sample input)
  {
    wInterp.push(input);

    for (size_t i = 1; i <= overSample; ++i) {
      Sample wFraction = (overSample - (Sample)i) / overSample;

      buf[wptr] = wInterp.at(wFraction);
      wptr += 1;
      while ((size_t)wptr >= buf.size()) wptr -= (int32_t)buf.size();
    }

    rptr += overSample;
    while (rptr >= (int32_t)buf.size()) rptr -= (int32_t)buf.size();
    int32_t i0 = rptr - 1;
    while (i0 < 0) i0 += (int32_t)buf.size();
    int32_t i1 = rptr - 2;
    while (i1 < 0) i1 += (int32_t)buf.size();

    return buf[i0] - rFraction * (buf[i0] - buf[i1]);
  }

private:
  static const size_t overSample = Order;
  static const size_t fix = (overSample * (Order - 1)) / 2;
  Sample sampleRate = 44100.0;
  Sample rFraction = 0;
  std::vector<Sample> buf{1};
  int32_t wptr = 0;
  int32_t rptr = 0;
  FractionalDelayLagrange<Sample, Order> wInterp;
};

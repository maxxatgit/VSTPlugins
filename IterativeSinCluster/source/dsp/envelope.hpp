// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/smoother.hpp"

#include <cmath>

namespace SomeDSP {

// t in [0, 1].
template<typename Sample> inline Sample cosinterp(Sample t)
{
  return Sample(0.5) * (Sample(1) - std::cos(Sample(pi) * t));
}

template<typename Sample> class ExpDecayCurve {
public:
  void reset(Sample sampleRate, Sample seconds)
  {
    value = Sample(1);
    set(sampleRate, seconds);
  }

  void set(Sample sampleRate, Sample seconds)
  {
    alpha = std::pow(threshold, Sample(1) / (seconds * sampleRate));
  }

  bool isTerminated() { return value <= threshold; }

  Sample process()
  {
    if (value <= threshold) return Sample(0);
    value *= alpha;
    return value - threshold;
  }

protected:
  const Sample threshold = Sample(1e-5);
  Sample value = 0;
  Sample alpha = 0;
};

template<typename Sample> class ExpAttackCurve {
public:
  void reset(Sample sampleRate, Sample seconds)
  {
    value = threshold;
    set(sampleRate, seconds);
  }

  void set(Sample sampleRate, Sample seconds)
  {
    alpha = std::pow(Sample(1.0) / threshold, Sample(1.0) / (seconds * sampleRate));
  }

  bool isTerminated() { return value >= Sample(1); }

  Sample process()
  {
    value *= alpha;
    if (value >= Sample(1)) return Sample(1 - threshold);
    return value - threshold;
  }

protected:
  const Sample threshold = Sample(1e-5);
  Sample value = 0;
  Sample alpha = 0;
};

// 1 - ExpDecayCurve.process();
template<typename Sample> class NegativeExpAttackCurve {
public:
  void reset(Sample sampleRate, Sample seconds)
  {
    value = Sample(1);
    set(sampleRate, seconds);
  }

  void set(Sample sampleRate, Sample seconds)
  {
    alpha = std::pow(threshold, Sample(1) / (seconds * sampleRate));
  }

  bool isTerminated() { return value <= threshold; }

  Sample process()
  {
    if (value <= threshold) return Sample(1 - threshold);
    value *= alpha;
    return Sample(1 - threshold) - value;
  }

protected:
  const Sample threshold = Sample(1e-5);
  Sample value = 0;
  Sample alpha = 0;
};

template<typename Sample> class ExpADSREnvelope {
public:
  void setup(Sample sampleRate)
  {
    this->sampleRate = sampleRate;
    declickLength = int32_t(0.001 * sampleRate);
  }

  Sample adaptTime(Sample seconds, Sample noteFreq)
  {
    const Sample cycle = Sample(1) / noteFreq;
    return seconds < cycle ? cycle : seconds;
  }

  void reset(
    Sample attackTime,
    Sample decayTime,
    Sample sustainLevel,
    Sample releaseTime,
    Sample noteFreq,
    Sample curve_)
  {
    declickCounter = 0;
    state = State::attack;

    sustain = std::max<Sample>(Sample(0.0), std::min<Sample>(sustainLevel, Sample(1.0)));

    range = Sample(1);
    curve = curve_;

    attackTime = adaptTime(attackTime, noteFreq);
    atk.reset(sampleRate, attackTime);
    atkNeg.reset(sampleRate, attackTime);
    dec.reset(sampleRate, decayTime);
    rel.reset(sampleRate, adaptTime(releaseTime, noteFreq));
  }

  void set(
    Sample attackTime,
    Sample decayTime,
    Sample sustainLevel,
    Sample releaseTime,
    Sample noteFreq)
  {
    switch (state) {
      case State::attack:
        attackTime = adaptTime(attackTime, noteFreq);
        atk.set(sampleRate, attackTime);
        atkNeg.set(sampleRate, attackTime);
        // Fall through.

      case State::decay:
        dec.set(sampleRate, decayTime);
        // Fall through.

      case State::sustain:
        sustain
          = std::max<Sample>(Sample(0.0), std::min<Sample>(sustainLevel, Sample(1.0)));
        // Fall through.

      case State::release:
        rel.set(sampleRate, adaptTime(releaseTime, noteFreq));
        // Fall through.

      default:
        break;
    }
  }

  void release()
  {
    range = value;
    state = State::release;
  }

  void terminate()
  {
    state = State::terminated;
    value = 0;
    range = 1;
    sustain = 1;
  }

  bool isAttacking() { return state == State::attack; }
  bool isReleasing() { return state == State::release; }
  bool isTerminated() { return state == State::terminated; }

  inline Sample declickIn(Sample input)
  {
    if (declickCounter >= declickLength) return input;
    declickCounter += 1;
    return input * cosinterp<Sample>(declickCounter / (Sample)declickLength);
  }

  Sample process()
  {
    switch (state) {
      case State::attack: {
        const auto atkPos = atk.process();
        const auto atkMix = atkPos + curve * (atkNeg.process() - atkPos);
        value = range * atkMix;
        if (atk.isTerminated()) {
          state = State::decay;
          range = Sample(1.0) - sustain;
        }
      } break;

      case State::decay:
        value = range * dec.process() + sustain;
        if (value <= sustain) state = State::sustain;
        break;

      case State::sustain:
        value = sustain;
        break;

      case State::release:
        value = range * rel.process();
        if (rel.isTerminated()) state = State::terminated;
        break;

      default:
        return 0;
    }
    return declickIn(value);
  }

protected:
  enum class State : int32_t { attack, decay, sustain, release, terminated };

  int32_t declickLength;
  int32_t declickCounter = 0;

  ExpAttackCurve<Sample> atk{};
  NegativeExpAttackCurve<Sample> atkNeg{};
  ExpDecayCurve<Sample> dec{};
  ExpDecayCurve<Sample> rel{};

  State state = State::terminated;
  Sample value = 0;
  Sample curve = 0;
  Sample sampleRate = 44100;
  Sample range = 1;
  Sample sustain = 1;
};

} // namespace SomeDSP

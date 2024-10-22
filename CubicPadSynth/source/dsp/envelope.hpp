// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../../common/dsp/smoother.hpp"
#include "../../../lib/vcl.hpp"
#include "../../../lib/vcl/vectormath_exp.h"
#include "../../../lib/vcl/vectormath_hyp.h"

#include <algorithm>
#include <cmath>

namespace SomeDSP {

class alignas(64) ExpADSREnvelope16 {
public:
  void setup(float sampleRate, float sustainLevel)
  {
    this->sampleRate = sampleRate;
    sus.reset(sustainLevel);
    declickInRamp = 0.01f * sampleRate;
  }

  float adaptTime(float seconds, float noteFreq)
  {
    const float cycle = float(4) / noteFreq;
    return seconds >= cycle ? seconds : cycle > float(0.1) ? float(0.1) : cycle;
  }

  Vec16f adaptTime(Vec16f seconds, Vec16f noteFreq)
  {
    Vec16f cycle = float(4) / noteFreq;
    return select(seconds >= cycle, seconds, cycle);
  }

  float secondToMultiplier(float seconds)
  {
    return powf(threshold, float(1) / (seconds * sampleRate));
  }

  Vec16f secondToMultiplier(Vec16f seconds)
  {
    return pow(Vec16f(threshold), float(1) / (seconds * sampleRate));
  }

  void reset(int index)
  {
    state.insert(index, stateAttack);
    value.insert(index, float(1));
    declickIn.insert(index, 0);
  }

  void resetSustain(float sustainLevel)
  {
    sus.reset(std::clamp<float>(sustainLevel, 0.0f, 1.0f));
  }

  void set(
    float attackTime,
    float decayTime,
    float sustainLevel,
    float releaseTime,
    Vec16f noteFreq)
  {
    sus.push(std::clamp<float>(sustainLevel, 0.0f, 1.0f));
    atk = secondToMultiplier(adaptTime(attackTime, noteFreq));
    dec = secondToMultiplier(decayTime);
    rel = secondToMultiplier(adaptTime(releaseTime, noteFreq));
  }

  void setRelease(int index, float seconds)
  {
    rel.insert(index, secondToMultiplier(seconds));
  }

  void release(int index)
  {
    value.insert(index, out[index]);
    state.insert(index, stateRelease);
  }

  void terminate()
  {
    value = 0;
    out = 0;
    state = stateTerminated;
  }

  bool isAttacking(int index) { return state[index] == stateAttack; }
  bool isReleasing(int index) { return state[index] == stateRelease; }
  bool isTerminated(int index) { return state[index] == stateTerminated; }
  float extract(int index) { return out[index]; }

  Vec16f process()
  {
    sus.process();

    Vec16ib valueRefresh(value <= threshold);
    state = select(valueRefresh, state + 1, state);
    value = select(valueRefresh, float(1), value);

    Vec16ib stateAtk(state == stateAttack);
    Vec16ib stateDec(state == stateDecay);

    value = select(stateAtk, value * atk, value);
    value = select(stateDec, value * dec, value);
    value = select(state == stateSustain, sus.getValue(), value);
    value = select(state == stateRelease, value * rel, value);
    value = select(state >= stateTerminated, threshold, value);

    out = value;
    out = select(stateAtk, float(1) - out, out);
    out = select(stateDec, (float(1) - sus.getValue()) * out + sus.getValue(), out);
    out -= threshold;
    // return out;

    declickIn += declickInRamp;
    declickIn = select(declickIn > float(1), float(1), declickIn);
    return declickIn * out;
  }

protected:
  static constexpr float threshold = 1e-5f;

  enum State : int32_t {
    stateAttack,
    stateDecay,
    stateSustain,
    stateRelease,
    stateTerminated
  };

  float sampleRate = 44100;
  LinearSmoother<float> sus;
  Vec16f declickInRamp = 1;
  Vec16f declickIn = 1;
  Vec16f atk = 1;
  Vec16f dec = 1;
  Vec16f rel = 1;
  Vec16i state = stateTerminated;
  Vec16f value = 0;
  Vec16f out = 0;
};

class alignas(64) LinearADSREnvelope16 {
public:
  void setup(float sampleRate, float sustainLevel)
  {
    this->sampleRate = sampleRate;
    sus.reset(sustainLevel);
  }

  float adaptTime(float seconds, float noteFreq)
  {
    const float cycle = float(1) / noteFreq;
    return seconds >= cycle ? seconds : cycle > float(0.1) ? float(0.1) : cycle;
  }

  Vec16f adaptTime(Vec16f seconds, Vec16f noteFreq)
  {
    Vec16f cycle = 1.0f / noteFreq;
    return select(seconds >= cycle, seconds, cycle);
  }

  float secondToDelta(float seconds) { return float(1) / (sampleRate * seconds); }
  Vec16f secondToDelta(Vec16f seconds) { return float(1) / (sampleRate * seconds); }

  void reset(
    int index,
    float attackTime,
    float decayTime,
    float sustainLevel,
    float releaseTime,
    float noteFreq)
  {
    state.insert(index, stateAttack);
    value.insert(index, float(1) - value[index]);
    set(attackTime, decayTime, sustainLevel, releaseTime, noteFreq);
  }

  void resetSustain(float sustainLevel)
  {
    sus.reset(std::clamp<float>(sustainLevel, 0.0f, 1.0f));
    state = stateTerminated;
    value = 0;
    out = 0;
  }

  void set(
    float attackTime,
    float decayTime,
    float sustainLevel,
    float releaseTime,
    Vec16f noteFreq)
  {
    sus.push(std::max<float>(float(0.0), std::min<float>(sustainLevel, float(1.0))));
    atk = secondToDelta(adaptTime(attackTime, noteFreq));
    dec = secondToDelta(adaptTime(decayTime, noteFreq));
    rel = secondToDelta(adaptTime(releaseTime, noteFreq));
  }

  void release(int index)
  {
    value.insert(index, out[index]);
    state.insert(index, stateRelease);
  }

  bool isAttacking(int index) { return state[index] == stateAttack; }
  bool isReleasing(int index) { return state[index] == stateRelease; }
  bool isTerminated(int index) { return state[index] == stateTerminated; }

  Vec16f process()
  {
    sus.process();

    Vec16ib valueRefresh(value <= 0.0f);
    state = select(valueRefresh, state + 1, state);
    value = select(valueRefresh, 1.0f, value);

    Vec16ib stateAtk(state == stateAttack);
    Vec16ib stateDec(state == stateDecay);
    Vec16ib stateRel(state == stateRelease);

    value = select(stateAtk, value - atk, value);
    value = select(stateDec, value - dec, value);
    value = select(state == stateSustain, sus.getValue(), value);
    value = select(stateRel, value - rel, value);
    value = select(state >= stateTerminated, 0.0f, value);

    out = value;
    out = select(stateAtk, float(1) - out, out);
    out = select(stateDec, (float(1) - sus.getValue()) * out + sus.getValue(), out);
    return out;
  }

protected:
  enum State : int32_t {
    stateAttack,
    stateDecay,
    stateSustain,
    stateRelease,
    stateTerminated
  };

  float sampleRate = 44100;
  LinearSmoother<float> sus;
  Vec16f atk = 0.01f;
  Vec16f dec = 0.01f;
  Vec16f rel = 0.01f;
  Vec16i state = stateTerminated;
  Vec16f value = 0;
  Vec16f out = 0;
};

} // namespace SomeDSP

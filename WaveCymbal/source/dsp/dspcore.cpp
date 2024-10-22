// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "../../../lib/juce_ScopedNoDenormal.hpp"

#include "dspcore.hpp"

inline float clamp(float value, float min, float max)
{
  return (value < min) ? min : (value > max) ? max : value;
}

inline float midiNoteToFrequency(float pitch, float tuning)
{
  return 440.0f * powf(2.0f, ((pitch - 69.0f) * 100.0f + tuning) / 1200.0f);
}

float paramToPitch(float bend) { return powf(2.0f, ((bend - 0.5f) * 400.0f) / 1200.0f); }

void DSPCore::setSystem()
{
  excitor.set(
    param.value[ParameterID::pickCombTime]->getFloat(),
    param.value[ParameterID::pickCombFeedback]->getFloat(),
    param.value[ParameterID::randomAmount]->getFloat());

  cymbal.set(
    1 + param.value[ParameterID::nCymbal]->getInt(),
    1 + param.value[ParameterID::stack]->getInt(),
    param.value[ParameterID::minFrequency]->getFloat(),
    param.value[ParameterID::maxFrequency]->getFloat(),
    param.value[ParameterID::distance]->getFloat(),
    param.value[ParameterID::damping]->getFloat(),
    param.value[ParameterID::pulsePosition]->getFloat(),
    param.value[ParameterID::pulseWidth]->getFloat(),
    param.value[ParameterID::decay]->getFloat(),
    param.value[ParameterID::bandpassQ]->getFloat(),
    static_cast<CrossoverType>(param.value[ParameterID::cutoffMap]->getInt()),
    param.value[ParameterID::randomAmount]->getFloat());
}

DSPCore::DSPCore() { midiNotes.reserve(128); }

void DSPCore::setup(double sampleRate)
{
  this->sampleRate = float(sampleRate);

  midiNotes.resize(0);

  SmootherCommon<float>::setSampleRate(this->sampleRate);
  SmootherCommon<float>::setTime(param.value[ParameterID::smoothness]->getFloat());

  noteStack.reserve(128);
  noteStack.resize(0);

  pulsar.sampleRate = this->sampleRate;
  velvetNoise.sampleRate = this->sampleRate;

  excitor.setup(this->sampleRate);
  cymbal.setup(this->sampleRate);
  setSystem();

  startup();
}

void DSPCore::reset()
{
  pulsar.reset();
  velvetNoise.reset();
  brownNoise.reset(0);

  excitor.reset();
  cymbal.reset();

  trigger = false;

  interpMasterGain.reset(param.value[ParameterID::gain]->getFloat());
  interpPitch.reset(0.0f);

  startup();
}

void DSPCore::startup() { rnd.seed = param.value[ParameterID::seed]->getInt(); }

void DSPCore::setParameters()
{
  SmootherCommon<float>::setTime(param.value[ParameterID::smoothness]->getFloat());

  interpMasterGain.push(param.value[ParameterID::gain]->getFloat());

  if (trigger) {
    trigger = false;

    if (param.value[ParameterID::retrigger]->getInt())
      rnd.seed = param.value[ParameterID::seed]->getInt();

    excitor.trigger(rnd);
    cymbal.trigger(rnd);
  }

  setSystem();

  if (param.value[ParameterID::oscType]->getInt() >= 2 && !noteStack.empty()) {
    const auto freq = noteStack.back().frequency
      * paramToPitch(param.value[ParameterID::pitchBend]->getFloat());
    interpPitch.push(freq);
    velvetNoise.setDensity(freq);
  } else {
    pulsar.setFrequency(0);
    velvetNoise.setDensity(0);
    interpPitch.push(0.0f);
  }
}

void DSPCore::process(
  const size_t length, const float *in0, const float *in1, float *out0, float *out1)
{
  ScopedNoDenormals scopedDenormals;

  SmootherCommon<float>::setBufferSize(float(length));

  const bool excitation = param.value[ParameterID::excitation]->getInt();
  const bool collision = param.value[ParameterID::collision]->getInt();
  const uint32_t oscType = param.value[ParameterID::oscType]->getInt();

  float sample;
  for (size_t i = 0; i < length; ++i) {
    processMidiNote(i);

    const float pitch = interpPitch.process();
    switch (oscType) {
      case 0: // Off
        sample = in0[i] + in1[i];
        break;

      case 2: // Sustain
        pulsar.setFrequency(pitch);
        // Fall through.

      default:
      case 1: // Impulse
        sample = pulsar.process() + in0[i] + in1[i];
        break;

      case 3: // Velvet
        sample = velvetNoise.process() + in0[i] + in1[i];
        break;

      case 4: // Brown
        brownNoise.drift = 2 * pitch / sampleRate;
        sample = brownNoise.process() + in0[i] + in1[i];
        break;
    }

    if (excitation) sample = excitor.process(sample);
    sample = cymbal.process(sample, collision);

    const float masterGain = interpMasterGain.process();

    // Only write to buffer if bypass is off. This is because VST 3 specification says
    // that input and output buffer may points to the same memory region.
    if (!param.value[ParameterID::bypass]->getInt()) {
      out0[i] = masterGain * sample;
      out1[i] = masterGain * sample;
    }
  }
}

void DSPCore::noteOn(int32_t noteId, int16_t pitch, float tuning, float velocity)
{
  trigger = true;
  pulsar.phase = 1.0f;
  velvetNoise.phase = 1.0f;

  NoteInfo info;
  info.id = noteId;
  info.frequency = midiNoteToFrequency(pitch, tuning);
  info.velocity = velocity;
  noteStack.push_back(info);
}

void DSPCore::noteOff(int32_t noteId)
{
  auto it = std::find_if(noteStack.begin(), noteStack.end(), [&](const NoteInfo &info) {
    return info.id == noteId;
  });
  if (it == noteStack.end()) return;
  noteStack.erase(it);

  if (noteStack.size() == 0) pulsar.reset();
}

// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../../../lib/vcl.hpp"
#include "../../../lib/vcl/vectormath_exp.h"
#include "../parameter.hpp"
#include "envelope.hpp"
#include "noise.hpp"
#include "oscillator.hpp"

#include <array>
#include <cmath>
#include <random>

using namespace SomeDSP;
using namespace Steinberg::Synth;

class EMAFilter16 {
public:
  void setP(float p) { kp = std::clamp<float>(p, float(0), float(1)); };
  void setP(int index, float p) { kp.insert(index, p); };
  void reset() { value = 0; }
  Vec16f process(Vec16f input) { return value += kp * (input - value); }

private:
  Vec16f kp = 1; // In [0, 1].
  Vec16f value = 0;
};

inline float calcMasterPitch(int32_t octave, int32_t semi, int32_t milli, float bend)
{
  return 12 * octave + semi + milli / 1000.0f + (bend - 0.5f) * 4.0f;
}

inline float getMasterPitch(GlobalParameter &param)
{
  using ID = ParameterID::ID;
  return calcMasterPitch(
    int32_t(param.value[ID::oscOctave]->getInt()) - 12,
    param.value[ID::oscSemi]->getInt() - 120, param.value[ID::oscMilli]->getInt() - 1000,
    param.value[ID::pitchBend]->getFloat());
}

constexpr size_t nUnit = 8;

enum class NoteState { active, release, rest };

struct NoteProcessInfo {
  std::minstd_rand rng{0};

  LinearSmoother<float> masterPitch;
  LinearSmoother<float> equalTemperament;
  LinearSmoother<float> pitchA4Hz;
  LinearSmoother<float> tableLowpass;
  LinearSmoother<float> tableLowpassKeyFollow;
  LinearSmoother<float> tableLowpassEnvelopeAmount;
  LinearSmoother<float> pitchEnvelopeAmount;
  LinearSmoother<float> lfoFrequency;
  LinearSmoother<float> lfoPitchAmount;
  LinearSmoother<float> lfoLowpass;

  void reset(GlobalParameter &param)
  {
    using ID = ParameterID::ID;

    masterPitch.reset(getMasterPitch(param));
    equalTemperament.reset(param.value[ID::equalTemperament]->getFloat() + 1);
    pitchA4Hz.reset(param.value[ID::pitchA4Hz]->getFloat() + 100);
    tableLowpass.reset(
      float(Scales::tableLowpass.getMax()) - param.value[ID::tableLowpass]->getFloat());
    tableLowpassKeyFollow.reset(param.value[ID::tableLowpassKeyFollow]->getFloat());
    tableLowpassEnvelopeAmount.reset(
      param.value[ID::tableLowpassEnvelopeAmount]->getFloat());
    pitchEnvelopeAmount.reset(
      param.value[ID::pitchEnvelopeAmount]->getFloat()
      * (param.value[ID::pitchEnvelopeAmountNegative]->getInt() ? -1 : 1));

    lfoFrequency.reset(0);
    lfoPitchAmount.reset(param.value[ID::lfoPitchAmount]->getFloat());
    lfoLowpass.reset(param.value[ID::lfoLowpass]->getFloat());
  }
};

#define PROCESSING_UNIT_CLASS(INSTRSET)                                                  \
  struct ProcessingUnit_##INSTRSET {                                                     \
    TableOsc16<tableSize> osc;                                                           \
    LfoTableOsc16<lfoTableSize> lfo;                                                     \
    EMAFilter16 lfoSmoother;                                                             \
    ExpADSREnvelope16 gainEnvelope;                                                      \
    LinearADSREnvelope16 pitchEnvelope;                                                  \
    LinearADSREnvelope16 lowpassEnvelope;                                                \
                                                                                         \
    Vec16f notePitch = 0;                                                                \
    Vec16f pitch = 0;                                                                    \
    Vec16f lowpassPitch = 0;                                                             \
    Vec16f notePan = 0.5f;                                                               \
    Vec16f frequency = 1;                                                                \
    Vec16f gain = 0;                                                                     \
    Vec16f gain0 = 0;                                                                    \
    Vec16f gain1 = 0;                                                                    \
    Vec16f velocity = 0;                                                                 \
                                                                                         \
    bool isActive = false;                                                               \
                                                                                         \
    void setParameters(float sampleRate, NoteProcessInfo &info, GlobalParameter &param); \
    std::array<float, 2> process(                                                        \
      float sampleRate,                                                                  \
      WaveTable<tableSize, nOvertone> &wavetable,                                        \
      LfoWaveTable<lfoTableSize> &lfoWaveTable,                                          \
      NoteProcessInfo &info);                                                            \
    void reset(GlobalParameter &param);                                                  \
  };

#define NOTE_CLASS(INSTRSET)                                                             \
  class Note_##INSTRSET {                                                                \
  public:                                                                                \
    NoteState state = NoteState::rest;                                                   \
                                                                                         \
    float sampleRate = 44100;                                                            \
                                                                                         \
    int vecIndex = 0;                                                                    \
    int arrayIndex = 0;                                                                  \
    int32_t id = -1;                                                                     \
                                                                                         \
    void setup(float sampleRate);                                                        \
    void noteOn(                                                                         \
      int32_t noteId,                                                                    \
      float notePitch,                                                                   \
      float velocity,                                                                    \
      float pan,                                                                         \
      float phase,                                                                       \
      NoteProcessInfo &info,                                                             \
      std::array<ProcessingUnit_##INSTRSET, nUnit> &units,                               \
      GlobalParameter &param);                                                           \
    void release(std::array<ProcessingUnit_##INSTRSET, nUnit> &units);                   \
    void release(std::array<ProcessingUnit_##INSTRSET, nUnit> &units, float seconds);    \
    void rest();                                                                         \
    bool isAttacking(std::array<ProcessingUnit_##INSTRSET, nUnit> &units);               \
    float getGain(std::array<ProcessingUnit_##INSTRSET, nUnit> &units);                  \
  };

class DSPInterface {
public:
  virtual ~DSPInterface() {};

  static const size_t maxVoice = 128;
  GlobalParameter param;

  virtual void setup(double sampleRate) = 0;
  virtual void reset() = 0;   // Stop sounds.
  virtual void startup() = 0; // Reset phase, random seed etc.
  virtual void setParameters(float tempo) = 0;
  virtual void process(const size_t length, float *out0, float *out1) = 0;
  virtual void noteOn(int32_t noteId, int16_t pitch, float tuning, float velocity) = 0;
  virtual void noteOff(int32_t noteId) = 0;
  virtual void refreshTable() = 0;
  virtual void refreshLfo() = 0;

  struct MidiNote {
    bool isNoteOn;
    uint32_t frame;
    int32_t id;
    int16_t pitch;
    float tuning;
    float velocity;
  };

  std::vector<MidiNote> midiNotes;

  virtual void pushMidiNote(
    bool isNoteOn,
    uint32_t frame,
    int32_t noteId,
    int16_t pitch,
    float tuning,
    float velocity)
    = 0;
  virtual void processMidiNote(uint32_t frame) = 0;
};

#define DSPCORE_CLASS(INSTRSET)                                                          \
  class DSPCore_##INSTRSET final : public DSPInterface {                                 \
  public:                                                                                \
    DSPCore_##INSTRSET();                                                                \
                                                                                         \
    void setup(double sampleRate) override;                                              \
    void reset() override;                                                               \
    void startup() override;                                                             \
    void setParameters(float tempo) override;                                            \
    void process(const size_t length, float *out0, float *out1) override;                \
    void noteOn(int32_t noteId, int16_t pitch, float tuning, float velocity) override;   \
    void fillTransitionBuffer(size_t noteIndex);                                         \
    void noteOff(int32_t noteId) override;                                               \
    void refreshTable() override;                                                        \
    void refreshLfo() override;                                                          \
                                                                                         \
    void pushMidiNote(                                                                   \
      bool isNoteOn,                                                                     \
      uint32_t frame,                                                                    \
      int32_t noteId,                                                                    \
      int16_t pitch,                                                                     \
      float tuning,                                                                      \
      float velocity) override                                                           \
    {                                                                                    \
      MidiNote note;                                                                     \
      note.isNoteOn = isNoteOn;                                                          \
      note.frame = frame;                                                                \
      note.id = noteId;                                                                  \
      note.pitch = pitch;                                                                \
      note.tuning = tuning;                                                              \
      note.velocity = velocity;                                                          \
      midiNotes.push_back(note);                                                         \
    }                                                                                    \
                                                                                         \
    void processMidiNote(uint32_t frame) override                                        \
    {                                                                                    \
      while (true) {                                                                     \
        auto it                                                                          \
          = std::find_if(midiNotes.begin(), midiNotes.end(), [&](const MidiNote &nt) {   \
              return nt.frame == frame;                                                  \
            });                                                                          \
        if (it == std::end(midiNotes)) return;                                           \
        if (it->isNoteOn)                                                                \
          noteOn(it->id, it->pitch, it->tuning, it->velocity);                           \
        else                                                                             \
          noteOff(it->id);                                                               \
        midiNotes.erase(it);                                                             \
      }                                                                                  \
    }                                                                                    \
                                                                                         \
  private:                                                                               \
    void sortVoiceIndicesByGain();                                                       \
    void terminateNotes(size_t nNote);                                                   \
                                                                                         \
    float sampleRate = 44100.0f;                                                         \
                                                                                         \
    std::array<float, nOvertone> otFrequency{};                                          \
    std::array<float, nOvertone> otGain{};                                               \
    std::array<float, nOvertone> otPhase{};                                              \
    std::array<float, nOvertone> otBandWidth{};                                          \
                                                                                         \
    bool prepareRefresh = true;                                                          \
    bool isTableRefeshed = false;                                                        \
    bool isLFORefreshed = false;                                                         \
    WaveTable<tableSize, nOvertone> wavetable;                                           \
    LfoWaveTable<lfoTableSize> lfoWavetable;                                             \
    std::array<ProcessingUnit_##INSTRSET, nUnit> units;                                  \
                                                                                         \
    size_t nVoice = 32;                                                                  \
    int32_t panCounter = 0;                                                              \
    std::vector<size_t> noteIndices;                                                     \
    std::vector<size_t> voiceIndices;                                                    \
    std::vector<float> unisonPan;                                                        \
    std::array<Note_##INSTRSET, maxVoice> notes;                                         \
                                                                                         \
    NoteProcessInfo info;                                                                \
    LinearSmoother<float> interpMasterGain;                                              \
                                                                                         \
    std::vector<std::array<float, 2>> transitionBuffer{};                                \
    bool isTransitioning = false;                                                        \
    size_t trIndex = 0;                                                                  \
    size_t trStop = 0;                                                                   \
    TableOsc<tableSize> trOsc;                                                           \
  };

PROCESSING_UNIT_CLASS(FixedInstruction)
NOTE_CLASS(FixedInstruction)
DSPCORE_CLASS(FixedInstruction)

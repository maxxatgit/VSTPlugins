// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/multirate.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../parameter.hpp"
#include "lfo.hpp"
#include "oscillator.hpp"

#include <array>
#include <cmath>
#include <random>

using namespace SomeDSP;
using namespace Steinberg::Synth;

enum class NoteState { active, release, rest };

#define NOTE_PROCESS_INFO_SMOOTHER(METHOD)                                               \
  using ID = ParameterID::ID;                                                            \
  auto &pv = param.value;                                                                \
                                                                                         \
  eqTemp = pv[ID::equalTemperament]->getInt() + 1;                                       \
  auto semitone = int_fast32_t(pv[ID::semitone]->getInt()) - 120;                        \
  auto octave = int_fast32_t(pv[ID::octave]->getInt()) - 12;                             \
  auto milli = float(0.001) * (int_fast32_t(pv[ID::milli]->getInt()) - 1000);            \
  auto a4Hz = pv[ID::pitchA4Hz]->getFloat() + float(100);                                \
  auto pitchBend = pv[ID::pitchBendRange]->getFloat() * pv[ID::pitchBend]->getFloat();   \
  auto centerPitch = std::log2(a4Hz / float(440));                                       \
  auto mainPitchInOctave                                                                 \
    = (centerPitch + octave + (semitone + milli + pitchBend) / eqTemp);                  \
  mainPitch.METHOD(std::exp2(mainPitchInOctave));                                        \
                                                                                         \
  auto fsRatio = float(48000) / sampleRate;                                              \
                                                                                         \
  for (size_t i0 = 0; i0 < nOscillator; ++i0) {                                          \
    envAttackKp[i0] = (float)EMAFilter<double>::secondToP(                               \
      sampleRate, pv[ID::envelopeAttackSecond0 + i0]->getDouble());                      \
    envDecayKp[i0] = (float)EMAFilter<double>::secondToP(                                \
      sampleRate, pv[ID::envelopeDecaySecond0 + i0]->getDouble());                       \
    envReleaseKp[i0] = (float)EMAFilter<double>::secondToP(                              \
      sampleRate, pv[ID::envelopeReleaseSecond0 + i0]->getDouble());                     \
                                                                                         \
    for (size_t i1 = 0; i1 < nWaveModInput; ++i1) {                                      \
      size_t offset = nWaveModInput * i0 + i1;                                           \
      waveModInput[i0][i1] = pv[ID::waveMod0Input0 + offset]->getFloat();                \
    }                                                                                    \
                                                                                         \
    tableParam[i0].waveInterpType = pv[ID::oscInterpolationType0 + i0]->getInt();        \
                                                                                         \
    tableParam[i0].oscPitch.METHOD(                                                      \
      std::exp2(pv[ID::oscPitchSemitone0 + i0]->getFloat() / eqTemp));                   \
    tableParam[i0].sumMix.METHOD(pv[ID::sumMix0 + i0]->getFloat());                      \
    tableParam[i0].feedbackLowpassKp.METHOD((float)EMAFilter<double>::cutoffToP(         \
      sampleRate, pv[ID::feedbackLowpassHz0 + i0]->getDouble()));                        \
    tableParam[i0].sumToImmediatePm.METHOD(pv[ID::sumToImmediatePm0 + i0]->getFloat());  \
    tableParam[i0].sumToAccumulatePm.METHOD(                                             \
      pv[ID::sumToAccumulatePm0 + i0]->getFloat() * fsRatio);                            \
    tableParam[i0].sumToFm.METHOD(pv[ID::sumToFm0 + i0]->getFloat());                    \
    tableParam[i0].sumToAm.METHOD(pv[ID::sumToAm0 + i0]->getFloat());                    \
                                                                                         \
    tableParam[i0].hardSync = pv[ID::hardSync0 + i0]->getFloat();                        \
    tableParam[i0].phaseSkew = pv[ID::phaseSkew0 + i0]->getFloat();                      \
    tableParam[i0].distortion = pv[ID::distortion0 + i0]->getFloat();                    \
    tableParam[i0].spectralSpread = pv[ID::spectralSpread0 + i0]->getFloat();            \
    tableParam[i0].phaseSlope = pv[ID::phaseSlope0 + i0]->getFloat();                    \
    tableParam[i0].spectralLowpass = pv[ID::spectralLowpass0 + i0]->getFloat();          \
    tableParam[i0].spectralHighpass = pv[ID::spectralHighpass0 + i0]->getFloat();        \
                                                                                         \
    for (size_t i1 = 0; i1 < ModID::MODID_ENUM_LENGTH; ++i1) {                           \
      size_t offset = i1 + i0 * ModID::MODID_ENUM_LENGTH;                                \
                                                                                         \
      tableParam[i0].pitch.METHOD##At(i1, pv[ID::modPitch0 + offset]->getFloat());       \
      tableParam[i0].immedaitePm.METHOD##At(                                             \
        i1, pv[ID::modImmediatePm0 + offset]->getFloat());                               \
      tableParam[i0].accumulatePm.METHOD##At(                                            \
        i1, pv[ID::modAccumulatePm0 + offset]->getFloat() * fsRatio);                    \
      tableParam[i0].fm.METHOD##At(i1, pv[ID::modFm0 + offset]->getFloat());             \
      tableParam[i0].modHardSync[i1] = pv[ID::modHardSync0 + offset]->getFloat();        \
      tableParam[i0].modPhaseSkew[i1] = pv[ID::modPhaseSkew0 + offset]->getFloat();      \
      tableParam[i0].modDistortion[i1] = pv[ID::modDistortion0 + offset]->getFloat();    \
      tableParam[i0].modSpectralSpread[i1]                                               \
        = pv[ID::modSpectralSpread0 + offset]->getFloat();                               \
      tableParam[i0].modPhaseSlope[i1] = pv[ID::modPhaseSlope0 + offset]->getFloat();    \
      tableParam[i0].modSpectralLowpass[i1]                                              \
        = pv[ID::modSpectralLowpass0 + offset]->getFloat();                              \
      tableParam[i0].modSpectralHighpass[i1]                                             \
        = pv[ID::modSpectralHighpass0 + offset]->getFloat();                             \
    }                                                                                    \
                                                                                         \
    for (size_t i1 = 0; i1 < nLfoWavetable; ++i1) {                                      \
      size_t offset = i1 + i0 * nLfoWavetable;                                           \
      lfoWavetable[i0].METHOD##At(i1, pv[ID::lfo0Waveform0 + offset]->getFloat());       \
    }                                                                                    \
                                                                                         \
    for (size_t i1 = 0; i1 < nOscWavetable; ++i1) {                                      \
      size_t offset = i1 + i0 * nOscWavetable;                                           \
      oscWavetable[i0].METHOD##At(i1, pv[ID::osc0Waveform0 + offset]->getFloat());       \
      oscWaveModGain[i0].METHOD##At(i1, pv[ID::waveMod0Gain0 + offset]->getFloat());     \
    }                                                                                    \
                                                                                         \
    envelopeSustainAmplitude[i0].METHOD(                                                 \
      pv[ID::envelopeSustainAmplitude0 + i0]->getFloat());                               \
    lfoPhaseDelta[i0].METHOD(getLfoPhaseDelta(i0, sampleRate, param));                   \
    const auto lfoLpHz = pv[ID::lfoLowpassHz0 + i0]->getDouble();                        \
    lfoLowpassKp[i0].METHOD(                                                             \
      lfoLpHz >= Scales::lfoLowpassHz.getMax()                                           \
        ? float(1)                                                                       \
        : (float)EMAFilter<double>::cutoffToP(sampleRate, lfoLpHz));                     \
                                                                                         \
    externalInput[i0].METHOD(pv[ID::externalInput0 + i0]->getFloat());                   \
  }                                                                                      \
                                                                                         \
  gainAttackKp = (float)EMAFilter<double>::secondToP(                                    \
    sampleRate, pv[ID::gainAttackSecond]->getDouble());                                  \
  gainDecayKp = (float)EMAFilter<double>::secondToP(                                     \
    sampleRate, pv[ID::gainDecaySecond]->getDouble());                                   \
  gainReleaseKp = (float)EMAFilter<double>::secondToP(                                   \
    sampleRate, pv[ID::gainReleaseSecond]->getDouble());                                 \
  gainSustainAmplitude.METHOD(pv[ID::gainSustainAmplitude]->getFloat());                 \
                                                                                         \
  oscMix.METHOD(pv[ID::oscMix]->getFloat());

struct NoteProcessInfo {
  std::array<WavetableParameter, nOscillator> tableParam;

  std::array<float, nOscillator> envAttackKp{};
  std::array<float, nOscillator> envDecayKp{};
  std::array<float, nOscillator> envReleaseKp{};
  std::array<std::array<float, nWaveModInput>, nOscillator> waveModInput{};
  std::array<ExpSmoother<float>, nOscillator> envelopeSustainAmplitude;
  std::array<ExpSmoother<float>, nOscillator> lfoPhaseDelta;
  std::array<ExpSmoother<float>, nOscillator> lfoLowpassKp;
  std::array<ExpSmoother<float>, nOscillator> externalInput;
  std::array<ParallelExpSmoother<float, nLfoWavetable>, nOscillator> lfoWavetable;
  std::array<ParallelExpSmoother<float, nOscWavetable>, nOscillator> oscWavetable;
  std::array<ParallelExpSmoother<float, nOscWavetable>, nOscillator> oscWaveModGain;
  float gainAttackKp = 1;
  float gainDecayKp = 1;
  float gainReleaseKp = 1;
  float eqTemp = float(12);
  ExpSmoother<float> gainSustainAmplitude;
  ExpSmoother<float> oscMix;
  ExpSmoother<float> mainPitch;

  inline float getLfoPhaseDelta(size_t oscIndex, float sampleRate, GlobalParameter &param)
  {
    using ID = ParameterID::ID;
    auto &pv = param.value;

    return pv[ID::lfoRate0 + oscIndex]->getFloat() / sampleRate;
  }

  void reset(float sampleRate, GlobalParameter &param)
  {
    NOTE_PROCESS_INFO_SMOOTHER(reset);
  }

  void setParameters(float sampleRate, GlobalParameter &param)
  {
    NOTE_PROCESS_INFO_SMOOTHER(push);
  }

  void process()
  {
    for (auto &x : tableParam) {
      x.oscPitch.process();
      x.sumMix.process();
      x.feedbackLowpassKp.process();
      x.sumToImmediatePm.process();
      x.sumToAccumulatePm.process();
      x.sumToFm.process();
      x.sumToAm.process();

      x.pitch.process();
      x.immedaitePm.process();
      x.accumulatePm.process();
      x.fm.process();
    }

    for (auto &x : envelopeSustainAmplitude) x.process();
    for (auto &x : lfoPhaseDelta) x.process();
    for (auto &x : lfoLowpassKp) x.process();
    for (auto &x : externalInput) x.process();
    for (auto &x : lfoWavetable) x.process();
    for (auto &x : oscWavetable) x.process();
    for (auto &x : oscWaveModGain) x.process();

    gainSustainAmplitude.process();
    oscMix.process();
    mainPitch.process();
  }
};

class Note {
public:
  NoteState state = NoteState::rest;

  int_fast32_t id = -1;
  float notePitch = 1;
  float velocity = 0;
  float noteHz = 100;
  float pan = float(0.5);
  float gain = 0;

  using WaveModDelay = FixedIntDelay<float, maxWaveModDelay>;

  std::array<float, ModID::MODID_ENUM_LENGTH> modulation{};
  std::array<float, nOscillator> feedback{};
  ADSREnvelope<float> gainEnvelope;
  std::array<ADSREnvelope<float>, nOscillator> envelope;
  std::array<ExpSmoother<float>, nOscillator> lfoPitch;
  std::array<EasyLfo<float, nLfoWavetable>, nOscillator> lfo;
  std::array<std::array<WaveModDelay, nOscWavetable>, nOscillator> waveModDelay;
  std::array<std::array<float, nOscWavetable>, nOscillator> wavetable;
  std::array<VariableWaveTableOscillator, nOscillator> oscillator;

  void setup(float sampleRate);
  void reset();
  void setParameters(float sampleRate, NoteProcessInfo &info, GlobalParameter &param);
  void noteOn(
    int_fast32_t noteId,
    float notePitch,
    float velocity,
    float pan,
    float sampleRate,
    NoteProcessInfo &info,
    GlobalParameter &param);
  void noteOff(float sampleRate);
  void rest();
  bool isAttacking();
  float getGain();
  std::array<float, 2> process(float sampleRate, NoteProcessInfo &info);
};

class DSPCore {
public:
  struct MidiNote {
    bool isNoteOn;
    size_t frame;
    int_fast32_t id;
    int_fast16_t pitch;
    float tuning;
    float velocity;
  };

  GlobalParameter param;
  bool isPlaying = false;
  float tempo = 120.0f;
  double beatsElapsed = 0.0f;

  DSPCore();

  void setup(double sampleRate);
  void reset();
  void startup();
  void setParameters();
  void process(const size_t length, float *out0, float *out1);
  void noteOn(int_fast32_t noteId, int_fast16_t pitch, float tuning, float velocity);
  void noteOff(int_fast32_t noteId);

  std::vector<MidiNote> midiNotes;

  void pushMidiNote(
    bool isNoteOn,
    size_t frame,
    int_fast32_t noteId,
    int_fast16_t pitch,
    float tuning,
    float velocity)
  {
    MidiNote note;
    note.isNoteOn = isNoteOn;
    note.frame = frame;
    note.id = noteId;
    note.pitch = pitch;
    note.tuning = tuning;
    note.velocity = velocity;
    midiNotes.push_back(note);
  }

  void processMidiNote(size_t frame)
  {
    while (true) {
      auto it = std::find_if(midiNotes.begin(), midiNotes.end(), [&](const MidiNote &nt) {
        return nt.frame == frame;
      });
      if (it == std::end(midiNotes)) return;
      if (it->isNoteOn)
        noteOn(it->id, it->pitch, it->tuning, it->velocity);
      else
        noteOff(it->id);
      midiNotes.erase(it);
    }
  }

private:
  inline void fillTransitionBuffer(size_t noteIndex);
  inline void updateSampleRate(bool reset = false);
  void processSample(std::array<float, 2> &frame);

  using DownSamplerType = DownSampler<double, Sos16FoldFirstStage<double>>;

  static constexpr std::array<size_t, 3> fold{1, 2, DownSamplerType::fold};

  float sampleRate = 44100.0f;
  float upRate = DownSamplerType::fold * 44100.0f;
  float velocity = 0.0f;
  DecibelScale<float> velocityMap{-60, 0, true};

  size_t nVoice = maxVoice;
  std::vector<size_t> noteIndices;
  std::vector<size_t> voiceIndices;
  std::vector<float> unisonPan;
  std::array<Note, maxVoice> notes;

  NoteProcessInfo info;
  bool dcHighpassEnable = false;
  ExpSmoother<float> interpMasterGain;
  ExpSmoother<float> dcHighpassCutoffKp;

  std::array<EMAHighpass<double>, 2> dcHighpass;
  std::array<DownSamplerType, 2> downSampler;

  std::vector<std::array<float, 2>> transitionBuffer{};
  bool isTransitioning = false;
  size_t trIndex = 0;
  size_t trStop = 0;
};

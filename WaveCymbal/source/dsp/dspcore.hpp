// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../parameter.hpp"
#include "ksstring.hpp"

#include <array>
#include <cmath>
#include <vector>

using namespace SomeDSP;
using namespace Steinberg::Synth;

struct NoteInfo {
  int32_t id;
  float frequency;
  float velocity;
};

class DSPCore {
public:
  DSPCore();

  static const size_t maxVoice = 32;
  GlobalParameter param;

  void setup(double sampleRate);
  void reset();   // Stop sounds.
  void startup(); // Reset phase, random seed etc.
  void setParameters();
  void process(
    const size_t length, const float *in0, const float *in1, float *out0, float *out1);
  void noteOn(int32_t noteId, int16_t pitch, float tuning, float velocity);
  void noteOff(int32_t noteId);

  struct MidiNote {
    bool isNoteOn;
    uint32_t frame;
    int32_t id;
    int16_t pitch;
    float tuning;
    float velocity;
  };

  std::vector<MidiNote> midiNotes;

  void pushMidiNote(
    bool isNoteOn,
    uint32_t frame,
    int32_t noteId,
    int16_t pitch,
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
  void setSystem();

  float sampleRate = 44100.0f;

  // Top of this stack is current note.
  std::vector<NoteInfo> noteStack;

  Pulsar<float> pulsar{44100.0f, 0};
  VelvetNoise<float> velvetNoise{44100.0f, 100.0f, 0};
  Brown<float> brownNoise{0};

  Random<float> rnd{0};
  Excitor<float> excitor;
  WaveHat<float> cymbal;

  // debug
  bool trigger = false;

  LinearSmoother<float> interpMasterGain;
  LinearSmoother<float> interpPitch;
};

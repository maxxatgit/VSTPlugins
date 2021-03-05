// (c) 2019-2020 Takamitsu Endo
//
// This file is part of FDNCymbal.
//
// FDNCymbal is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// FDNCymbal is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with FDNCymbal.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../common/dsp/constants.hpp"
#include "../../../common/dsp/smoother.hpp"
#include "../parameter.hpp"
#include "delay.hpp"
#include "envelope.hpp"
#include "iir.hpp"
#include "oscillator.hpp"

#include <array>
#include <cmath>
#include <memory>
#include <vector>

using namespace SomeDSP;
using namespace Steinberg::Synth;

constexpr size_t fdnMatrixSize = 12;
constexpr size_t nAP1 = 8;
constexpr size_t nAP2 = 8;
constexpr double highpassQ = 0.01;
constexpr float tremoloDelayMaxTime = 0.001f;

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
  float sampleRate = 44100.0f;

  float velocity = 0;
  std::vector<NoteInfo> noteStack; // Top of this stack is current note.

  Random<float> rng{0};
  Random<float> rngStick{0};
  Random<float> rngTremolo{0};
  Pulsar<float> pulsar{44100.0f, 0.0f};

  ExpDecay<float> stickEnvelope;
  std::array<BiquadOsc<float>, 16> stickOscillator;
  VelvetNoise<float> velvet;

  float fdnSig = 0.0f;
  std::array<FeedbackDelayNetwork<float, fdnMatrixSize>, 8> fdnCascade;

  float serialAP1Sig = 0.0f;
  SerialAllpass<float, nAP1> serialAP1;
  BiquadHighPass<double> serialAP1Highpass;

  float serialAP2Sig = 0.0f;
  std::array<SerialAllpass<float, nAP2>, 4> serialAP2;
  BiquadHighPass<double> serialAP2Highpass;

  Delay<float> tremoloDelay;
  float tremoloPhase = 0.0f;
  float randomTremoloDepth = 0.0f;
  float randomTremoloFrequency = 0.0f;
  float randomTremoloDelayTime = 0.0f;

  LinearSmoother<float> interpPitch;
  LinearSmoother<float> interpStickToneMix;
  LinearSmoother<float> interpStickPulseMix;
  LinearSmoother<float> interpStickVelvetMix;
  LinearSmoother<float> interpFDNFeedback;
  LinearSmoother<float> interpFDNCascadeMix;
  LinearSmoother<float> interpAllpassMix;
  LinearSmoother<float> interpAllpass1Feedback;
  LinearSmoother<float> interpAllpass2Feedback;
  LinearSmoother<float> interpTremoloMix;
  LinearSmoother<float> interpTremoloDepth;
  LinearSmoother<float> interpTremoloFrequency;
  LinearSmoother<float> interpTremoloDelayTime;
  LinearSmoother<float> interpMasterGain;
};

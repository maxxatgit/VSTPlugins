// Original by:
// (c) 2018, Steinberg Media Technologies GmbH, All Rights Reserved
//
// Modified by:
// (c) 2021 Takamitsu Endo
//
// This file is part of BasicLimiter.
//
// BasicLimiter is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// BasicLimiter is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with BasicLimiter.  If not, see <https://www.gnu.org/licenses/>.

#include "plugprocessor.hpp"
#include "fuid.hpp"

#include "base/source/fstreamer.h"
#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivstevents.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

#include "../../lib/vcl/vectorclass.h"

#include <iostream>

namespace Steinberg {
namespace Synth {

PlugProcessor::PlugProcessor()
{
  auto iset = instrset_detect();
  if (iset >= 10) {
    dsp = std::make_unique<DSPCore_AVX512>();
  } else if (iset >= 8) {
    dsp = std::make_unique<DSPCore_AVX2>();
  } else if (iset >= 7) {
    dsp = std::make_unique<DSPCore_AVX>();
  } else {
    std::cerr << "\nError: Instruction set AVX or later not supported on this computer";
    exit(EXIT_FAILURE);
  }

  setControllerClass(ControllerUID);
}

tresult PLUGIN_API PlugProcessor::initialize(FUnknown *context)
{
  tresult result = AudioEffect::initialize(context);
  if (result != kResultTrue) return result;

  addAudioInput(STR16("StereoInput"), Vst::SpeakerArr::kStereo);
  addAudioOutput(STR16("StereoOutput"), Vst::SpeakerArr::kStereo);

  return result;
}

tresult PLUGIN_API PlugProcessor::setBusArrangements(
  Vst::SpeakerArrangement *inputs,
  int32 numIns,
  Vst::SpeakerArrangement *outputs,
  int32 numOuts)
{
  if (numIns == 1 && numOuts == 1 && inputs[0] == outputs[0]) {
    return AudioEffect::setBusArrangements(inputs, numIns, outputs, numOuts);
  }
  return kResultFalse;
}

uint32 PLUGIN_API PlugProcessor::getProcessContextRequirements()
{
  using Rq = Vst::IProcessContextRequirements;

  return Rq::kNeedTransportState;
}

tresult PLUGIN_API PlugProcessor::setupProcessing(Vst::ProcessSetup &setup)
{
  if (dsp == nullptr) return kNotInitialized;
  dsp->setup(processSetup.sampleRate);
  return AudioEffect::setupProcessing(setup);
}

tresult PLUGIN_API PlugProcessor::setActive(TBool state)
{
  if (dsp == nullptr) return kResultFalse;
  if (state) {
    dsp->setup(processSetup.sampleRate);
  } else {
    dsp->reset();
    lastState = 0;
  }
  return AudioEffect::setActive(state);
}

uint32 PLUGIN_API PlugProcessor::getLatencySamples() { return uint32(dsp->getLatency()); }

tresult PLUGIN_API PlugProcessor::process(Vst::ProcessData &data)
{
  if (dsp == nullptr) return kNotInitialized;

  // Read inputs parameter changes.
  if (data.inputParameterChanges) {
    int32 parameterCount = data.inputParameterChanges->getParameterCount();
    for (int32 index = 0; index < parameterCount; index++) {
      auto queue = data.inputParameterChanges->getParameterData(index);
      if (!queue) continue;
      Vst::ParamValue value;
      int32 sampleOffset;
      if (queue->getPoint(queue->getPointCount() - 1, sampleOffset, value) != kResultTrue)
        continue;
      size_t id = queue->getParameterId();
      if (id < dsp->param.value.size()) dsp->param.value[id]->setFromNormalized(value);
    }
  }

  if (data.processContext == nullptr) return kResultOk;

  uint64_t state = data.processContext->state;
  if (
    (lastState & Vst::ProcessContext::kPlaying) == 0
    && (state & Vst::ProcessContext::kPlaying) != 0) {
    dsp->startup();
  }
  lastState = state;

  dsp->setParameters();

  if (data.numInputs == 0) return kResultOk;
  if (data.numOutputs == 0) return kResultOk;
  if (data.numSamples <= 0) return kResultOk;
  if (data.inputs[0].numChannels != 2) return kResultOk;
  if (data.outputs[0].numChannels != 2) return kResultOk;
  if (data.symbolicSampleSize == Vst::kSample64) return kResultOk;

  auto isBypassing = dsp->param.value[ParameterID::bypass]->getInt();
  if (isBypassing) {
    if (!wasBypassing) dsp->reset();
    processBypass(data);
  } else {
    float *in0 = data.inputs[0].channelBuffers32[0];
    float *in1 = data.inputs[0].channelBuffers32[1];
    float *out0 = data.outputs[0].channelBuffers32[0];
    float *out1 = data.outputs[0].channelBuffers32[1];
    dsp->process((size_t)data.numSamples, in0, in1, out0, out1);
  }
  wasBypassing = isBypassing;

  return kResultOk;
}

void PlugProcessor::processBypass(Vst::ProcessData &data)
{
  float **in = data.inputs[0].channelBuffers32;
  float **out = data.outputs[0].channelBuffers32;
  for (int32_t ch = 0; ch < data.inputs[0].numChannels; ch++) {
    if (in[ch] != out[ch]) memcpy(out[ch], in[ch], data.numSamples * sizeof(float));
  }
}

tresult PLUGIN_API PlugProcessor::setState(IBStream *state)
{
  if (dsp == nullptr) return kNotInitialized;
  if (!state) return kResultFalse;
  return dsp->param.setState(state);
}

tresult PLUGIN_API PlugProcessor::getState(IBStream *state)
{
  if (dsp == nullptr) return kNotInitialized;
  return dsp->param.getState(state);
}

} // namespace Synth
} // namespace Steinberg

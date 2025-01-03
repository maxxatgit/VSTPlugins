// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../../common/parameterInterface.hpp"

#ifdef TEST_DSP
  #include "../../test/value.hpp"
#else
  #include "../../common/value.hpp"
#endif

namespace Steinberg {
namespace Synth {

namespace ParameterID {
enum ID {
  bypass,

  inputGain,
  clipGain,
  outputGain,
  add,
  mul,
  moreAdd,
  moreMul,
  type,
  hardclip,

  lowpass,
  lowpassCutoff,

  smoothness,

  limiter,
  limiterThreshold,
  limiterAttack, // Deprecated but not removed for backward compatibility.
  limiterRelease,

  guiInputGain,

  ID_ENUM_LENGTH,
  ID_ENUM_GUI_START = guiInputGain,
};
} // namespace ParameterID

struct Scales {
  static SomeDSP::UIntScale<double> boolScale;
  static SomeDSP::LinearScale<double> defaultScale;

  static SomeDSP::LogScale<double> inputGain;
  static SomeDSP::LinearScale<double> add;
  static SomeDSP::LinearScale<double> mul;
  static SomeDSP::LinearScale<double> moreAdd;
  static SomeDSP::LinearScale<double> moreMul;
  static SomeDSP::LogScale<double> outputGain;
  static SomeDSP::UIntScale<double> type;

  static SomeDSP::LogScale<double> lowpassCutoff;

  static SomeDSP::LogScale<double> smoothness;

  static SomeDSP::LogScale<double> limiterThreshold;
  static SomeDSP::LogScale<double> limiterAttack; // Deprecated.
  static SomeDSP::LogScale<double> limiterRelease;

  static SomeDSP::LinearScale<double> guiInputGainScale;
};

struct GlobalParameter : public ParameterInterface {
  std::vector<std::unique_ptr<ValueInterface>> value;

  GlobalParameter()
  {
    value.resize(ParameterID::ID_ENUM_LENGTH);

    using Info = Vst::ParameterInfo;
    using ID = ParameterID::ID;
    using LinearValue = DoubleValue<SomeDSP::LinearScale<double>>;
    using LogValue = DoubleValue<SomeDSP::LogScale<double>>;
    // using DecibelValue = DoubleValue<SomeDSP::DecibelScale<double>>;

    value[ID::bypass] = std::make_unique<UIntValue>(
      0, Scales::boolScale, "bypass", Info::kCanAutomate | Info::kIsBypass);

    value[ID::inputGain] = std::make_unique<LogValue>(
      0.5, Scales::inputGain, "inputGain", Info::kCanAutomate);
    value[ID::add]
      = std::make_unique<LinearValue>(1.0, Scales::add, "add", Info::kCanAutomate);
    value[ID::mul]
      = std::make_unique<LinearValue>(1.0, Scales::mul, "mul", Info::kCanAutomate);
    value[ID::moreAdd] = std::make_unique<LinearValue>(
      0.0, Scales::moreAdd, "moreAdd", Info::kCanAutomate);
    value[ID::moreMul] = std::make_unique<LinearValue>(
      0.0, Scales::moreMul, "moreMul", Info::kCanAutomate);
    value[ID::clipGain] = std::make_unique<LogValue>(
      1.0, Scales::outputGain, "clipGain", Info::kCanAutomate);
    value[ID::outputGain] = std::make_unique<LogValue>(
      0.5, Scales::outputGain, "outputGain", Info::kCanAutomate);
    value[ID::type]
      = std::make_unique<UIntValue>(1, Scales::type, "type", Info::kCanAutomate);
    value[ID::hardclip] = std::make_unique<UIntValue>(
      false, Scales::boolScale, "hardclip", Info::kCanAutomate);

    value[ID::lowpass] = std::make_unique<UIntValue>(
      true, Scales::boolScale, "lowpass", Info::kCanAutomate);
    value[ID::lowpassCutoff] = std::make_unique<LogValue>(
      1.0, Scales::lowpassCutoff, "lowpassCutoff", Info::kCanAutomate);

    value[ID::smoothness] = std::make_unique<LogValue>(
      0.1, Scales::smoothness, "smoothness", Info::kCanAutomate);

    value[ID::limiter]
      = std::make_unique<UIntValue>(1, Scales::boolScale, "limiter", Info::kCanAutomate);
    value[ID::limiterThreshold] = std::make_unique<LogValue>(
      Scales::limiterThreshold.invmap(1.0), Scales::limiterThreshold, "limiterThreshold",
      Info::kCanAutomate);
    value[ID::limiterAttack] = std::make_unique<LogValue>(
      Scales::limiterAttack.invmap(0.002), Scales::limiterAttack,
      "limiterAttack (deprecated)", Info::kIsHidden);
    value[ID::limiterRelease] = std::make_unique<LogValue>(
      Scales::limiterRelease.invmap(0.005), Scales::limiterRelease, "limiterRelease",
      Info::kCanAutomate);

    value[ID::guiInputGain] = std::make_unique<LinearValue>(
      0.0, Scales::guiInputGainScale, "guiInputGain", Info::kIsReadOnly);

    for (size_t id = 0; id < value.size(); ++id) value[id]->setId(Vst::ParamID(id));
  }

#ifdef TEST_DSP
  // Not used in DSP test.
  double getDefaultNormalized(int32_t) { return 0.0; }

#else
  tresult setState(IBStream *stream)
  {
    IBStreamer streamer(stream, kLittleEndian);
    for (auto &val : value)
      if (val->setState(streamer)) return kResultFalse;
    return kResultOk;
  }

  tresult getState(IBStream *stream)
  {
    IBStreamer streamer(stream, kLittleEndian);
    for (auto &val : value)
      if (val->getState(streamer)) return kResultFalse;
    return kResultOk;
  }

  tresult addParameter(Vst::ParameterContainer &parameters)
  {
    for (auto &val : value)
      if (val->addParameter(parameters)) return kResultFalse;
    return kResultOk;
  }

  double getDefaultNormalized(int32_t tag) override
  {
    if (size_t(abs(tag)) >= value.size()) return 0.0;
    return value[tag]->getDefaultNormalized();
  }
#endif
};

} // namespace Synth
} // namespace Steinberg

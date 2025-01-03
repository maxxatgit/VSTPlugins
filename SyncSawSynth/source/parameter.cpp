// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "parameter.hpp"

namespace Steinberg {
namespace Synth {

using namespace SomeDSP;

UIntScale<double> Scales::boolScale(1);
LinearScale<double> Scales::defaultScale(0.0, 1.0);

LogScale<double> Scales::oscGain(0.0, 1.0, 0.75, 0.5);
LinearScale<double> Scales::semi(-24.0, 24.0);
LinearScale<double> Scales::cent(-100.0, 100.0);
LinearScale<double> Scales::sync(0.01, 16.0);
UIntScale<double> Scales::osc1SyncType(3);
UIntScale<double> Scales::osc1PTROrder(16);

LogScale<double> Scales::fmToSync(0.0, 1.0, 0.5, 0.1);
LogScale<double> Scales::fmToFreq(0.0, 1.0, 0.5, 0.1);

LogScale<double> Scales::gain(0.0, 4.0, 0.75, 1.0);

LogScale<double> Scales::envelopeA(0.0001, 16.0, 0.5, 1.0);
LogScale<double> Scales::envelopeD(0.0001, 16.0, 0.5, 1.0);
LogScale<double> Scales::envelopeS(0.0, 0.9995, 0.5, 0.3);
LogScale<double> Scales::envelopeR(0.001, 16.0, 0.5, 2.0);

LogScale<double> Scales::filterCutoff(20.0, 20000.0, 0.5, 800.0);
LogScale<double> Scales::filterResonance(0.001, 1.0, 0.5, 0.3);
LogScale<double> Scales::filterFeedback(0.0, 1.0, 0.5, 0.3);
LogScale<double> Scales::filterSaturation(0.01, 8.0, 0.3, 0.5);
UIntScale<double> Scales::filterType(4);
UIntScale<double> Scales::filterShaper(3);
LinearScale<double> Scales::filterCutoffAmount(-1.0, 1.0);
LinearScale<double> Scales::filterKeyMod(-1.0, 1.0);

LogScale<double> Scales::modEnvelopeA(0.0, 4.0, 0.5, 0.5);
LogScale<double> Scales::modEnvelopeCurve(1.0, 96.0, 0.5, 8.0);
LogScale<double> Scales::modLFOFrequency(0.01, 20.0, 0.5, 1.0);
LogScale<double> Scales::modToFreq(0.0, 16.0, 0.5, 1.0);
LogScale<double> Scales::modToSync(0.0, 16.0, 0.5, 1.0);

UIntScale<double> Scales::nVoice(5);

} // namespace Synth
} // namespace Steinberg

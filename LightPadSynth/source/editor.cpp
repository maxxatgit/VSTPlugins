// (c) 2020 Takamitsu Endo
//
// This file is part of LightPadSynth.
//
// LightPadSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LightPadSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with LightPadSynth.  If not, see <https://www.gnu.org/licenses/>.

#include "editor.hpp"
#include "version.hpp"

#include <algorithm>
#include <sstream>

enum tabIndex { tabMain, tabPadSynth, tabInfo };

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

Editor::Editor(void *controller) : PlugEditor(controller)
{
  param = std::make_unique<Synth::GlobalParameter>();

  uiTextSize = 14.0f;
  midTextSize = 16.0f;
  pluginNameTextSize = 22.0f;
  margin = 5.0f;
  labelHeight = 20.0f;
  labelY = 30.0f;
  knobWidth = 50.0f;
  knobHeight = 40.0f;
  knobX = 60.0f; // With margin.
  knobY = knobHeight + labelY;
  barboxWidth = 12.0f * knobX;
  barboxHeight = 2.0f * knobY;
  barboxY = barboxHeight + 2.0f * margin;
  checkboxWidth = 60.0f;
  splashHeight = 40.0f;
  defaultWidth = int32(barboxWidth + labelY + 2 * knobX + 12 * margin + 40);
  defaultHeight = int32(40 + labelY + 4 * barboxY + 9 * margin);
  viewRect = ViewRect{0, 0, defaultWidth, defaultHeight};
  setRect(viewRect);
}

bool Editor::prepareUI()
{
  using ID = Synth::ParameterID::ID;
  using Scales = Synth::Scales;

  const auto top0 = 20.0f;
  const auto left0 = 20.0f;

  // TabView test.
  const auto tabTop0 = top0;
  const auto tabLeft0 = left0;

  std::vector<std::string> tabs = {"Main", "Wavetable", "Information"};
  const auto tabWidth = defaultWidth - 40.0f;
  const auto tabHeight = labelY + 4.0f * barboxY + 9.0f * margin;
  auto tabview = new TabView(
    tabs, new CFontDesc(PlugEditorStyle::fontName(), fontSize, CTxtFace::kNormalFace),
    labelY, CRect(tabLeft0, tabTop0, tabLeft0 + tabWidth, tabTop0 + tabHeight));
  tabview->setHighlightColor(colorBlue);
  frame->addView(tabview);

  const auto tabInsideTop0 = tabTop0 + labelY + 4.0f * margin;
  const auto tabInsideLeft0 = tabLeft0 + 4.0f * margin;

  // Gain.
  const auto gainTop = tabInsideTop0;
  const auto gainLeft = tabInsideLeft0 + labelY;
  tabview->addWidget(tabMain, addGroupLabel(gainLeft, gainTop, 6.0f * knobX, "Gain"));
  const auto gainKnobTop = gainTop + labelY;

  tabview->addWidget(
    tabMain, addKnob(gainLeft, gainKnobTop, knobWidth, colorBlue, "Gain", ID::gain));
  tabview->addWidget(
    tabMain,
    addKnob(gainLeft + 1.0f * knobX, gainKnobTop, knobWidth, colorBlue, "A", ID::gainA));
  tabview->addWidget(
    tabMain,
    addKnob(gainLeft + 2.0f * knobX, gainKnobTop, knobWidth, colorBlue, "D", ID::gainD));
  tabview->addWidget(
    tabMain,
    addKnob(gainLeft + 3.0f * knobX, gainKnobTop, knobWidth, colorBlue, "S", ID::gainS));
  tabview->addWidget(
    tabMain,
    addKnob(gainLeft + 4.0f * knobX, gainKnobTop, knobWidth, colorBlue, "R", ID::gainR));
  tabview->addWidget(
    tabMain,
    addKnob(
      gainLeft + 5.0f * knobX, gainKnobTop, knobWidth, colorBlue, "Curve",
      ID::gainCurve));

  // Filter.
  const auto filterTop = gainTop;
  const auto filterLeft = gainLeft + 6.0f * knobX + 2.0f * margin;
  tabview->addWidget(
    tabMain, addGroupLabel(filterLeft, filterTop, 8.0f * knobX, "Filter"));

  const auto filterTop0 = filterTop + labelY;
  tabview->addWidget(
    tabMain,
    addKnob(filterLeft, filterTop0, knobWidth, colorBlue, "Cutoff", ID::filterCutoff));
  tabview->addWidget(
    tabMain,
    addKnob(
      filterLeft + 1.0f * knobX, filterTop0, knobWidth, colorBlue, "Resonance",
      ID::filterResonance));

  tabview->addWidget(
    tabMain,
    addKnob(
      filterLeft + 2.0f * knobX, filterTop0, knobWidth, colorBlue, "A", ID::filterA));
  tabview->addWidget(
    tabMain,
    addKnob(
      filterLeft + 3.0f * knobX, filterTop0, knobWidth, colorBlue, "D", ID::filterD));
  tabview->addWidget(
    tabMain,
    addKnob(
      filterLeft + 4.0f * knobX, filterTop0, knobWidth, colorBlue, "S", ID::filterS));
  tabview->addWidget(
    tabMain,
    addKnob(
      filterLeft + 5.0f * knobX, filterTop0, knobWidth, colorBlue, "R", ID::filterR));
  tabview->addWidget(
    tabMain,
    addKnob(
      filterLeft + 6.0f * knobX, filterTop0, knobWidth, colorBlue, "Amount",
      ID::filterAmount));
  tabview->addWidget(
    tabMain,
    addKnob(
      filterLeft + 7.0f * knobX, filterTop0, knobWidth, colorBlue, "KeyFollow",
      ID::filterKeyFollow));

  // Tuning.
  const auto tuningTop = tabInsideTop0 + labelY + knobY;
  const auto tuningLeft = gainLeft;
  tabview->addWidget(
    tabMain,
    addGroupLabel(tuningLeft, tuningTop, 4.0f * knobX - 4.0f * margin, "Tuning"));

  const auto tuningLabelWidth = knobX - 2.0f * margin;
  const auto tuningLeft1 = tuningLeft;
  const auto tuningLeft2 = tuningLeft1 + tuningLabelWidth;

  const auto tuningTop1 = tuningTop + labelY;
  tabview->addWidget(
    tabMain, addLabel(tuningLeft1, tuningTop1, tuningLabelWidth, "Octave"));
  tabview->addWidget(
    tabMain,
    addTextKnob(
      tuningLeft2, tuningTop1, knobWidth, colorBlue, ID::oscOctave, Scales::oscOctave,
      false, 0, -12));

  const auto tuningTop2 = tuningTop1 + labelY;
  tabview->addWidget(
    tabMain, addLabel(tuningLeft1, tuningTop2, tuningLabelWidth, "Semi"));
  tabview->addWidget(
    tabMain,
    addTextKnob(
      tuningLeft2, tuningTop2, knobWidth, colorBlue, ID::oscSemi, Scales::oscSemi, false,
      0, -120));

  const auto tuningTop3 = tuningTop2 + labelY;
  tabview->addWidget(
    tabMain, addLabel(tuningLeft1, tuningTop3, tuningLabelWidth, "Milli"));
  auto knobOscMilli = addTextKnob(
    tuningLeft2, tuningTop3, knobWidth, colorBlue, ID::oscMilli, Scales::oscMilli, false,
    0, -1000);
  knobOscMilli->sensitivity = 0.001f;
  knobOscMilli->lowSensitivity = 0.00025f;
  tabview->addWidget(tabMain, knobOscMilli);

  const auto tuningLeft3 = tuningLeft + 2.0f * knobX - 2.0f * margin;
  const auto tuningLeft4 = tuningLeft3 + tuningLabelWidth;

  const auto tuningTop4 = tuningTop + 1.0f * labelY;
  tabview->addWidget(tabMain, addLabel(tuningLeft3, tuningTop4, tuningLabelWidth, "ET"));
  tabview->addWidget(
    tabMain,
    addTextKnob(
      tuningLeft4, tuningTop4, knobWidth, colorBlue, ID::equalTemperament,
      Scales::equalTemperament, false, 0, 1));

  const auto tuningTop5 = tuningTop4 + labelY;
  tabview->addWidget(
    tabMain, addLabel(tuningLeft3, tuningTop5, tuningLabelWidth, "A4 [Hz]"));
  tabview->addWidget(
    tabMain,
    addTextKnob(
      tuningLeft4, tuningTop5, knobWidth, colorBlue, ID::pitchA4Hz, Scales::pitchA4Hz,
      false, 0, 100));

  // Unison.
  const auto unisonTop = tuningTop;
  const auto unisonLeft = tuningLeft + 4.0f * knobX;
  tabview->addWidget(
    tabMain, addGroupLabel(unisonLeft, unisonTop, 5.0f * knobX, "Unison"));
  const auto unisonKnobTop = unisonTop + labelY;
  tabview->addWidget(tabMain, addLabel(unisonLeft, unisonKnobTop, knobWidth, "nUnison"));
  tabview->addWidget(
    tabMain,
    addTextKnob(
      unisonLeft, unisonKnobTop + labelHeight + margin, knobWidth, colorBlue, ID::nUnison,
      Scales::nUnison, false, 0, 1));
  tabview->addWidget(
    tabMain,
    addKnob(
      unisonLeft + knobX, unisonKnobTop, knobWidth, colorBlue, "Detune",
      ID::unisonDetune));
  tabview->addWidget(
    tabMain,
    addKnob(
      unisonLeft + 2.0f * knobX, unisonKnobTop, knobWidth, colorBlue, "GainRnd",
      ID::unisonGainRandom));
  tabview->addWidget(
    tabMain,
    addKnob(
      unisonLeft + 3.0f * knobX, unisonKnobTop, knobWidth, colorBlue, "Phase",
      ID::unisonPhase));
  tabview->addWidget(
    tabMain,
    addKnob(
      unisonLeft + 4.0f * knobX, unisonKnobTop, knobWidth, colorBlue, "Spread",
      ID::unisonPan));
  tabview->addWidget(
    tabMain,
    addCheckbox(
      unisonLeft + 1.0f * knobX - 1.5f * margin, unisonKnobTop + knobY, knobWidth,
      "Random Detune", ID::unisonDetuneRandom));

  std::vector<UTF8String> unisonPanTypeOptions{
    "Alternate L-R", "Alternate M-S", "Ascend L -> R", "Ascend R -> L", "High on Mid",
    "High on Side",  "Random",        "Rotate L",      "Rotate R",      "Shuffle"};
  tabview->addWidget(
    tabMain,
    addOptionMenu(
      unisonLeft + 3.5f * knobX + margin, unisonKnobTop + knobY, 2.0f * knobWidth,
      ID::unisonPanType, unisonPanTypeOptions));

  // Phase.
  const auto phaseTop = unisonTop;
  const auto phaseLeft = unisonLeft + 5.0f * knobX + 2.0f * margin;
  tabview->addWidget(tabMain, addGroupLabel(phaseLeft, phaseTop, 2.0f * knobX, "Phase"));
  const auto phaseKnobTop = phaseTop + labelY;

  tabview->addWidget(
    tabMain,
    addKnob(phaseLeft, phaseKnobTop, knobWidth, colorBlue, "Phase", ID::oscInitialPhase));

  const auto phaseLeft1 = phaseLeft + knobX;
  tabview->addWidget(
    tabMain,
    addCheckbox(phaseLeft1, phaseKnobTop, knobWidth, "Reset", ID::oscPhaseReset));
  tabview->addWidget(
    tabMain,
    addCheckbox(
      phaseLeft1, phaseKnobTop + labelY, knobWidth, "Random", ID::oscPhaseRandom));

  // Misc.
  const auto miscTop = phaseTop;
  const auto miscLeft = phaseLeft + 2.0f * knobX + 2.0f * margin;
  tabview->addWidget(
    tabMain, addGroupLabel(miscLeft, miscTop, 3.0f * knobX - 2.0f * margin, "Misc."));

  const auto miscTop0 = miscTop + labelY;
  const auto miscLeft0 = miscLeft;
  tabview->addWidget(
    tabMain,
    addKnob(
      miscLeft0 + margin, miscTop0, knobWidth, colorBlue, "Smooth", ID::smoothness));

  const auto nVoiceLeft = miscLeft0 + knobX + margin;
  std::vector<UTF8String> nVoiceOptions
    = {"16 Voices", "32 Voices", "48 Voices",  "64 Voices",
       "80 Voices", "96 Voices", "112 Voices", "128 Voices"};
  tabview->addWidget(tabMain, addLabel(nVoiceLeft, miscTop0, 8 * margin, "Poly"));
  tabview->addWidget(
    tabMain,
    addOptionMenu(nVoiceLeft + 8 * margin, miscTop0, knobX, ID::nVoice, nVoiceOptions));

  tabview->addWidget(
    tabMain, addLabel(nVoiceLeft, miscTop0 + labelY, 8 * margin, "Seed"));
  tabview->addWidget(
    tabMain,
    addTextKnob(
      nVoiceLeft + 8 * margin, miscTop0 + labelY, knobX, colorBlue, ID::seed,
      Scales::seed));

  // Delay.
  const auto delayTop = unisonTop + 2.0f * labelY + knobY;
  const auto delayLeft = gainLeft;
  tabview->addWidget(tabMain, addGroupLabel(delayLeft, delayTop, 5.0f * knobX, "Delay"));
  const auto delayKnobTop = delayTop + labelY;

  tabview->addWidget(
    tabMain, addKnob(delayLeft, delayKnobTop, knobWidth, colorBlue, "Mix", ID::delayMix));
  tabview->addWidget(
    tabMain,
    addKnob(
      delayLeft + 1.0f * knobX, delayKnobTop, knobWidth, colorBlue, "Feedback",
      ID::delayFeedback));
  tabview->addWidget(
    tabMain,
    addKnob(
      delayLeft + 2.0f * knobX, delayKnobTop, knobWidth, colorBlue, "Attack",
      ID::delayAttack));

  const auto delayDetuneLeft = delayLeft + 3.0f * knobX;
  tabview->addWidget(
    tabMain, addLabel(delayDetuneLeft, delayKnobTop, tuningLabelWidth, "Semi"));
  tabview->addWidget(
    tabMain,
    addTextKnob(
      delayDetuneLeft + knobX, delayKnobTop, knobWidth, colorBlue, ID::delayDetuneSemi,
      Scales::delayDetuneSemi, false, 0, -120));

  tabview->addWidget(
    tabMain, addLabel(delayDetuneLeft, delayKnobTop + labelY, tuningLabelWidth, "Milli"));
  auto knobDelayMilli = addTextKnob(
    delayDetuneLeft + knobX, delayKnobTop + labelY, knobWidth, colorBlue,
    ID::delayDetuneMilli, Scales::oscMilli, false, 0, -1000);
  knobDelayMilli->sensitivity = 0.001f;
  knobDelayMilli->lowSensitivity = 0.00025f;
  tabview->addWidget(tabMain, knobDelayMilli);

  // LFO.
  const auto lfoTop = delayTop;
  const auto lfoLeft = delayLeft + 5.0f * knobX + 2.0f * margin;
  tabview->addWidget(tabMain, addGroupLabel(lfoLeft, lfoTop, 9.0f * knobX, "Delay LFO"));
  const auto lfoKnobTop = lfoTop + labelY;

  const auto lfoLeft1 = lfoLeft;
  const auto lfoTempoTop = lfoKnobTop;
  auto knobLfoTempoNumerator = addTextKnob(
    lfoLeft1, lfoTempoTop, knobWidth, colorBlue, ID::lfoTempoNumerator,
    Scales::lfoTempoNumerator, false, 0, 1);
  knobLfoTempoNumerator->sensitivity = 0.001;
  knobLfoTempoNumerator->lowSensitivity = 0.00025;
  tabview->addWidget(tabMain, knobLfoTempoNumerator);

  auto knobLfoTempoDenominator = addTextKnob(
    lfoLeft1, lfoTempoTop + labelHeight + 1.0f, knobWidth, colorBlue,
    ID::lfoTempoDenominator, Scales::lfoTempoDenominator, false, 0, 1);
  knobLfoTempoDenominator->sensitivity = 0.001;
  knobLfoTempoNumerator->lowSensitivity = 0.00025;
  tabview->addWidget(tabMain, knobLfoTempoDenominator);

  tabview->addWidget(
    tabMain,
    addLabel(lfoLeft1, lfoTempoTop + labelHeight + labelY - margin, knobWidth, "Tempo"));

  tabview->addWidget(
    tabMain,
    addKnob(
      lfoLeft1 + 1.0f * knobX, lfoKnobTop, knobWidth, colorBlue, "Multiply",
      ID::lfoFrequencyMultiplier));
  tabview->addWidget(
    tabMain,
    addKnob(
      lfoLeft1 + 2.0f * knobX, lfoKnobTop, knobWidth, colorBlue, "Amount",
      ID::lfoDelayAmount));
  tabview->addWidget(
    tabMain,
    addKnob(
      lfoLeft1 + 3.0f * knobX, lfoKnobTop, knobWidth, colorBlue, "Lowpass",
      ID::lfoLowpass));

  const auto lfoLeft2 = lfoLeft + 5.0f * knobX;

  std::vector<UTF8String> lfoWavetableTypeOptions{"Step", "Linear", "Cubic"};
  tabview->addWidget(
    tabMain,
    addLabel(lfoLeft2 + 0.375f * knobX, lfoKnobTop, knobWidth * 1.5f, "Interpolation"));
  tabview->addWidget(
    tabMain,
    addOptionMenu(
      lfoLeft2 + 0.375f * knobX, lfoKnobTop + labelY, knobWidth * 1.5f,
      ID::lfoWavetableType, lfoWavetableTypeOptions));

  tabview->addWidget(
    tabMain,
    addKickButton(
      lfoLeft2 + 2.0f * knobX, lfoKnobTop + 2.0f * margin, 2.0f * knobX, "Refresh LFO",
      ID::refreshLFO));

  // LFO wavetable.
  const auto lfoWaveTop = lfoKnobTop + knobY + 0.5f * labelY;
  const auto lfoWaveLeft = tabInsideLeft0;
  const auto lfoBarboxHeight = barboxHeight + 3.5f * labelY + 3.0f * margin;
  tabview->addWidget(
    tabMain, addGroupVerticalLabel(lfoWaveLeft, lfoWaveTop, lfoBarboxHeight, "LFO Wave"));
  auto barboxLfoWavetable = addBarBox(
    lfoWaveLeft + labelY, lfoWaveTop, barboxWidth + 2.0f * knobX + 4.0f * margin,
    lfoBarboxHeight, ID::lfoWavetable0, nLFOWavetable, Scales::lfoWavetable, "LFO Wave");
  barboxLfoWavetable->sliderZero = 0.5f;
  tabview->addWidget(tabMain, barboxLfoWavetable);

  // Wavetable pitch.
  const auto tablePitchTop = tabInsideTop0;
  const auto tablePitchLeft0 = tabInsideLeft0;
  const auto tablePitchLeft1 = tablePitchLeft0 + knobX;
  tabview->addWidget(
    tabPadSynth, addGroupLabel(tablePitchLeft0, tablePitchTop, 2.0f * knobX, "Pitch"));

  tabview->addWidget(
    tabPadSynth, addLabel(tablePitchLeft0, tablePitchTop + labelY, knobX, "Base Freq."));
  tabview->addWidget(
    tabPadSynth,
    addTextKnob(
      tablePitchLeft1, tablePitchTop + labelY, knobX, colorBlue, ID::tableBaseFrequency,
      Scales::tableBaseFrequency, false, 2));

  tabview->addWidget(
    tabPadSynth,
    addLabel(tablePitchLeft0, tablePitchTop + 2.0f * labelY, knobX, "Multiply"));
  tabview->addWidget(
    tabPadSynth,
    addTextKnob(
      tablePitchLeft1, tablePitchTop + 2.0f * labelY, knobX, colorBlue,
      ID::overtonePitchMultiply, Scales::overtonePitchMultiply, false, 4));

  tabview->addWidget(
    tabPadSynth,
    addLabel(tablePitchLeft0, tablePitchTop + 3.0f * labelY, knobX, "Modulo"));
  tabview->addWidget(
    tabPadSynth,
    addTextKnob(
      tablePitchLeft1, tablePitchTop + 3.0f * labelY, knobX, colorBlue,
      ID::overtonePitchModulo, Scales::overtonePitchModulo, false, 4));

  const auto tableSpectrumTop = tablePitchTop + 4.0f * labelY;
  const auto tableSpectrumLeft0 = tablePitchLeft0;
  const auto tableSpectrumLeft1 = tablePitchLeft1;
  tabview->addWidget(
    tabPadSynth,
    addGroupLabel(tableSpectrumLeft0, tableSpectrumTop, 2.0f * knobX, "Spectrum"));

  tabview->addWidget(
    tabPadSynth,
    addLabel(tableSpectrumLeft0, tableSpectrumTop + labelY, knobX, "Expand"));
  tabview->addWidget(
    tabPadSynth,
    addTextKnob(
      tableSpectrumLeft1, tableSpectrumTop + labelY, knobX, colorBlue, ID::spectrumExpand,
      Scales::spectrumExpand, false, 4));

  tabview->addWidget(
    tabPadSynth,
    addLabel(tableSpectrumLeft0, tableSpectrumTop + 2.0f * labelY, knobX, "Rotate"));
  auto knobSpectrumRotate = addTextKnob(
    tableSpectrumLeft1, tableSpectrumTop + 2.0f * labelY, knobX, colorBlue,
    ID::spectrumRotate, Scales::defaultScale, false, 6);
  knobSpectrumRotate->lowSensitivity = 1.0f / spectrumSize;
  tabview->addWidget(tabPadSynth, knobSpectrumRotate);

  tabview->addWidget(
    tabPadSynth,
    addLabel(tableSpectrumLeft0, tableSpectrumTop + 3.0 * labelY, knobX, "Comb"));
  auto knobProfileComb = addTextKnob(
    tableSpectrumLeft1, tableSpectrumTop + 3.0 * labelY, knobX, colorBlue,
    ID::profileComb, Scales::profileComb);
  knobProfileComb->sensitivity = 0.002;
  knobProfileComb->lowSensitivity = 0.0002;
  tabview->addWidget(tabPadSynth, knobProfileComb);

  tabview->addWidget(
    tabPadSynth,
    addLabel(tableSpectrumLeft0, tableSpectrumTop + 4.0 * labelY, knobX, "Shape"));
  tabview->addWidget(
    tabPadSynth,
    addTextKnob(
      tableSpectrumLeft1, tableSpectrumTop + 4.0 * labelY, knobX, colorBlue,
      ID::profileShape, Scales::profileShape, false, 4, 0));

  const auto tablePhaseTop = tableSpectrumTop + 5.0f * labelY;
  const auto tablePhaseLeft0 = tablePitchLeft0;
  tabview->addWidget(
    tabPadSynth, addGroupLabel(tablePhaseLeft0, tablePhaseTop, 2.0f * knobX, "Phase"));
  tabview->addWidget(
    tabPadSynth,
    addCheckbox(
      tablePhaseLeft0, tablePhaseTop + labelY, checkboxWidth, "UniformPhase",
      ID::uniformPhaseProfile));

  // Wavetable random.
  const auto tableRandomTop = tablePhaseTop + 2.0f * labelY;
  const auto tableRandomLeft0 = tablePitchLeft0;
  const auto tableRandomLeft1 = tablePitchLeft1;
  tabview->addWidget(
    tabPadSynth, addGroupLabel(tableRandomLeft0, tableRandomTop, 2.0f * knobX, "Random"));

  tabview->addWidget(
    tabPadSynth, addLabel(tableRandomLeft0, tableRandomTop + labelY, knobX, "Seed"));
  tabview->addWidget(
    tabPadSynth,
    addTextKnob(
      tableRandomLeft1, tableRandomTop + labelY, knobX, colorBlue, ID::padSynthSeed,
      Scales::seed));

  // Wavetable buffer size.
  const auto tableBufferTop = tableRandomTop + 2.0f * labelY;
  const auto tableBufferLeft0 = tablePitchLeft0;
  const auto tableBufferLeft1 = tablePitchLeft1;
  tabview->addWidget(
    tabPadSynth,
    addGroupLabel(tableBufferLeft0, tableBufferTop, 2.0f * knobX, "BufferSize"));

  std::vector<UTF8String> bufferSizeItems{
    "2^10", "2^11", "2^12",           "2^13",           "2^14",           "2^15",
    "2^16", "2^17", "2^18 (128 MiB)", "2^19 (256 MiB)", "2^20 (512 MiB)", "2^21 (1 GiB)"};
  tabview->addWidget(
    tabPadSynth,
    addOptionMenu(
      tableRandomLeft0, tableBufferTop + labelY, 2.0f * knobX, ID::tableBufferSize,
      bufferSizeItems));

  // Wavetable modifier.
  const auto tableModifierTop = tableBufferTop + 2.0f * labelY;
  const auto tableModifierLeft0 = tablePitchLeft0;
  const auto tableModifierLeft1 = tablePitchLeft1;
  tabview->addWidget(
    tabPadSynth,
    addGroupLabel(tableModifierLeft0, tableModifierTop, 2.0f * knobX, "Modifier"));

  const auto tableModifierTop0 = tableModifierTop + labelY;
  tabview->addWidget(
    tabPadSynth,
    addKnob(
      tableModifierLeft0, tableModifierTop0, knobWidth, colorBlue, "Gain^",
      ID::overtoneGainPower));
  tabview->addWidget(
    tabPadSynth,
    addKnob(
      tableModifierLeft1, tableModifierTop0, knobWidth, colorBlue, "Width*",
      ID::overtoneWidthMultiply));

  // Refresh button.
  const auto refreshTop = tabTop0 + tabHeight - 2.0f * labelY;
  const auto refreshLeft = tabInsideLeft0;
  tabview->addWidget(
    tabPadSynth,
    addKickButton(
      refreshLeft, refreshTop, 2.0f * knobX, "Refresh Table", ID::refreshTable));

  // Overtone Gain.
  const auto otGainTop = tabInsideTop0;
  const auto otGainLeft = tabInsideLeft0 + 2.0f * knobX + 4.0f * margin;
  tabview->addWidget(
    tabPadSynth, addGroupVerticalLabel(otGainLeft, otGainTop, barboxHeight, "Gain"));

  const auto otGainLeft0 = otGainLeft + labelY;
  tabview->addWidget(
    tabPadSynth,
    addBarBox(
      otGainLeft0, otGainTop, barboxWidth, barboxHeight, ID::overtoneGain0, nOvertone,
      Scales::overtoneGain, "Gain"));

  // Overtone Width.
  const auto otWidthTop = otGainTop + barboxY + margin;
  const auto otWidthLeft = otGainLeft;
  tabview->addWidget(
    tabPadSynth, addGroupVerticalLabel(otWidthLeft, otWidthTop, barboxHeight, "Width"));

  const auto otWidthLeft0 = otWidthLeft + labelY;
  tabview->addWidget(
    tabPadSynth,
    addBarBox(
      otWidthLeft0, otWidthTop, barboxWidth, barboxHeight, ID::overtoneWidth0, nOvertone,
      Scales::overtoneWidth, "Width"));

  // Overtone Pitch.
  const auto otPitchTop = otWidthTop + barboxY + margin;
  const auto otPitchLeft = otGainLeft;
  tabview->addWidget(
    tabPadSynth, addGroupVerticalLabel(otPitchLeft, otPitchTop, barboxHeight, "Pitch"));

  const auto otPitchLeft0 = otPitchLeft + labelY;
  tabview->addWidget(
    tabPadSynth,
    addBarBox(
      otPitchLeft0, otPitchTop, barboxWidth, barboxHeight, ID::overtonePitch0, nOvertone,
      Scales::overtonePitch, "Pitch"));

  // Overtone Phase.
  const auto otPhaseTop = otPitchTop + barboxY + margin;
  const auto otPhaseLeft = otGainLeft;
  tabview->addWidget(
    tabPadSynth, addGroupVerticalLabel(otPhaseLeft, otPhaseTop, barboxHeight, "Phase"));

  const auto otPhaseLeft0 = otPhaseLeft + labelY;
  tabview->addWidget(
    tabPadSynth,
    addBarBox(
      otPhaseLeft0, otPhaseTop, barboxWidth, barboxHeight, ID::overtonePhase0, nOvertone,
      Scales::overtonePhase, "Phase"));

  auto textKnobControl = R"(- Knob -
Shift + Left Drag|Fine Tuning
Ctrl + Left Click|Reset to Default)";
  tabview->addWidget(
    tabInfo,
    addTextTableView(
      tabInsideLeft0, tabInsideTop0, 400.0f, 400.0f, textKnobControl, 150.0f, fontSize));

  auto textNumberControl = R"(- Number -
Shares same controls with knob, and:
Right Click|Flip Minimum and Maximum)";
  tabview->addWidget(
    tabInfo,
    addTextTableView(
      tabInsideLeft0, tabInsideTop0 + 80.0f, 400.0f, 400.0f, textNumberControl, 150.0f,
      fontSize));

  auto textOvertoneControl = R"(- Overtone & LFO Wave -
Ctrl + Left Click|Reset to Default
Right Drag|Draw Line
D|Reset to Default
Shift + D|Toggle Min/Mid/Max
E|Emphasize Low
Shift + E|Emphasize High
F|Low-pass Filter
Shift + F|High-pass Filter
I|Invert Value
Shift + I|Invert Value (Minimum to 0)
N|Normalize
Shift + N|Normalize (Minimum to 0)
P|Permute
R|Randomize
Shift + R|Sparse Randomize
S|Sort Decending Order
Shift + S|Sort Ascending Order
T|Subtle Randomize
, (Comma)|Rotate Back
. (Period)|Rotate Forward
1|Decrease
2-9|Decrease 2n-9n)";
  tabview->addWidget(
    tabInfo,
    addTextTableView(
      tabInsideLeft0, tabInsideTop0 + 160.0f, 400.0f, 400.0f, textOvertoneControl, 150.0f,
      fontSize));

  const auto tabInfoLeft1 = tabInsideLeft0 + tabWidth / 2.0f;

  auto textRefreshNotice = R"(Wavetables do not refresh automatically.
Press following button to apply changes.
- `Refresh LFO` at center-left in Main tab.
- `Refresh Table` at bottom-left in Wavetable tab.)";
  tabview->addWidget(
    tabInfo,
    addTextView(
      tabInfoLeft1, tabInsideTop0, 400.0f, 400.0f, textRefreshNotice, fontSize));

  auto textBufferSizeNotice = R"(Be careful changing BufferSize.
Max 1 GiB of memory will be allocated.
Small buffer size makes some configs to be quiet.)";
  tabview->addWidget(
    tabInfo,
    addTextView(
      tabInfoLeft1, tabInsideTop0 + 100.0f, 400.0f, 400.0f, textBufferSizeNotice,
      fontSize));

  const auto tabInfoBottom = tabInsideTop0 + tabHeight - labelY;
  std::stringstream ssPluginName;
  ssPluginName << "\nLightPadSynth " << VERSION_STR;
  auto pluginNameTextView = addTextView(
    tabInfoLeft1, tabInfoBottom - 160.0f, 400.0f, 400.0f, ssPluginName.str(), 36.0f);
  tabview->addWidget(tabInfo, pluginNameTextView);

  tabview->addWidget(
    tabInfo,
    addTextView(
      tabInfoLeft1, tabInfoBottom - 100.0f, 400.0f, 400.0f,
      "© 2020 Takamitsu Endo (ryukau@gmail.com)\n\nHave a nice day!", fontSize));

  tabview->refreshTab();

  return true;
}

} // namespace Vst
} // namespace Steinberg

// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#pragma once

#include "../../common/gui/plugeditor.hpp"
#include "parameter.hpp"

#include <algorithm>
#include <memory>
#include <unordered_map>

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

class Editor : public PlugEditor {
public:
  Editor(void *controller);

  void valueChanged(CControl *pControl) override;
  void updateUI(ParamID id, ParamValue normalized) override;

  DELEGATE_REFCOUNT(VSTGUIEditor);

private:
  struct XYPadAxis {
    size_t index = 0;
    SharedPointer<XYPad> xypad;
    SharedPointer<CControl> control;

    XYPadAxis(size_t index, XYPad *xypad, CControl *control)
      : xypad(xypad), control(control), index(std::min(index, size_t(1)))
    {
    }

    void sync(float normalized)
    {
      xypad->setValueAt(xypad->id[index], normalized);
      xypad->invalid();
      control->setValue(normalized);
    }
  };

  std::unordered_map<ParamID, std::shared_ptr<XYPadAxis>> xyControlMap;
  void syncUI(ParamID id, float normalized);

  bool prepareUI() override;

  template<Uhhyou::Style style = Uhhyou::Style::common>
  auto addSmallKnobVert(
    CCoord left,
    CCoord top,
    CCoord size,
    CCoord margin,
    CCoord textSize,
    std::string name,
    ParamID tag)
  {
    auto knob
      = new Knob<style>(CRect(left, top, left + size, top + size), this, tag, palette);
    knob->setArcWidth(2.0);
    knob->setValueNormalized(controller->getParamNormalized(tag));
    knob->setDefaultValue(param->getDefaultNormalized(tag));
    frame->addView(knob);
    addToControlMap(tag, knob);

    auto label = addLabel(
      left, top + size + margin, size, size, textSize, name, CHoriTxtAlign::kCenterText);
    return knob;
  }

  template<Uhhyou::Style style = Uhhyou::Style::common>
  auto addSmallKnob(
    CCoord left,
    CCoord top,
    CCoord width,
    CCoord height,
    CCoord margin,
    CCoord textSize,
    std::string name,
    ParamID tag)
  {
    auto knob = new Knob<style>(
      CRect(left, top, left + height, top + height), this, tag, palette);
    knob->setArcWidth(2.0);
    knob->setValueNormalized(controller->getParamNormalized(tag));
    knob->setDefaultValue(param->getDefaultNormalized(tag));
    frame->addView(knob);
    addToControlMap(tag, knob);

    auto offset = height + 2 * margin;
    auto label = addLabel(
      left + offset, top, width - offset, height, textSize, name,
      CHoriTxtAlign::kLeftText);
    return std::make_tuple(knob, label);
  }

  void addXYControls(
    CCoord left,
    CCoord top,
    CCoord width,
    CCoord smallKnobWidth,
    CCoord margin,
    CCoord textSize,
    ParamID idX,
    ParamID idY)
  {
    auto xypad = addXYPad(left, top, width, width, idX, idY);
    auto xKnob = addSmallKnobVert(
      left + width + 2 * margin, top + 2 * margin, smallKnobWidth, margin, textSize, "X",
      idX);
    auto yKnob = addSmallKnobVert(
      left + width + 2 * margin, top + 2 * margin + int(width / 2), smallKnobWidth,
      margin, textSize, "Y", idY);

    xyControlMap.emplace(idX, std::make_shared<XYPadAxis>(0, xypad, xKnob));
    xyControlMap.emplace(idY, std::make_shared<XYPadAxis>(1, xypad, yKnob));
  }
};

} // namespace Vst
} // namespace Steinberg

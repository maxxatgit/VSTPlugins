// SPDX-License-Identifier: GPL-3.0-only
// Copyright Takamitsu Endo (ryukau@gmail.com)

#include "../../../common/gui/splash.hpp"
#include "../version.hpp"

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

void CreditView::draw(CDrawContext *pContext)
{
  pContext->setDrawMode(CDrawMode(CDrawModeFlags::kAntiAliasing));
  CDrawContext::Transform t(
    *pContext, CGraphicsTransform().translate(getViewSize().getTopLeft()));

  const auto width = getWidth();
  const auto height = getHeight();
  const double borderWidth = 2.0;
  const double halfBorderWidth = borderWidth / 2.0;

  // Background.
  pContext->setLineWidth(borderWidth);
  pContext->setFillColor(pal.background());
  pContext->drawRect(CRect(0.0, 0.0, width, height), kDrawFilled);

  // Border.
  pContext->setFrameColor(isMouseEntered ? pal.highlightMain() : pal.border());
  pContext->drawRect(
    CRect(
      halfBorderWidth, halfBorderWidth, width - halfBorderWidth,
      height - halfBorderWidth),
    kDrawStroked);

  // Text.
  pContext->setFont(fontIdTitle);
  pContext->setFontColor(pal.foreground());
  pContext->drawString("AccumulativeRingMod " VERSION_STR, CPoint(20.0, 40.0));

  pContext->setFont(fontIdText);
  pContext->setFontColor(pal.foreground());
  pContext->drawString("© 2023 Takamitsu Endo (ryukau@gmail.com)", CPoint(20.0f, 60.0f));

  std::string leftText = R"(- Number & Knob -
Shift + Left Drag|Fine Adjustment
Ctrl + Left Click|Reset to Default
Middle Click|Flip Min/Mid/Max
Shift + Middle Click|Take Floor

- XY Pad -
Ctrl + Left Click|Reset value.
Middle Drag|Only Change X value.
Mouse Wheel|Fine Adjustment X value.
Shift + Middle Drag|Only Change Y value.
Shift + Mouse Wheel|Fine Adjustment Y value.
)";

  std::string rightText = R"(Some DAW copies main input signal to side
chain when side chain is not activated.

When stereo image becomes strange, make
sure to set Stereo Link to greater than 1,
and set Cross and Offset to 0.

CPU load rises when Phase Warp is not 0.

Have a nice day!)";

  constexpr float top0 = 100.0f;
  constexpr float lineHeight = 20.0f;
  constexpr float blockWidth = 120.0f;
  drawTextBlock(pContext, 20.0f, top0, lineHeight, blockWidth, leftText);
  drawTextBlock(
    pContext, int(width / 2) + 20.0f, top0, lineHeight, blockWidth, rightText);

  setDirty(false);
}

} // namespace Vst
} // namespace Steinberg

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
  pContext->drawString("FDN64Reverb " VERSION_STR, CPoint(20.0, 40.0));

  pContext->setFont(fontIdText);
  pContext->setFontColor(pal.foreground());
  pContext->drawString("© 2022 Takamitsu Endo (ryukau@gmail.com)", CPoint(20.0f, 60.0f));

  std::string leftText = R"(- BarBox -
Ctrl + Left Drag|Reset to Default
Middle Drag|Draw Line
Shift + D|Toggle Min/Mid/Max
I|Invert Value
P|Permute
R|Randomize
S|Sort Decending Order
T|Random Walk
Shift + T|Random Walk to 0
Z|Undo
Shift + Z|Redo
, (Comma)|Rotate Back
. (Period)|Rotate Forward
1-4|Decrease 1n-4n
5-9|Hold 2n-5n

And more! Refer to the manual for full list
of shortcuts.)";

  std::string rightText = R"(- Number -
Shift + Left Drag|Fine Adjustment
Ctrl + Left Click|Reset to Default
Middle Click|Flip Min/Mid/Max
Shift + Middle Click|Take Floor

Time Multi. and Feedback can be used to quickly
shorten or lengthen reverb time.

To change stereo image, press Matrix button. It
has no effect on Hadamard and Conference matrix.

To get rotation effect, set Skew greater than 0.

Increasing Interp. Rate adds more noise when
delay time is modulated by hand or LFO.

Have a nice day!)";

  const float top0 = 100.0f;
  const float mid = (750 - 2 * 20) / 2;
  const float lineHeight = 20.0f;
  const float blockWidth = 160.0f;
  drawTextBlock(pContext, 20.0f, top0, lineHeight, blockWidth, leftText);
  drawTextBlock(pContext, mid, top0, lineHeight, blockWidth, rightText);

  setDirty(false);
}

} // namespace Vst
} // namespace Steinberg

---
lang: en
...

# FDN64Reverb
![](img/fdn64reverb.png)

FDN64Reverb is a reverb equipped with a feedback delay network (FDN). However, the size of feedback matrix is 64.

- [Download FDN64Reverb 0.1.0 - VST® 3 (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/BasicLimiterAndFDN64Reverb/FDN64Reverb_0.1.0.zip) <img
  src="img/VST_Compatible_Logo_Steinberg_negative.svg"
  alt="VST compatible logo."
  width="60px"
  style="display: inline-block; vertical-align: middle;">
- [Download Presets (github.com)](https://github.com/ryukau/VSTPlugins/releases/download/BasicLimiterAndFDN64Reverb/FDN64ReverbPresets.zip)

FDN64Reverb requires CPU which supports AVX or later SIMD instructions.

The package includes following builds:

- Windows 64bit
- Linux 64bit

On 2022-05-15, macOS is not supported. Current code fails to build as macOS universal binary. I'm planning to support macOS in future. However due to my financial situation, I can't purchase M1 mac for now.

Linux build is built on Ubuntu 20.04. If you are using distribution other than Ubuntu 20.04, plugin will not likely run. In this case, please take a look at [build instruction](https://github.com/ryukau/VSTPlugins/blob/master/build_instruction.md).

## Contact
Feel free to contact me on [GitHub repository](https://github.com/ryukau/VSTPlugins/commits/master) or `ryukau@gmail.com.`

You can fund the development through [paypal.me/ryukau](paypal.me/ryukau).  Current goal is to purchase M1 mac for macOS and ARM port. 🤑💸💻

## Installation
### Plugin
Place `*.vst3` directory to:

- `/Program Files/Common Files/VST3/` for Windows.
- `$HOME/.vst3/` for Linux.
- `/Users/$USERNAME/Library/Audio/Plug-ins/VST3/` for macOS.

DAW may provides additional VST3 directory. For more information, please refer to the manual of the DAW.

### Presets
Extract preset zip, then place preset directory to the OS specific path:

- Windows : `/Users/$USERNAME/Documents/VST3 Presets/Uhhyou`
- Linux : `$HOME/.vst3/presets/Uhhyou`
- macOS : `/Users/$USERNAME/Library/Audio/Presets/Uhhyou`

Preset directory name must be the same as the plugin. Make `Uhhyou` directory if it does not exist.

### Windows Specific
If DAW doesn't recognize the plugin, try installing C++ redistributable (`vc_redist.x64.exe`). Installer can be found in the link below.

- [The latest supported Visual C++ downloads](https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads)

### Linux Specific
On Ubuntu 18.0.4, those packages are required.

```bash
sudo apt install libxcb-cursor0  libxkbcommon-x11-0
```

If DAW doesn't recognize the plugin, take a look at `Package Requirements` section of the link below and make sure all the VST3 related package is installed.

- [VSTGUI: Setup](https://steinbergmedia.github.io/vst3_doc/vstgui/html/page_setup.html)

REAPER on Linux may not recognize the plugin. A workaround is to delete a file `~/.config/REAPER/reaper-vstplugins64.ini` and restart REAPER.

### macOS Specific
**Important**: Current package do not contain mac OS build.

When trying to run plugin first time, following message may appear on macOS.

```
<PluginName>.vst3 is damaged and can't be opened. You should move it to
the Trash"
```

In this case, open terminal and try running following command to unzipped `.vst3` directory.

```sh
xattr -rc /path/to/PluginName.vst3
```

Plugin may be considered as unsigned/un-notarized application. In this case, try following the steps below.

1. Open terminal and run `sudo spctl --master-disable`.
2. Go to System Preferences → Security and Privacy → General → Allow apps downloaded from, then select "Anywhere".

Beware that steps above degrades security of your system. To revert the settings, follow the steps below.

1. Go to System Preferences → Security and Privacy → General → Allow apps downloaded from, then select option to "App Store and identified developers".
2. Open terminal and run `sudo spctl --master-enable`.

#### Reference
- [How to Fix App “is damaged and can’t be opened. You should move it to the Trash” Error on Mac](https://osxdaily.com/2019/02/13/fix-app-damaged-cant-be-opened-trash-error-mac/)
- [Allowing unsigned/un-notarized applications/plugins in Mac OS | Venn Audio](https://www.vennaudio.com/allowing-unsigned-un-notarized-applications-plugins-in-mac-os/)
- [Safely open apps on your Mac - Apple Support](https://support.apple.com/en-us/HT202491)

## Color Configuration
At first time, create color config file to:

- `/Users/USERNAME/AppData/Roaming/UhhyouPlugins/style/style.json` on Windows.
- `$XDG_CONFIG_HOME/UhhyouPlugins/style/style.json` on Linux.
  - If `$XDG_CONFIG_HOME` is empty, make `$HOME/.config/UhhyouPlugins/style/style.json`.
- `/Users/$USERNAME/Library/Preferences/UhhyouPlugins/style/style.json` on macOS.

Below is a example of `style.json`.

```json
{
  "fontPath": "",
  "foreground": "#000000",
  "foregroundButtonOn": "#000000",
  "foregroundInactive": "#8a8a8a",
  "background": "#ffffff",
  "boxBackground": "#ffffff",
  "border": "#000000",
  "borderCheckbox": "#000000",
  "borderLabel": "#000000",
  "unfocused": "#dddddd",
  "highlightMain": "#0ba4f1",
  "highlightAccent": "#13c136",
  "highlightButton": "#fcc04f",
  "highlightWarning": "#fc8080",
  "overlay": "#00000088",
  "overlayHighlight": "#00ff0033"
}
```

Hex color codes are used.

- 6 digit color is RGB.
- 8 digit color is RGBA.

First letter `#` is conventional. Plugins ignore the first letter of color code, thus `?102938`, `\n11335577` are valid.

Do not use characters outside of `0-9a-f` for color value.

- `fontPath`: Absolute path to *.ttf font file. Not implemented in VST 3 version.
- `foreground`: Text color.
- `foregroundButtonOn`: Text color of active toggle button. Recommend to use the same value of `foreground` or `boxBackground`.
- `foregroundInactive`: Text color of inactive components. Currently, only used for TabView.
- `background`: Background color.
- `boxBackground`: Background color of inside of box shaped components (Barbox, Button, Checkbox, OptionMenu, TextKnob, VSlider).
- `border`: Border color of box shaped components.
- `borderCheckbox`: Border color of CheckBox.
- `borderLabel`: Line color of parameter section label.
- `unfocused`: Color to fill unfocused components. Currently, only used for knobs.
- `highlightMain`: Color to indicate focus is on a component. Highlight colors are also used for value of slider components (BarBox and VSlider).
- `highlightAccent`: Same as `highlightMain`. Used for cosmetics.
- `highlightButton`: Color to indicate focus is on a button.
- `highlightWarning`: Same as `highlightMain`, but only used for parameters which requires extra caution.
- `overlay`: Overlay color. Used to overlay texts and indicators.
- `overlayHighlight`: Overlay color to highlight current focus.

## Controls
<kbd>Right Click</kbd> on the controls pops up a context menu provided by DAW.

Knob and slider can do:

- <kbd>Ctrl</kbd> + <kbd>Left Click</kbd>: Reset value.
- <kbd>Shift</kbd> + <kbd>Left Drag</kbd>: Fine adjustment.

There is an additional control for number sliders used for `Octave`, `Seed` etc.

- <kbd>Middle Click</kbd> : Flip minimum and maximum.

Control with many blue vertical bars (BarBox) have some keyboard shortcuts. `LFO Wave` on Main tab and `Gain`, `Width`, `Pitch`, `Phase` on Wavetable tab are using BarBox. Shortcuts are enabled after left clicking BarBox and mouse cursor is on the inside of BarBox. Cheat sheet is available on Infomation tab.

| Input                                                      | Control                            |
| ---------------------------------------------------------- | ---------------------------------- |
| <kbd>Left Drag</kbd>                                       | Change Value                       |
| <kbd>Shift</kbd> + <kbd>Left Drag</kbd>                    | Change Value (Snapped)             |
| <kbd>Ctrl</kbd> + <kbd>Left Drag</kbd>                     | Reset to Default                   |
| <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>Left Drag</kbd>  | Change Value (Skip Between Frames) |
| <kbd>Right Drag</kbd>                                      | Draw Line                          |
| <kbd>Shift</kbd> + <kbd>Right Drag</kbd>                   | Edit One Bar                       |
| <kbd>Ctrl</kbd> + <kbd>Right Drag</kbd>                    | Reset to Default                   |
| <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>Right Drag</kbd> | Toggle Lock                        |
| <kbd>a</kbd>                                               | Alternate Sign                     |
| <kbd>d</kbd>                                               | Reset Everything to Default        |
| <kbd>D</kbd>                                               | Toggle Min/Mid/Max                 |
| <kbd>e</kbd>                                               | Emphasize Low                      |
| <kbd>E</kbd>                                               | Emphasize High                     |
| <kbd>f</kbd>                                               | Low-pass Filter                    |
| <kbd>F</kbd>                                               | High-pass Filter                   |
| <kbd>i</kbd>                                               | Invert Value (Preserve minimum)    |
| <kbd>I</kbd>                                               | Invert Value (Minimum to 0)        |
| <kbd>l</kbd>                                               | Toggle Lock under Mouse Cursor     |
| <kbd>L</kbd>                                               | Toggle Lock for All                |
| <kbd>n</kbd>                                               | Normalize (Preserve minimum)       |
| <kbd>N</kbd>                                               | Normalize (Minimum to 0)           |
| <kbd>p</kbd>                                               | Permute                            |
| <kbd>r</kbd>                                               | Randomize                          |
| <kbd>R</kbd>                                               | Sparse Randomize                   |
| <kbd>s</kbd>                                               | Sort Descending Order              |
| <kbd>S</kbd>                                               | Sort Ascending Order               |
| <kbd>t</kbd>                                               | Subtle Randomize (Random walk)     |
| <kbd>T</kbd>                                               | Subtle Randomize (Converge to 0)   |
| <kbd>z</kbd>                                               | Undo                               |
| <kbd>Z</kbd>                                               | Redo                               |
| <kbd>,</kbd> (Comma)                                       | Rotate Back                        |
| <kbd>.</kbd> (Period)                                      | Rotate Forward                     |
| <kbd>1</kbd>                                               | Decrease                           |
| <kbd>2</kbd>-<kbd>9</kbd>                                  | Decrease 2n-9n                     |

Snapping is not available for all BarBox. If you'd like to have snapping for certain BarBox, feel free to open issue to [GitHub repository](https://github.com/ryukau/VSTPlugins).

Edit One Bar with <kbd>Shift</kbd> + <kbd>Right Drag</kbd> holds a bar under the cursor when mouse right button is pressed. Then only changes that one bar while holding down mouse right button.

Toggle Lock with <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>Right Drag</kbd> behaves as line edit. When right mouse button (RMB) is pressed, it holds the opposite state of the bar below mouse cursor, then use the state for the rest of bars. For example, if RMB is pressed on a locked bar, dragging unlocks bars while holding down RMB.

## Caution
Setting `Feedback` to 1 and `Highpass Cutoff` to 0 might adds massive direct current depending on input signal.

## Block Diagram
If the image is small, use <kbd>Ctrl</kbd> + <kbd>Mouse Wheel</kbd> or "View Image" on right click menu to scale.

Diagram only shows overview. It's not exact implementation.

![](img/fdn64reverb.svg)

## Parameters
Delay Time \[s\]

:   Delay times in seconds. Below is the equation to calculate the total of delay time.

    ```
    delayTime = (Time Multi.) * (Delay Time) + (Time LFO Amount) * random().
    ```

    Note that maximum delay time do not exceed 1 second. This means that if both `Delay Time` and `Time LFO Amount` are set to 1, then LFO stop working.

Time LFO Amount \[s\]

:   Amounts of LFO modulation to delay times.

    When `Interp. Rate` exceeds 1.0, LFO modulations start adding noticeable noise. To get smooth output, recommend to set `Interp. Rate` under 0.25.

Lowpass Cutoff \[Hz\]

:   Cutoff frequencies of lowpass filters that are applied to delay outputs.

Highpass Cutoff \[Hz\]

:   Cutoff frequencies of highpass filters that are applied to delay outputs.

    Note that setting `Feedback` to 1 and `Highpass Cutoff` to 0 might adds massive direct current depending on input signal.

### Delay
Time Multi.

:   A multiplier to `Delay Time`. Useful to change all the delay time at once.

Feedback

:   Feedback amount of FDN. In other words, `Feedback` is a scalar that multiplies feedback matrix.

    When `Feedback` is close to 1.0, it lengthen the reverb time. When set to exactly 1.0, the reverb stop to decay. In this case, `Gate` can be used to shut down the reverb.

Interp. Rate

:   Rate limiting value of delay time interpolation. For example, when `Interp. Rate` is set to 0.1, changing delay time from 0 to 1 samples requires 10 samples of interpolation time.

Gate \[dB\]

:   Threshold of gate.

    When input amplitude is less than the value of `Gate` for a while, the gate modulates `Stereo Cross` to make output amplitude to 0. This gate is added to use along with rotation, therefore it closes fast.

Matrix

:   Type of feedback matrix. Note that changing this parameter may cause pop nosie.

    | Abbreviation | Full Name          | Additional Infomation               | Quality |
    | ------------ | ------------------ | ----------------------------------- | ------- |
    | Ortho.       | Orthogonal         |                                     | Good    |
    | S. Ortho.    | Special orthogonal |                                     | Good    |
    | Circ. Ortho. | Circulant          |                                     | OK      |
    | Circ. 4      | Circulant          | 4 non-zero elements per row         | SFX     |
    | Circ. 8      | Circulant          | 8 non-zero elements per row         | SFX     |
    | Circ. 16     | Circulant          | 16 non-zero elements per row        | SFX     |
    | Circ. 32     | Circulant          | 32 non-zero elements per row        | SFX     |
    | Upper Tri. + | Upper triangular   | Range of randomization is `[0, 1]`  | Bad     |
    | Upper Tri. - | Upper triangular   | Range of randomization is `[-1, 0]` | Bad     |
    | Lower Tri. + | Lower triangular   | Range of randomization is `[0, 1]`  | Bad     |
    | Lower Tri. - | Lower triangular   | Range of randomization is `[-1, 0]` | Bad     |
    | Schroeder +  | Schroeder reverb   | Range of randomization is `[0, 1]`  | Bad     |
    | Schroeder -  | Schroeder reverb   | Range of randomization is `[-1, 0]` | Bad     |
    | Absorbent +  | Absorbent allpass  | Range of randomization is `[0, 1]`  | OK      |
    | Absorbent -  | Absorbent allpass  | Range of randomization is `[-1, 0]` | OK      |
    | Hadamard     | Hadamard           | No randomization                    | Good    |
    | Conference   | Conference         | No randomization                    | Good    |

    Quality very rough indication. Bad means it frequently adds metallic tones to output. Good means almost no metallic tone. OK is in between good and bad. SFX means they sound strange as a reverb. Bad matrices can be used to add more effects with rotation.

Seed

:   Seed value for randomization of feedback matrix. Note that changing this parameter may cause pop nosie.

Change Matrix

:   A button to randomize feedback matrix. Pressing `Change Matrix` also changes the value of `Seed`. Note that changing this parameter may cause pop nosie.

### Mix
Dry \[dB\]

:   A gain applied to input signal which bypasses FDN.

Wet \[dB\]

:   FDN output gain.

Stereo Cross

:   Stereo crossing feedback amount between left and right FDNs. Setting `Stereo Cross` to 1.0 stops input to prevent blow up.

### Rotation
Speed \[Hz\]

:   Rotation speed of FDN input gains.

Offset

:   Initial phase of the waveform to rotate input gains.

    `Offset` can be used to change the sound when `Speed` is 0, but `Skew` is greater than 0. Feedback matrix affects the character of output.

Skew

:   Changes the waveform used to rotate input gains. Note that when `Skew` is set to 0, `Speed` and `Offset` stop working.

    Below is a plot of relation between `Skew` and rotation waveform.

    <figure>
    <img src="img/skewplot.svg" alt="Image of plot of relation between `Skew` parameter and rotation waveform. The equation of waveform is `exp(skew * sin(ω * t))`." style="padding-bottom: 12px;"/>
    </figure>

## Change Log
- 0.1.0
  - Initial release.

### Old Versions
There aren't any old versions.

## License
FDN64Reverb is licensed under GPLv3. Complete licenses are linked below.

- [https://github.com/ryukau/VSTPlugins/tree/master/License](https://github.com/ryukau/VSTPlugins/tree/master/License)

If the link above doesn't work, please send email to `ryukau@gmail.com`.

### About VST
VST is a trademark of Steinberg Media Technologies GmbH, registered in Europe and other countries.
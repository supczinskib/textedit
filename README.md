# TextEdit for Luckfox Pico Mini

A lightweight FLTK-based text editor adapted for the Luckfox Pico Mini / RV1103 embedded Linux environment.

This project is based on the `editor` application from the Tiny Core Linux `fltk_projects` repository:

- Upstream project: <https://github.com/tinycorelinux/fltk_projects>
- Original component: `editor`

The goal of this fork is to keep the editor small and simple while making it usable on a very small framebuffer/X11 desktop environment, especially a 480x320 LCD on Luckfox Pico Mini.

## Why this fork exists

The original Tiny Core Linux editor is a compact FLTK text editor. On the Luckfox Pico Mini target system, several practical adjustments were needed:

- the display is very small: 480x320 pixels;
- part of the screen is occupied by the system/task bar;
- X11 runs through a lightweight framebuffer setup;
- the mouse cursor is software-rendered, so unnecessary window redraws are very visible;
- the user interface must remain readable and configurable through X resource-style settings;
- mixed UTF-8 text should work better with the available fonts on the target system.

This fork keeps the original spirit of the application, but adapts it to these constraints.

## Main changes

### User interface localization

The editor contains built-in translations for the application menus, dialogs, file chooser labels and common buttons.

Supported language tables include:

- Arabic
- Simplified Chinese
- Dutch
- English
- French
- German
- Italian
- Polish
- Portuguese
- Russian
- Spanish
- Turkish

The language is selected from the environment, for example `LANGUAGE`.

### Configurable colors through app-defaults

The editor reads color settings from:

```text
/usr/share/X11/app-defaults/TextEdit
```

The file uses X resource-style syntax, for example:

```text
TextEdit.dialogBackground:       #2f2e2b
TextEdit.fieldBackground:        #3c3b37
TextEdit.foreground:             #ffffff
TextEdit.selection:              #6AA835
TextEdit.textBackground:         #202020
TextEdit.textForeground:         #ffffff
TextEdit.textSelection:          #6AA835
TextEdit.menuBackground:         #3c3b37
TextEdit.menuForeground:         #ffffff
TextEdit.menuSelection:          #6AA835
TextEdit.checkboxBackground:     #3c3b37
TextEdit.checkboxSelection:      #6AA835
TextEdit.browserSelection:       #6AA835
TextEdit.inputSelection:         #6AA835
TextEdit.scrollbarBackground:    #3c3b37
TextEdit.scrollbarArrow:         #3c3b37
TextEdit.cjkFont:                WenQuanYi Micro Hei
```

### Small-screen file chooser adjustments

The FLTK file chooser is restricted to fit the target screen:

- maximum width: 480 px;
- usable work height: 296 px;
- additional space reserved for the window manager title bar;
- final file chooser height is reduced so the title bar remains visible.

The file chooser also starts in the user home directory when no file is currently selected, instead of starting in the less useful `File Systems` view.

### Reduced unnecessary redraws

The file chooser is resized, centered and color-adjusted once after it appears. It is not repeatedly redrawn on every mouse event.

This is important on framebuffer/X11 systems where the mouse pointer is software-rendered. Repeated redraws under the pointer can make the cursor visibly flicker.

### Centered dialogs

Application dialogs are centered relative to their parent window where practical. This includes custom dialogs and selected FLTK-generated helper windows.

### CJK font fallback for mixed UTF-8 files

The main editor font remains suitable for Latin, Cyrillic, Arabic and other common scripts. Chinese/CJK characters can be rendered with a separate configurable font through FLTK text display styles.

Default CJK font setting:

```text
TextEdit.cjkFont:                WenQuanYi Micro Hei
```

This is intended for systems where DejaVu Sans is used as the normal UI/text font, while Chinese glyphs are provided by `wqy-microhei.ttf`.

### Context menu localization

The text editor context menu is localized instead of using untranslated default labels.

## Target environment

This fork is primarily intended for:

- Luckfox Pico Mini / RV1103;
- Buildroot-based embedded Linux;
- FLTK/X11;
- small LCD panels around 480x320;
- lightweight window managers such as IceWM;
- systems where disk space, RAM and redraw cost matter.

It may also build and run on other Linux systems with FLTK, but the modifications are tuned for the embedded target above.

## Building

Use the same general build approach as the Tiny Core Linux `fltk_projects/editor` application, adjusted for your cross-toolchain and FLTK installation.

A typical native build on a system with FLTK development files would use `fltk-config`, for example:

```sh
c++ -Os -o textedit textedit.cxx $(fltk-config --cxxflags --ldflags)
strip textedit
```

For cross-compilation, use your target compiler and sysroot, for example:

```sh
${CXX:-arm-rockchip830-linux-uclibcgnueabihf-g++} \
  -Os \
  -o textedit \
  textedit.cxx \
  $(fltk-config --cxxflags --ldflags)

${STRIP:-arm-rockchip830-linux-uclibcgnueabihf-strip} textedit
```

The exact command depends on how FLTK is installed in the target SDK/sysroot.

## Installing app-defaults

Install the resource file as:

```sh
install -d /usr/share/X11/app-defaults
install -m 0644 TextEdit /usr/share/X11/app-defaults/TextEdit
```

## Upstream credit

This project is a modified version of the Tiny Core Linux FLTK editor from `tinycorelinux/fltk_projects`.

All credit for the original editor belongs to the Tiny Core Linux / FLTK project authors and contributors. This fork contains practical modifications for the Luckfox Pico Mini embedded environment.

## Notes

This fork is not intended to turn the editor into a large desktop editor. The goal is the opposite: keep it small, understandable and suitable for a constrained embedded GUI system.

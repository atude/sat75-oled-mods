# Satisfaction 75 OLED mods

[Join the discord for information on all features and downloadable .bins!](https://discord.gg/MddWhNPpkB)

### Instructions

1. Open QMK toolbox
2. Put your Satisfaction75 in bootloader mode by pressing *FN+LCTRL*
3. Load the *satisfaction75_atude_vXX.bin*
4. Flash

### Includes

- Bongo cat - outlined and filled, interactions with winkey, ctrl, caps lock, wpm, etc.
- Keyboard matrix
- Luna
- Kirby
- Persistent settings with ability to set OLED timeout

### Requirements

- QMK environment setup

### Building

1. In QMK repo, copy all files from here into keyboards/cannonkeys/satisfaction75
2. Build using `qmk compile -kb cannonkeys/satisfaction75/rev1 -km via` or flash with `qmk flash -kb cannonkeys/satisfaction75/rev1 -km via`

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).


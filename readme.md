## [MOVED: MODS ARE NOW DONE IN ATYU](https://github.com/atude/atyu-app)

![](https://raw.githubusercontent.com/atude/sat75-oled-mods/main/banner.png)

# Satisfaction 75 OLED mods

[Join the discord for information on all features and downloadable .bins!](https://discord.gg/MddWhNPpkB)

### Instructions

1. Download the *satisfaction75_atude_vXX.bin* in this repo (or in the discord server)
1. Open QMK toolbox
1. Put your Satisfaction75 in bootloader mode by pressing *FN+LCTRL*
1. Load the bin file and flash

### Includes

- Bongo cat - outlined and filled, interactions with winkey, ctrl, caps lock, wpm, etc.
- Keyboard matrix
- Luna
- Kirby
- Pusheen
- Persistent settings with ability to set OLED timeout
- More info available in discord server

### Control mappings

These are controls outside of the default Satisfaction75 mappings:

- **F23/F24** -> Change between outlined and filled bongo. Or change between luna, kirby and pusheen
- **F22** -> Change time display to time, date, or info text
- **Clock Set** -> Settings

Detailed controls and mappings found in the Discord server

## Development

### Requirements

- QMK environment setup

### Building

1. In QMK repo, copy all files from here into keyboards/cannonkeys/satisfaction75
2. Build using `qmk compile -kb cannonkeys/satisfaction75/rev1 -km via` or flash with `qmk flash -kb cannonkeys/satisfaction75/rev1 -km via`

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).


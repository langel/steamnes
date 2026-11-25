### The Plan
1. make an NES emulator with C/SDL2 that runs GunTneR
2. wrap it with a C++ GUI supporting Steam achievements
3. profit

Targeting mapper 0 support only for now.  [*sic*]
Copy a rom file to `/steamnes/game_rom.nes`

## TO DO
- extract SDL2 functional wrappers to sdl2w repo
- extract FCL layer to fcl repo
- abstract apu registers to voices and oscillators
- finish apu
- drag-n-drop rom loader
- support for more mappers (anrom/cnrom/unrom)
- ppu glitch options
- audio channel visualizers
- chr text notifs/debug messages (sdl2w?)
- alt+enter fullscreen (sdl2w window)
- spacebar mute
- audio volume slider
- settle on pixel aspect ratio
- settle on window ratios (w/ debug and not)
- escape triggers exit/options

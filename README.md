# Quake2 for AmigaOS 4

This experimental port is based on Hyperion's Quake2 sourcecode with following limitations:

1) Does NOT support any MODs currently
2) No software renderer
3) AHI-only
4) No joystick support currently
5) No CD audio support currently
6) Tested only with AmigaOS 4.1 but should work on 4.0 too

## Installation

If you haven't installed Quake 2 already, then let's suppose you want to install the game on "Games"
partition:

1) Create Games:Quake2/ directory
2) Create Games:Quake2/baseq2/ directory
3) Copy Quake2 executable to Games:Quake2/ dir
4) Copy original PAK files (the game data) to Games:Quake2/baseq2/ directory
5) Start Games:Quake2/Quake2

Note: the binary has a "stack cookie" set to 1000000 bytes.

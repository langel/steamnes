placeholder text for ideas around this project

## Audio

- ability to set base clock
	- pokey has 15kHz and 64kHz (and 1.79mHz?)
	- tia has 30kHz
	- vic20 has NTSC: 14318181/14 Hz or PAL: 4433618/4 Hz
	- sms psg @ 3.579545
	- genesis ym @ 53.267 kHz (NTSC), 52.781 kHz (PAL)
	- nes @ 
- set clock division bit depth
	- pokey has 8bit
	- tia has 5bit
	- vic20 has 7bit (but additional divisors per channel)
	- sms psg 12bit
	- genesis ym @
	- nes @ 11bit
- 8 audio channels of various abilities
	- various oscillator modes
		- 2 oscillator mode
			- 4 selectable waveforms (sqr/saw/tri/sin)
			- 2nd osc fine or coarse tune relative to 1st
		- 2op fm mode
		- 8bit pcm playback
		- noise mode (lfsr)
	- adsr envelope (exp/log/lin/rlog)
	- output routing
- a small selection of effects
	- 2 delays (2 mono or combine for stereo)
	- 2 multimode filter (low/band/high/notch)
	- (could maybe have 8 effects total to match mixer inputs?)
- routing options / mixer
	- 8 inputs with vol/pan
	- effects send
	- master volume


## Video

- single program window
	- dimensions can be whatever
	- fullscreen letterboxes mismatched dimensions
- single output canvas
	- dimensions have limitations?
	- automatically scaled / stretched to fit window
	- 256 indexed colors palette 
		- 24bit RGB values
		- color 255 is transparent?
	- 256 on screen sprites
		- max size: 256x256
		- x,y pos off screen
		- x,y origin (pos doesn't have to be top-left of sprite)
		- collision detection
	- background/foreground tiling layers?
	- fps setting (affects game/audio loops? probably!)
	- retro shader effects (CRT curve / scanlines)


## Inputs
- read controller without window focus


## non-square pixel thought stream

4/3 or 12/9  vs 16/9

320x200 has a 1.6 aspect ratio 

However, when the resolution was used all displays were 4:3, and most users would stretch the 200 vertical lines to fill up the screen.  Instead of perfectly square pixels, you would get pixels 1.2 times as vertical compared with the horizontal width on a monitor where the vertical width had been stretched to the edges of the monitor.

(width1 / height1) / (width2 / height2) = pixel ratio?

320x200 = 8/5 but its for a 4/3 screen has a 1.2 pixel aspect ratio

420x200 = 21/10 but its for a 16/9 screen has a 1.18125(?) pixel aspect ratio

426x200 = 213/100 but its for a 16/9 screen has a 1.198125(?) pixel aspect ratio

427x200 = 427/200 but its for a 16/9 screen has a 1.2009375(?) pixel aspect ratio

432x200 = 54/25 but its for a 16/9 screen has a 1.215(?) pixel ratio

480x200 = 12/5 but its for a 16/9 screen has a 1.35(?) pixel aspect ratio

720x200 = 18/5 but its for a 16/9 screen has a 2.025(?) pixel aspect ratio

https://en.wikipedia.org/wiki/Pixel_aspect_ratio

Pixel Aspect Ratio = Display Aspect Ratio / Storage Aspect Ratio

320x200 = 8:5 / 4:3 = 1.2

320x200 = 8:5 / 16:9 = 0.9

420x200 = 21:10 / 16:9 = 1.18125

Quad the Size (4x) : 1680x800

hardware limitations: resizing the pixel density costs one visible frame of the old pixel density in the new pixel density; if new one is bigger missing data is filled with noise

### how many Video Modes?
each mode has 84,000 pixels with a different pixel aspect ratio

| id | width | height | dar | par | potential tile size | tile fill |
| :---: | ---: | ---: | :---: | ---: | :---: | :---: |
| 0 | 525 | 160 |105/32 | 1.84570 | 25x10 | 21x16 |
| 1 | 480 | 175 | 96/36 | 1.54285 | 10x5  | 48x36 |
| 2 | 420 | 200 | 21/10 | 1.18125 | 10x8  | 42x50 |
| 3 | 400 | 210 | 40/21 | 1.07143 | 10x10 | 40x21 | 
| 4 | 375 | 224 | gross | 0.94168 | 15x16 | 25x14 |
| 5 | 350 | 240 | 35/24 | 0.82031 | 10x16 | 35x15 | 
| 6 | 336 | 250 | gross | 0.74404 | 8x10  | 42x25 |
| 7 | 300 | 280 | 15/14 | 0.60267 | 10x10 | 30x28 |

#### moar chr rom fonts (aquarius)
https://www.gridsagegames.com/rexpaint/resources.html#Fonts

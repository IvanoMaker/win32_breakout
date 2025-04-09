# win32_breakout

This is a basic rendition of the classic Atari arcade game "breakout". This version is written from scratch in C++ using vanilla features in the Win32 API. This is intended to be used as a demo for the API as well as a boiler plate for making basic 2D games on windows platforms.

## Functionality
* Draws rectangles, text, and bitmaps.
* Handles user keyboard input.
* Runs at a fixed framerate of 60 fps.
* Implements double-buffered animation for a clean, flicker free image.
* pausing/starting
* game end state handling

## Gameplay
To play the game, download the build or clone the repository and compile the main.cpp file. Then open the executable.

### Controls
- `<-` : Move the paddle left
- `->` : Move the paddle right
- `ESC`: pause the game
- to unpause the game, press either the left or right arrows

## Clone the repository:
   ```bash
   git clone https://github.com/IvanoMaker/win32_breakout.git

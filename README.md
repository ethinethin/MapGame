MapGame
-

A procedurally-generated map game written in C (with SDL2). The majority of development has been streamed at: https://twitch.tv/ethinethin

Installation
-

Compiling on MacOS, Linux, or *BSD is straightforward. First download and install the SDL2 libraries from here: https://libsdl.org/download-2.0.php

Then clone, compile and run with:

    git clone https://github.com/ethinethin/MapGame
    cd MapGame
    make
    ./mapgame

Instructions for compiling on Windows are yet to be determined.

Starting a Game
-

On the title screen, use the mouse or `[keyboard]` to access:

- `[N]` start a new game
- `[C]` load game or continue game in-progress
- `[O]` options
- `[Q]` quit

Display Options
-

Several resolutions are provided to fit most display configurations. The game can either be run in a window or fullscreen. Fullscreen —> Default is the recommended fullscreen option, which uses the native desktop resolution.

If your preferred, non-desktop resolution is not on the list, you can try changing the resolution in `save/opts.mg` by opening it in a text editor.

In-game Controls
-

- Movement:
    - `WASD keys` move up/left/down/right
    - `Arrow keys` also move
- Title screen:
    - `Escape` brings up the title screen
    - Pressing `Escape` again or `[C]` will continue the game
    - From title screen, you can click with `Left mouse button` or press `[S]` to save game
- Picking up items on ground:
    - `Left click` with mouse
    - `[R]` retrieves item with keyboard
- Dropping items:
    - Drag and drop with `Left mouse button`
    - `[T]` throws item with keyboard
- Swap items in inventory
    - Drag and drop with `Left mouse button`
    - `[F]` swaps items
    - `[Q]` and `[E]` moves the inventory cursor
    - `[I]` toggles inventory window
- Harvesting items
    - `Right click` harvests items from adjacent world tiles
    - `[Z]` also harvests items
- Crafting items
    - `[C]` opens crafting interface
    - `Left mouse button` interacts with crafting interface
- World Map
    - `[M]` displays world map
- Special items
    - `Right clicking` an item stack enters placement mode, which allows the dropping of single items. In placement mode, `left click` places the item and `right click` exits placement mode.
    - If a single wall, floor, roof, holder, or door is placed on the ground, it is placed as a functional item.
    - `Left click` interacts with doors and chests.
    - Placed items can be picked up with `right click`. Chests can only be picked up if they are empty.

Feedback and/or Support
-

Reach out to me on any platform for feedback and/or support.

- Github: https://github.com/ethinethin
- Twitch: https://twitch.tv/ethinethin
- Youtube: https://youtube.com/user/ethinethin
- Twitter: https://twitter.com/ethinethingames
- Email: ethinethin@gmail.com

Thanks
-

Special thanks to Twitch users who have helped me with this project!

License
-

BSD 3-Clause License

Copyright (c) 2021, Ethinethin <ethinethin@gmail.com>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

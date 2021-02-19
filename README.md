MapGame
-

A procedurally-generated map game written in C and SDL2. I am writing this as part of my "Twitch Game Jam" (rules below), with development being streamed at: https://twitch.tv/ethinethin

Installation
-

Want to try out the game? It should compile no problem on Linux or macOS with the instructions below. It will also probably compile on Windows (and the *BSDs if you can install SDL2), but I don't know how (...yet!).

Download and install SDL2 from here: https://libsdl.org/download-2.0.php

Then clone, compile and run with:

    git clone https://github.com/ethinethin/MapGame
    cd MapGame
    make
    ./mapgame

Piece of cake! In-game keys are:

- ESC: quit game
- Arrow keys and WASD: move around
- M: show world map
- R: pickup item
- T: throw item
- F: swap item in inventory
- I: toggle inventory on/off
- Q and E: move cursor in quickbar/inventory
- Z: harvest from tiles

Mouse controls:

- Click on nearby item: pickup item
- Click in inventory: move cursor in quickbar/inventory
- Drag and drop in inventory/quickbar: swap item in inventory
- Drag and drop on map: throw item
- Right click to harvest

That's all there is so far!

License
-

Like what you want see and want to use it? Fine! It's licensed under a BSD 3-clause License, so you can do almost anything you want, just give a quick read to the LICENSE file.

Game Jam Rules
-

I am programming this as part of a challenge, with the following conditions:

- Only doing major coding on-stream
- Debugging and administrative stuff (like writing this README) allowed off stream
- Only streaming in limited capacity (2 to 4 hours per stream, 2 to 4 days per week)

You can check out my stream at: https://twitch.tv/ethinethin


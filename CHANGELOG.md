Changelog
-

2021-05-23
-

- Crafting now recognizes item stackability
- Fixed a hiding bug in check_recipes
- Added random house generator
- Added the ability to place a house onto the map

2021-05-21
-

- Changed font loading from surface (CPU rendering) to texture (GPU rendering)
- Worldmap texture is now reloaded when display mode is switched
- Added loading bar for loading and reloading map texture, but removed it because of performance issues I still need to figure out
- Changed font rendering to allow alpha/transparency changes
- Item quantity appears when dragging and dropping items
- When picking up or harvesting items, the cursor changes to the last placed stack in inventory

2021-05-20
-

- Removed dynamic scaling from 2021-05-16
- Output all displays to a texture, which is then stretched to the screen/window size
- Scaled mouseclicks to the screen/window size
- Added multiple display modes:
	- Fullscreen (Desktop resolution)
	- Fullscreen (User-specified resolution)
	- Window (User-specified resolution)

2021-05-16
-

- Fixed Makefile to account for new source files automatically
- Added options screen
- Added window resizing
- Options saving/loading
- Some sprite scaling based on window size (incomplete)

2021-05-09
-

- Unknown recipes show up as a “?”
- Player can learn recipes from items in inventory
- Player recipe knowledge is saved and loaded
- NPCs are saved and loaded
- NPCs can be deleted from the NPC table

2021-05-07
-

- Added the ability to have more than four recipes
- Added scrolling through recipe book
- Added NPCs
- Created random NPCs and added to linked list
- Adjusted harvest probabilities and recipes

2021-04-25
-

- Game creates save files and directories if they don’t exist
- Player can save in any slot
- Game asks for confirmation for overwriting saves
- Title screen mouse controls (title, save/load)

2021-04-23
-

- Made the title screen background “more efficient”
- Added functions to check existence of save files
- Added functions to load multiple saves


2021-04-18
-

- Added saving/loading functions for:
	- Holders
- Added title screen

2021-04-16
-

- Added saving/loading functions for:
	- Player
	- World map
	- Harvesting

2021-04-11
-

- Added extra crafting buttons
- Greyed out CRAFT button when insufficient material
- Added crafting logic - players can craft items now
- Made a modular recipe book structure

2021-04-09
-

- Cursor is drawn around selected recipe
- Player can click on recipes to select
- Player can enter quantity to craft
- Made craft button placeholder sprites
- Crafting maximum can be determined allowing for max button usage and “Insufficient Materials” notification

2021-04-04
-

- Started on crafting system

2021-04-02
-

- Made new sprites for stone walls, floors, roofs, and others

2021-03-28
-

- Made custom font

2021-03-21
-

- Finished grassland items
- Added grassland items to harvest table

2021-03-19
-

- Worked on grassland harvestable item set
- Fixed update_seen for revealing worldmap

2021-03-18
-

- Finished desert tile sprites

2021-03-14
-

- Designed more desert tile sprites

2021-03-12
-

- Added scanlines & scanline toggling
- Designed two desert tile sets
- Added variable number of animation frames according to tile

2021-03-11
-

- Changed tile rendering size from 32x32 to 64x64
- Optimized code for dynamic tile size rendering
- Changed blinking indicator size
- Added puddles and swamp sprites in grassland

2021-03-07
-

- Added new sprites to the game
- Added animation of sprites to game
- Optimized all input loops to accommodate animation
- Added “X” in world map and allowed click closing
- Dropped item stacks are always PASSABLE
- Changed world map indicator to always show current view
- Added “X” in inventory

2021-03-06
-

- Designed several new sprites for grassland

2021-03-05
-

- Removed all software rendering of tiles
- Player can swap items in holder mode
- Placement mode can be used in holder mode

2021-03-04
-

- General bug fixes (random start location)
- Opening and closing chest
- Dragging and dropping chest (swap not implemented yet)

2021-02-28
-

- Raised stack limit to 999
- Added doors
- Added holders linked list (no holder functionality yet)

2021-02-19
-

- Added "place item" mode (drop singles from a stack)
- Dropped items (including roof and ground tiles) are always displayed smaller on map
- Placed walls, roofs and grounds are always displayed full size

2021-02-18
-

- Changed dropped items to 24x24 sprites
- Introduced wall and roof item types
- Walls displayed at 32x32
- Implemented roof tile layer and all logic with placing and harvesting
- Implemented complex "indoor/outdoor" logic to reveal inside of buildings
- Roof tiles displayed at lower alpha to show inside

2021-02-14
-

- Floor tile layer added to map
- Harvested tiles can regenerate
- Toggle inventory while harvesting

2021-02-12
-

- Right-click harvesting
- Sustained harvesting
- Harvesting progress bar
- Harvesting gives random quantities based on tile
- Tiles now have limited harvesting

2021-02-11
-

- Diagonal mouse dropping
- "Dev mode" reveal entire map
- Random "safe" starting location improved
- Modified player indicator on world map
- Basic Harvesting (keyboard controls)

2021-02-07
-

- Fixed drop preview ghosting
- Implemented walk while dropping
- Cursor remains in inventory after closing
- Biome blending
- World map rendered as texture
- Blinking player indicator on world map
- Random starting location

2021-02-06
-

- Pickup with mouse
- Hardware accelerated sprites
- Fixed handle\_pickup and handle\_throw functions
- Moved update\_seen
- Item drag and drop to drop item on map

2021-02-05
-

- Cursor movement in inventory with mouse click
- Swap items with mouse click
- Move cursor on quickbar with number keys
- Swap with mouse drag and drop

2021-02-04
-

- Inventory swapping with keyboard
- Cursor movement in inventory with keyboard

2021-01-30
-

- Improved player movement
- Tested inventory system (stacking, etc.)
- Added quickbar
- Added cursor for quickbar with keyboard and mouse controls
- Implemented items blocking movement

2021-01-29
-

- Added inventory system

2021-01-24
-

- Added font
- Added random world generation via map generation functions
- Re-ordered loading process
- Added loading bar

2021-01-23
-

- Added map loading in smaller pieces called "farts"
- Added desert and forest biomes
- Removed mini-map and added a callable map function
- Settled on final map size of 1024x512
- Individual "farts" loaded as 128x128, not necessarily final size

2021-01-22
-

- Implemented multiple biomes

2021-01-20
-

- Added flags for more strict warnings to Makefile
- Cleaned up all excessive warnings

2021-01-19
-

- Fixed a bug that revealed an extra row of the map when at the edges
- Cleaned up all x/y rows/cols confusion in maps.c

2021-01-18
-

- Fixed a bug in the procedural generation, which is now working as intended

2021-01-17
-

- Created CHANGELOG.md
- Implemented world map
- Implemented fog of war on world map
- Implemented sprite and color logic in tile structure

2021-01-16
-

- Incorporated sprites for tiles
- Introduced character sprite/movement
- Implemented map scrolling
- Fixed bug in window/map position
- Made GitHub repo

2021-01-15
-

- Restructured map generation code into current organization
- Converted console/text-based tiles to graphical output (SDL2)
- Fixed bug in map generation probability

2021-01-05
-

- Made map generation probability engine
- Output map to console


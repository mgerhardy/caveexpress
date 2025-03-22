Version 1.0 (2013-11-09)
* Multiplayermode
* Fixed a bug with the savegames

Version 1.1 (2013-11-13)
* New death screens
* Bubbles in the water
* Fixed a mouse cursor bug

Version 1.2 (2013-11-18)
* Fixed map editor saving
* Fixed problems with sound playbacks
* Updated some map starting positions
* Fixed empty 'You won' window
* Improved campaign progress handling
* Help messages in the tutorial campaign
* New rock campaign

Version 1.3 (2013-11-25)
* Fixed map starting position in rock campaign
* OUYA support
* Fixed ui focus handling for invisible objects
* Improved joystick support
* Fixed crash at startup on some android devices

Version 1.4 (2013-12-22)
* Fixed the width of the Extra window on some devices
* Fixed invalid alpha handling for some textures
* Improved joystick support
* Improved keyboard and joystick UI management
* A new campaign with some new maps
* Fixed shutdown issues on Android
* Improved the map editor
* Implemented alternating rising and falling water
* Fixed dropping of packages under some circumstances
* Fixed sound problems on Android on shutdown or muting
* Fixed intro map bug on Windows
* Snow particles for the ice maps
* Some new ice theme map tiles

Version 1.5 (2014-01-18)
* Fixed sliding but dazed mammut bug
* Improved multiplayer
* Updated water physics
* Fixed linux builds
* The analog stick can now be used to scroll through the menus
* Added player crash animation
* Continue button is now in the campaign menu
* Fixed android red screen issue

Version 1.6 (2014-05-02)
* Open sourced the game code and the assets
* Added ouya joystick button help
* Allow to switch between big and small layout (If your screen is big enough to allow the big layout)
* Allow to switch between the game modes (Beware, switching to hard mode will wipe your campaign progress)
* Reworked the map and game options menus
* Fixed a bug with the angry dinosaurs that leads to sliding in rare situations
* Two new campaigns with new game modes
  * The first campaign is also about getting the packages indirectly to their packagetarget
  * The second (flappy) campaign is not about birds - but almost the same (Cryptic? Try it!)
* Translation support (english and german currently)
* Readded android quit button
* Changed ads handling in the free android version
* Introduction windows for the tutorial campaign
* Player does no longer drop the package when you tap onto him. Use the second finger to handle drops

Feedback for the control changes are more than welcome.
We are working hard on improving the game entry for the next version.

Version 1.7 (2014-05-21)
* Fixed crashes when the player dies

Version 1.8 (2014-08-12)
* Google NativeClient port
* Reduced file sizes
* Add zooming support

Version 1.9 (2014-08-12)
* Fixed a crash for some android versions

Version 2.0 (2014-09-19)
* Limit the frames per second (reduced battery drain on mobile)
* Added tracker support

Version 2.1 (2014-12-08)
* Disable the screensaver while the game is running
* Select the last played map in the map selection
* Added manual scrolling support
* Print messages when you can't collect the box
* Fixed physic errors

Version 2.2 (2014-12-23)
* Added wasd key bindings
* Added google play achievements

Version 2.3 (2015-01-24)
* Multiple player start positions for multiplayer

Version 2.4 (2016-02-18)
* Reduced file sizes
* Improved input handling
* Improved ui
* Water refractions with GL3 renderer
* Fixed key bindings if some unhandled modifiers (like the numpad) are active
* Fixed mouse speed
* Reduced ad frequency
* Rate me popup
* Improved game controller support
* Ported to steamlink device
* Updated tutorial maps to be a little bit easier
* Changed focus order of some of the buttons
* Reduced NPC flying speed
* Allow to modify the music and sound volume
* Transfering NPCs is now implemented
* New campaign
* The egg makes you invulnerable for some time
* Flying NPC drops egg on death
* Banana powerup - you can carry more than one package with it

Version 2.5 (2021-04-08 ?)
* Updated box2d
* Updated lua
* UTF8 support
* Fixed missing window resize events
* Added CI builds in github actions tab

Version 2.6 (2025-not yet released)
* Content
  * 5 New campaigns with new maps (33 total):
    * jungle (7 maps), desert (5), letters (9), villages (6), races (6)
  * 2 New sceneries: jungle and desert
  * New palm and desert trees
* Gameplay changes
  * Diving, now possible to fall underwater - few later maps use it
  * Possible to fly up with 3 packages, or even with 4 after powerup (banana)
  * Faster flying horizontally (on PC)
  * Faster consuming of packages in target, added sound
  * Faster NPCs (taxi gameplay) - less strict landing, faster walk start, take or drop, announce during walk
  * Friendly NPC don't get dropped when hit from above, they will only fall after a hard hit
  * More stamina restored by fruits
* UI and HUD
  * Stamina/health bar colors, yellow and flashing red when low
  * Reworked in-game HUD, now also showing:
    * time as text, package and NPC transfer counts (currently done and total to do on map)
  * Reworked in-game and editor Help screens
  * Map and campaign browsers now bigger, tiles 6 x 4, bigger font and map title shown
  * Renamed all maps and campaigns
    * Maps are named uniquely now (for multiplayer) and are starting with campaign name
* Changed water color to skyblue
* Particles
  * Water splashes
  * Rain in jungle
  * Smaller snow and more wobbly
  * Wind sand, flying leaves
* Editor
  * Fixed garbage tiles on big maps after scroll
  * Fixed scroll by arrow keys and scrollbars visible area
  * Added zoom to fit key F
  * Shift tile with alt+shift
* Updated tools, new: textureatlas

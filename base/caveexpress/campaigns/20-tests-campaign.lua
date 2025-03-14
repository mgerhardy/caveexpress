return  -- comment this line, to show campaign in game

-- create a new campaign
local c = Campaign.new("tests")
-- set an ingame icon for this campaign
--c:setSetting("icon", "tutorial-campaign")
-- set the on screen message when this campaign is activated
c:setSetting("text", "Tests")

-- now add all maps
c:addMaps("test-crash-fish-nothing-collected")
c:addMaps("test-crash-fish-package")
c:addMaps("test-crash-flying-package")
c:addMaps("test-crash-hitpoints")
c:addMaps("test-crash-sidescroll")
c:addMaps("test-crash-walking-package")
c:addMaps("test-crash-walking-stone")
c:addMaps("test-crash-water")
c:addMaps("test")
c:addMaps("test-platform-big")
c:addMaps("test-platform")
c:addMaps("test-win-package")
c:unlock()

-- create a new campaign
local c = Campaign.new("tutorial")
-- set an ingame icon for this campaign
--c:setSetting("icon", "tutorial-campaign")
-- set the on screen message when this campaign is activated
c:setSetting("text", "CavePacker")
-- now add all maps
c:addMaps("tutorial*")
c:addMaps("0*")
c:unlock()
c:unlockMap("tutorial0001")
c:unlockMap("tutorial0002")
c:unlockMap("tutorial0003")

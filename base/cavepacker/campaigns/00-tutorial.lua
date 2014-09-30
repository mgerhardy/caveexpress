-- create a new campaign
local c = Campaign.new("tutorial")
-- set an ingame icon for this campaign
c:setSetting("icon", "tutorial-campaign")
-- set the on screen message when this campaign is activated
c:setSetting("text", "Tutorial")
-- now add all maps
c:addMaps("tutorial*")
c:unlock()

-- create a new campaign
local c = Campaign.new("xsokoban")
-- set an ingame icon for this campaign
c:setSetting("icon", "xsokoban-campaign")
-- set the on screen message when this campaign is activated
c:setSetting("text", "XSokoban")
-- now add all maps
c:addMaps("xsokoban*")
c:unlock()

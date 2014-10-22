-- create a new campaign
local c = Campaign.new("ksokoban")
-- set an ingame icon for this campaign
c:setSetting("icon", "ksokoban-campaign")
-- set the on screen message when this campaign is activated
c:setSetting("text", "KSokoban")
-- now add all maps
c:addMaps("ksokoban*")
c:unlock()

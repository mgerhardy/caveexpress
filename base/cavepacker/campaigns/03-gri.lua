-- create a new campaign
local c = Campaign.new("gri")
-- set an ingame icon for this campaign
c:setSetting("icon", "gri-campaign")
-- set the on screen message when this campaign is activated
c:setSetting("text", "gri")
-- now add all maps
c:addMaps("gri*")
c:unlock()

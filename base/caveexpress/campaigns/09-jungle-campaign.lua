-- create a new campaign
local c = Campaign.new("jungle")
-- set an ingame icon for this campaign
c:setSetting("icon", "icon-campaign-rock")
-- set the on screen message when this campaign is activated
c:setSetting("text", "Jungle")
-- now add all maps
c:addMaps("jungle-01")

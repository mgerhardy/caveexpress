-- create a new campaign
local c = Campaign.new("races")
-- set an ingame icon for this campaign
c:setSetting("icon", "icon-campaign-ice")
-- set the on screen message when this campaign is activated
c:setSetting("text", "Races")
-- now add all maps
c:addMaps("races-01")

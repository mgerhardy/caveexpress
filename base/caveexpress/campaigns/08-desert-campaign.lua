-- create a new campaign
local c = Campaign.new("desert")
-- set an ingame icon for this campaign
c:setSetting("icon", "icon-campaign-ice")
-- set the on screen message when this campaign is activated
c:setSetting("text", "Desert")
-- now add all maps
c:addMaps("desert-01")
c:addMaps("desert-02")

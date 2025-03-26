-- create a new campaign
local c = Campaign.new("desert")
-- set an ingame icon for this campaign
c:setSetting("icon", "icon-campaign-ice")
-- set the on screen message when this campaign is activated
c:setSetting("text", "Desert 2")
-- now add all maps
c:addMaps("desert-01")
c:addMaps("desert-02")
c:addMaps("desert-03")
c:addMaps("desert-04")
c:addMaps("desert-05")
c:addMaps("desert-06")
c:unlock()

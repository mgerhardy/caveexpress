-- create a new campaign
local c = Campaign.new("races")
-- set an ingame icon for this campaign
c:setSetting("icon", "icon-campaign-ice")
-- set the on screen message when this campaign is activated
c:setSetting("text", "Races")
-- now add all maps
c:addMaps("races-01")
c:addMaps("races-02")
c:addMaps("races-03")
c:addMaps("races-04")
c:addMaps("races-05")
c:addMaps("races-06")
c:addMaps("races-07")
c:unlock()

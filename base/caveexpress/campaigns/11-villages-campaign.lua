-- create a new campaign
local c = Campaign.new("villages")
-- set an ingame icon for this campaign
c:setSetting("icon", "icon-campaign-rock")
-- set the on screen message when this campaign is activated
c:setSetting("text", "Villages")
-- now add all maps
c:addMaps("villages-01")
c:addMaps("villages-02")
c:addMaps("villages-03")
c:addMaps("villages-04")
c:addMaps("villages-05")
c:addMaps("villages-06")
c:addMaps("villages-07")
c:addMaps("villages-08")
c:addMaps("villages-09")
c:addMaps("villages-10")
c:unlock()

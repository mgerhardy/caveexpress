-- create a new campaign
local c = Campaign.new("jungle")
-- set an ingame icon for this campaign
c:setSetting("icon", "icon-campaign-rock")
-- set the on screen message when this campaign is activated
c:setSetting("text", "Jungle 2")
-- now add all maps
c:addMaps("jungle-01")
c:addMaps("jungle-02")
c:addMaps("jungle-03")
c:addMaps("jungle-04")
c:addMaps("jungle-05")
c:addMaps("jungle-06")
c:addMaps("jungle-07")
c:addMaps("jungle-08")
c:addMaps("jungle-09")
c:addMaps("jungle-10")
c:addMaps("jungle-11")
c:addMaps("jungle-12")
c:addMaps("jungle-13")
c:unlock()

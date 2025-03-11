-- create a new campaign
local c = Campaign.new("letters")
-- set an ingame icon for this campaign
c:setSetting("icon", "icon-campaign-rock")
-- set the on screen message when this campaign is activated
c:setSetting("text", "Letters")
-- now add all maps
c:addMaps("letter-01")
c:addMaps("letter-02")
c:addMaps("letter-03")
c:addMaps("letter-04")
c:addMaps("letter-05")
c:addMaps("letter-06")

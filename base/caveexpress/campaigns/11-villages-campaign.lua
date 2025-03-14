-- create a new campaign
local c = Campaign.new("villages")
-- set an ingame icon for this campaign
c:setSetting("icon", "icon-campaign-rock")
-- set the on screen message when this campaign is activated
c:setSetting("text", "Villages")
-- now add all maps
c:addMaps("villages-01")
c:unlock()

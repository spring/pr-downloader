function gadget:GetInfo()
	return {
		name    = "Test-Gadget",
		desc    = "Runs tests specified in config-file",
		author  = "Matthias Ableitner/abma",
		date    = "Sep. 2010",
		license = "GNU GPL, v2 or later",
		layer   = 0,
		enabled = true
	}
end

-- TODO hideinterface
-- load widgets specified by command-line
-- autoexit
-- remove start counter / set to 1 (?)
-- remove exit screen

function tellme(symbol, name)
	if (type(symbol) == "table") then
		for n,v in pairs(symbol) do
			if (type(v) == "table") then
				tellme(n ..".".. name)
			else
				Spring.Echo ( name .. n)
			end
		end
	else
		Spring.Echo(name)
	end
end
function gadget:Initialize()
	Spring.Echo( "--------------------------------------")
--	tellme(_G.System, "")
	Spring.Echo()
	Spring.Echo( "--------------------------------------")
end


function gadget:GetHourTimer()
	Spring.Echo("test")
end

function gadget:UnitCreated(unitID, unitDefID, unitTeam)
	Spring.Echo( "Hello Unit " .. unitID)

end

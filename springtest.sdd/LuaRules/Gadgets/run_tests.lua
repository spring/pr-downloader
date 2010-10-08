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

if (gadgetHandler:IsSyncedCode()) then

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
	Spring.SendCommands("hideinterface")
end

function gadget:GameFrame(n)
	if n==0 then
		Spring.Echo("test")
--		Spring.SendCommands("hideinterface")
		Spring.SendCommands("cheat 1")
		Spring.SendCommands("addplayer")
	end
	if n==1 then
		Spring.CreateUnit("corcom" , 2700 , 1100 , 10, 0, 0 )
		Spring.CreateUnit("armbanth" , 0 , 0 , 0, 0, 1 )
	end
	if n==2 then
		Spring.SendCommands("aikill 0")
	end
	if n==10 then
		Spring.SendCommands("aicontrol 0 RAI 0.601")
		Spring.SendCommands("setmaxspeed 100")
		Spring.SendCommands("setminspeed 100")
	end
	if n==100000 then
		Spring.SendCommands("screenshot")
		Spring.SendCommands("pause")
	end
end
function gadget:EndGame()
	Spring.SendCommands("quit")
	Spring.SendCommands("quitforce")
end


function gadget:GetHourTimer()
	Spring.Echo("test")
end

function gadget:UnitCreated(unitID, unitDefID, unitTeam)
	Spring.Echo( "Hello Unit " .. unitID)

end




end


/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#include <lslunitsync/c_api.h>
#include <lslunitsync/image.h>
#include <lslunitsync/unitsync.h>
#include <lslunitsync/springbundle.h>
#include <boost/format.hpp>
#include <cmath>
#include <iostream>


extern void lsllogerror(char const*, ...)
{
}
extern void lsllogdebug(char const*, ...)
{
}
extern void lsllogwarning(char const*, ...)
{
}

void dummySync()
{
	LSL::SpringBundle systembundle;
	LSL::SpringBundle::LocateSystemInstalledSpring(systembundle);

	LSL::Unitsync usync;
	{
		/*const bool usync_loaded =*/usync.ReloadUnitSyncLib();
	}
	//	std::cout << boost::format( "found %d maps and %d games\n") % usync.GetNumMaps() % usync.GetNumMods() ;
	/*	LSL::UnitsyncImage mini = usync.GetMinimap( "Alaska" );
	LSL::UnitsyncImage metal = usync.GetMetalmap( "Alaska" );
	LSL::UnitsyncImage height = usync.GetHeightmap( "Alaska" );
	mini.Save( "/tmp/alaska_mini.png" );
	metal.Save( "/tmp/alaska_metal.png" );
	height.Save( "/tmp/alaska_height.png" );
	LSL::UnitsyncImage heightL( "/tmp/alaska_height.png" );
	heightL.Save( "/tmp/alaska_heightL.png" );
*/
}

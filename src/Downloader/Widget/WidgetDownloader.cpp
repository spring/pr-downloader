/* This file is part of pr-downloader (GPL v2 or later), see the LICENSE file */

#include "WidgetDownloader.h"
#include "Widget.h"
#include "FileSystem/FileSystem.h"

bool CWidgetDownloader::download(IDownload* download)
{
	return true;
}

/**

	mode
		0: list all widgets
		1: download infos
		2: general info
		3: list all widgets again?
		4: screenshots
		5: download count
		6: ?
		7: unknown mode


	http://spring.vsync.de/luaManager/lua_manager.php?m=0


	Downloads Screenshot:
	http://spring.vsync.de/luaManager/lua_manager.php?m=4&id=100
		<root><File ID="96"><NameId>100</NameId><Url>http://widgetdb.springrts.de/images/100_luaDinamicBlobShadows.jpg</Url></File></root>

	Download Infos
	http://spring.vsync.de/luaManager/lua_manager.php?m=1&id=55
	<root><File ID="76"><Url>http://widgetdb.springrts.de/files/55/LuaUI/Widgets/gui_idle_builders_new.lua</Url><MD5>d1f4898b759f4bf26e781e451def8217</MD5><LocalPath>/LuaUI/Widgets/gui_idle_builders_new.lua</LocalPath><LuaId>55</LuaId></File></root>

	General Info
	http://spring.vsync.de/luaManager/lua_manager.php?m=2&id=55


*/

bool CWidgetDownloader::search(std::list<IDownload*>& result, const std::string& name, IDownload::category cat)
{
	std::string path=fileSystem->getSpringDir();
	path=path+PATH_DELIMITER + "rapid" + PATH_DELIMITER + "Widgets.xml";

	if (!fileSystem->isOlder(path,WIDGET_RECHECK_TIME)) {
		result.clear();
		IDownload dl(path);
		dl.addMirror("http://widgetdb.springrts.de/lua_manager.php?m=0");
		result.push_back(&dl);
		httpDownload->download(result);
	}
	CWidget* widget=new CWidget(path);
//	std::list<IDownload>* widgets = new std::list<IDownload>* widgets;

	delete(widget);
	return true;
}


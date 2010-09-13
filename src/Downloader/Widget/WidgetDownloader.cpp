#ifndef WIDGET_DOWNLOADER_H
#define WIDGET_DOWNLOADER_H

#include "WidgetDownloader.h"
#include "pugixml/pugixml.hpp"
#include "../../FileSystem.h"
#include <iostream>

void CWidgetDownloader::start(IDownload* download){

}

const IDownload* CWidgetDownloader::addDownload(const std::string& url, const std::string& filename){
	return NULL;
}
bool CWidgetDownloader::removeDownload(IDownload& download){
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

std::list<IDownload>* CWidgetDownloader::search(const std::string& name){
	std::string path=fileSystem->getSpringDir();
	path=path+PATH_DELIMITER + "rapid" + PATH_DELIMITER + "Widgets.xml";

	if (!fileSystem->isOlder(path,WIDGET_RECHECK_TIME)){
		httpDownload->addDownload("http://spring.vsync.de/luaManager/lua_manager.php?m=0", path);
		httpDownload->start();
	}

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(path.c_str());
	printf("Parsing %s\n", path.c_str());
	std::cout << "Load result: " << result.description() << ", mesh name: " << doc.child("root").attribute("ID").value() << std::endl;

	return NULL;
}

#endif

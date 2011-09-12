#include "Widget.h"
#include "pugixml/pugixml.hpp"
#include <stdio.h>
#include <list>
#include <iostream>
#include "Util.h"

CWidget::CWidget(const std::string& filename)
{
	pugi::xml_document doc;
//	pugi::xml_parse_result result = doc.load_file(filename.c_str());
	INFO("Parsing %s\n", filename.c_str());
	pugi::xml_node widget=doc.child("root");

	std::list<CWidget> widgets;
	int count=0;
	for (widget = widget.first_child(); widget; widget = widget.next_sibling()) {
		name = widget.child_value("Name");
		changelog =  widget.child_value("Changelog");
		author = widget.child_value("Author");
		/*		std::cout << widget.child_value("Version");
				std::cout << widget.child_value("Rating");
				std::cout << widget.child_value("DownsPerDay");
				std::cout << widget.child_value("CommentCount");
				std::cout << widget.child_value("DownloadCount");
				std::cout << widget.child_value("NameId");
				std::cout << widget.child_value("CategoryId");
				std::cout << widget.child_value("Mods");
				std::cout << widget.child_value("Description");
				std::cout << widget.child_value("Entry");
				std::cout << std::endl; */
		count++;
	}
	INFO("Parsed %d widgets.\n",count);

}

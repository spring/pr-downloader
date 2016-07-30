#include "request_handler.h"
#include <fstream>
#include <sstream>
#include <string>
#include "mime_types.h"
#include "reply.h"
#include "request.h"
#include "lslunitsync/unitsync.h"
#include "lslunitsync/image.h"
#include "lslutils/misc.h"
#include <json/json.h>

namespace http
{
namespace server
{

request_handler::request_handler(const std::string& doc_root)
    : doc_root_(doc_root)
{
}

static void reply_http_ok(reply& rep, const std::string& mimetype)
{
	rep.status = reply::ok;
	rep.headers.resize(2);
	rep.headers[0].name = "Content-Length";
	rep.headers[0].value = std::to_string(rep.content.size());
	rep.headers[1].name = "Content-Type";
	rep.headers[1].value = mimetype;
}

static void create_file_list(reply& rep, const LSL::StringVector& items, const std::string& type)
{
	Json::Value root;
	for (const std::string item : items) {
		root.append(item);
	}
	std::stringstream ss;
	ss << root;
	rep.content.append(ss.str());
}


static bool gameinfo_request(const LSL::StringVector& params, reply& rep)
{
	if (params.size() == 1) {
		const LSL::StringVector games = LSL::usync().GetGameList();
		create_file_list(rep, games, "games/");
		reply_http_ok(rep, "text/html");
		return true;
	}
	if (params.size() == 2) {
		LSL::StringVector types;
		types.push_back("gameinfo");
		create_file_list(rep, types, "games/" + params[1] + "/");
		reply_http_ok(rep, "text/html");
		return true;
	}
	if (params.size() == 3 && params[2] == "gameinfo") {
		rep.content.append("gameinfo of " + params[1]); // return list of ai's, game options, units, etcetc
		reply_http_ok(rep, "text/html");
		return true;
	}
	return false;
}

static bool serve_file(reply& rep, const std::string& path, const std::string& mimetype = "")
{
	// Open the file to send back.
	std::ifstream is(path.c_str(), std::ios::in | std::ios::binary);
	if (!is) {
		//LslError("Couldn't open file %s", path.c_str());
		rep = reply::stock_reply(reply::not_found);
		return false;
	}

	// Fill out the reply to be sent to the client.
	rep.status = reply::ok;
	char buf[512];
	while (is.read(buf, sizeof(buf)).gcount() > 0)
		rep.content.append(buf, is.gcount());

	std::string extension;
	const std::size_t last_dot_pos = path.find_last_of(".");
	const std::size_t last_slash_pos = path.find_last_of("/");
	if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos) {
		extension = path.substr(last_dot_pos + 1);
	}
	rep.headers.resize(2);
	rep.headers[0].name = "Content-Length";
	rep.headers[0].value = std::to_string(rep.content.size());
	rep.headers[1].name = "Content-Type";
	if (mimetype.empty()) {
		rep.headers[1].value = mime_types::extension_to_type(extension);
	} else {
		rep.headers[1].value = mimetype;
	}
	return true;
}

static bool root_request(LSL::StringVector params, reply& rep)
{
	serve_file(rep, "index.html", "text/html");
	return true;
}

static bool mapinfo_request(const LSL::StringVector& params, reply& rep)
{
	if (params.size() == 1) {
		const LSL::StringVector maps = LSL::usync().GetMapList();
		create_file_list(rep, maps, "maps/");
		reply_http_ok(rep, "text/html");
		return true;
	}

	if (params.size() == 2) {
		LSL::StringVector types;
		types.push_back("minimap");
		types.push_back("minimap_thumb");
		types.push_back("metalmap");
		types.push_back("heightmap");
		types.push_back("mapinfo");
		types.push_back("mapoptions");
		create_file_list(rep, types, "maps/" + params[1] + "/");
		reply_http_ok(rep, "text/html");
		return true;
	}

	if (params.size() == 3) {
		LSL::ImageType ityp;
		if (params[2] == "minimap") {
			ityp = LSL::IMAGE_MAP;
		} else if (params[2] == "minimap_thumb") {
			ityp = LSL::IMAGE_MAP_THUMB;
		} else if (params[2] == "metalmap") {
			ityp = LSL::IMAGE_METALMAP;
		} else if (params[2] == "heightmap") {
			ityp = LSL::IMAGE_HEIGHTMAP;
		} else if (params[2] == "mapoptions") {
			return serve_file(rep, LSL::usync().GetMapOptionsPath(params[1]));
		} else if (params[2] == "mapinfo") {
			return serve_file(rep, LSL::usync().GetMapInfoPath(params[1]));
		} else {
			return false;
		}

		const std::string path = LSL::usync().GetMapImagePath(params[1], ityp);
		return serve_file(rep, path);
	}

	return false;
}

bool system_requests(const LSL::StringVector& params, reply& rep)
{
	if (params.size() == 1) {
		LSL::StringVector types;
		types.push_back("reload");
		create_file_list(rep, types, "system/");
		reply_http_ok(rep, "text/html");
		return true;
	}
	if (params.size() == 2) {
		if (params[1] == "reload") {
			LSL::usync().ReloadUnitSyncLib();
			reply_http_ok(rep, "text/html");
			return true;
		}
	}
	return false;
}

void request_handler::handle_request(const request& req, reply& rep)
{
	// Decode url to path.
	std::string request_path;
	if (!url_decode(req.uri, request_path)) {
		rep = reply::stock_reply(reply::bad_request);
		return;
	}

	// Request path must be absolute and not contain "..".
	if (request_path.empty() || request_path[0] != '/' || request_path.find("..") != std::string::npos) {
		rep = reply::stock_reply(reply::bad_request);
		return;
	}

	LSL::StringVector values = LSL::Util::StringTokenize(request_path, "/");

	if (request_path == "/" && root_request(values, rep))
		return;

	if (values.empty()) {
		rep = reply::stock_reply(reply::not_found);
		return;
	}
	if (values[0] == "games" && gameinfo_request(values, rep))
		return;
	if (values[0] == "maps" && mapinfo_request(values, rep))
		return;
	if (values[0] == "system" && system_requests(values, rep))
		return;

	rep = reply::stock_reply(reply::not_found);
	return;
}

bool request_handler::url_decode(const std::string& in, std::string& out)
{
	out.clear();
	out.reserve(in.size());
	for (std::size_t i = 0; i < in.size(); ++i) {
		if (in[i] == '%') {
			if (i + 3 <= in.size()) {
				int value = 0;
				std::istringstream is(in.substr(i + 1, 2));
				if (is >> std::hex >> value) {
					out += static_cast<char>(value);
					i += 2;
				} else {
					return false;
				}
			} else {
				return false;
			}
		} else if (in[i] == '+') {
			out += ' ';
		} else {
			out += in[i];
		}
	}
	return true;
}

} // namespace server
} // namespace http

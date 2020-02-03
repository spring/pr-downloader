#ifndef HTTP_REQUEST_HANDLER_HPP
#define HTTP_REQUEST_HANDLER_HPP

#include <string>
#include "lslutils/misc.h"

namespace http
{
namespace server
{

struct reply;
struct request;
struct server;

/// The common handler for all incoming requests.
class request_handler
{
public:
	request_handler(const request_handler&) = delete;
	request_handler& operator=(const request_handler&) = delete;

	/// Construct with a directory containing files to be served.
	explicit request_handler(const std::string& doc_root, server* s);

	/// Handle a request and produce a reply.
	void handle_request(const request& req, reply& rep);

private:
	void reply_http_ok(reply& rep, const std::string& mimetype);
	void create_file_list(reply& rep, const LSL::StringVector& items, const std::string& type);
	bool gameinfo_request(const LSL::StringVector& params, reply& rep);
	bool serve_file(reply& rep, const std::string& path, const std::string& mimetype);
	bool root_request(LSL::StringVector params, reply& rep);
	bool mapinfo_request(const LSL::StringVector& params, reply& rep);
	bool system_requests(const LSL::StringVector& params, reply& rep);

	/// Perform URL-decoding on a string. Returns false if the encoding was invalid.
	bool url_decode(const std::string& in, std::string& out);

	/// The directory containing the files to be served.
	std::string doc_root_;
	server* const server_;

};

} // namespace server
} // namespace http

#endif // HTTP_REQUEST_HANDLER_HPP


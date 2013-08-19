
#include "XmlRpcCurlClient.h"

#include <curl/curl.h>
#include "XmlRpc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


using namespace XmlRpc;

// Static data
const char XmlRpcCurlClient::REQUEST_BEGIN[] =
  "<?xml version=\"1.0\"?>\r\n"
  "<methodCall><methodName>";
const char XmlRpcCurlClient::REQUEST_END_METHODNAME[] = "</methodName>\r\n";
const char XmlRpcCurlClient::PARAMS_TAG[] = "<params>";
const char XmlRpcCurlClient::PARAMS_ETAG[] = "</params>";
const char XmlRpcCurlClient::PARAM_TAG[] = "<param>";
const char XmlRpcCurlClient::PARAM_ETAG[] =  "</param>";
const char XmlRpcCurlClient::REQUEST_END[] = "</methodCall>\r\n";
const char XmlRpcCurlClient::METHODRESPONSE_TAG[] = "<methodResponse>";
const char XmlRpcCurlClient::FAULT_TAG[] = "<fault>";



XmlRpcCurlClient::XmlRpcCurlClient(CURL* curl, const char* host, int port, const char* uri/*=0*/)
{
	XmlRpcUtil::log(1, "XmlRpcCurlClient new client: host %s, port %d.", host, port);
	_curl = curl;
	_uri = "http://";
	_uri += host;
	_uri += ":";
	_uri += port;
	if (uri)
		_uri += uri;
	else
		_uri += "/RPC2";
  _connectionState = NO_CONNECTION;
  _executing = false;
  _eof = false;

  // Default to keeping the connection open until an explicit close is done
  setKeepOpen();
}


XmlRpcCurlClient::~XmlRpcCurlClient()
{
}

// Close the owned fd
void
XmlRpcCurlClient::close()
{
  XmlRpcUtil::log(4, "XmlRpcCurlClient::close: fd %d.", getfd());
  _connectionState = NO_CONNECTION;
  XmlRpcSource::close();
}


// Clear the referenced flag even if exceptions or errors occur.
struct ClearFlagOnExit {
  ClearFlagOnExit(bool& flag) : _flag(flag) {}
  ~ClearFlagOnExit() { _flag = false; }
  bool& _flag;
};

// Execute the named procedure on the remote server.
// Params should be an array of the arguments for the method.
// Returns true if the request was sent and a result received (although the result
// might be a fault).
bool
XmlRpcCurlClient::execute(const char* method, XmlRpcValue const& params, XmlRpcValue& result)
{
	if ( ! generateRequest(method, params))
		return false;
	curl_easy_setopt(_curl, CURLOPT_URL, _uri.c_str());
	curl_easy_setopt(_curl, CURLOPT_POSTFIELDS, _request.c_str());
	curl_easy_setopt(_curl, CURLOPT_WRITEDATA, this);
	curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, XmlRpcCurlClient::readResponse);
	CURLcode res = curl_easy_perform(_curl);
	XmlRpcUtil::log(1, "XmlRpcCurlClient::execute: method %s (_connectionState %d).", method, _connectionState);
	if(res != CURLE_OK)
		return false;
	if(!parseResponse(result))
		return false;
	XmlRpcUtil::log(1, "XmlRpcCurlClient::execute: method %s completed.", method);
	_response = "";
	return true;
}

// XmlRpcSource interface implementation
// Handle server responses. Called by the event dispatcher during execute.
unsigned
XmlRpcCurlClient::handleEvent(unsigned eventType)
{
	return true;
}


// Connect to the xmlrpc server
bool
XmlRpcCurlClient::doConnect()
{
	return true;
}

// Encode the request to call the specified method with the specified parameters into xml
bool
XmlRpcCurlClient::generateRequest(const char* methodName, XmlRpcValue const& params)
{
  std::string body = REQUEST_BEGIN;
  body += methodName;
  body += REQUEST_END_METHODNAME;

  // If params is an array, each element is a separate parameter
  if (params.valid()) {
    body += PARAMS_TAG;
    if (params.getType() == XmlRpcValue::TypeArray)
    {
      for (int i=0; i<params.size(); ++i) {
        body += PARAM_TAG;
        body += params[i].toXml();
        body += PARAM_ETAG;
      }
    }
    else
    {
      body += PARAM_TAG;
      body += params.toXml();
      body += PARAM_ETAG;
    }

    body += PARAMS_ETAG;
  }
  body += REQUEST_END;

  std::string header = generateHeader(body);
  XmlRpcUtil::log(4, "XmlRpcCurlClient::generateRequest: header is %d bytes, content-length is %d.",
                  header.length(), body.length());

  _request = header + body;
  return true;
}

// Prepend http headers
std::string
XmlRpcCurlClient::generateHeader(std::string const& body)
{
	return "";
}

bool
XmlRpcCurlClient::writeRequest()
{
	return true;
}


// Read the header from the response
bool
XmlRpcCurlClient::readHeader()
{
	return true; // Continue monitoring this source
}


size_t
XmlRpcCurlClient::readResponse(void *ptr, size_t size, size_t nmemb, XmlRpcCurlClient *xmlclient)
{
	const int length = size*nmemb;
	xmlclient->_response.append((char*)ptr, length);
	return length;    // Stop monitoring this source (causes return from work)
}


// Convert the response xml into a result value
bool
XmlRpcCurlClient::parseResponse(XmlRpcValue& result)
{
  // Parse response xml into result
  int offset = 0;
  if ( ! XmlRpcUtil::findTag(METHODRESPONSE_TAG,_response,&offset)) {
    XmlRpcUtil::error("Error in XmlRpcCurlClient::parseResponse: Invalid response - no methodResponse. Response:\n%s", _response.c_str());
    return false;
  }

  // Expect either <params><param>... or <fault>...
  if ((XmlRpcUtil::nextTagIs(PARAMS_TAG,_response,&offset) &&
       XmlRpcUtil::nextTagIs(PARAM_TAG,_response,&offset)) ||
      (XmlRpcUtil::nextTagIs(FAULT_TAG,_response,&offset) && (_isFault = true)))
  {
    if ( ! result.fromXml(_response, &offset)) {
      XmlRpcUtil::error("Error in XmlRpcCurlClient::parseResponse: Invalid response value. Response:\n%s", _response.c_str());
      _response = "";
      return false;
    }
  } else {
    XmlRpcUtil::error("Error in XmlRpcCurlClient::parseResponse: Invalid response - no param or fault tag. Response:\n%s", _response.c_str());
    _response = "";
    return false;
  }

  _response = "";
  return result.valid();
}



#ifndef _XmlRpcCurlClient_H_
#define _XmlRpcCurlClient_H_
//
// XmlRpc++ Copyright (c) 2002-2003 by Chris Morley
//
#if defined(_MSC_VER)
# pragma warning(disable:4786)    // identifier was truncated in debug info
#endif


#ifndef MAKEDEPEND
# include <string>
#endif

#include <curl/curl.h>

namespace XmlRpc {

  // Arguments and results are represented by XmlRpcValues
  class XmlRpcValue;

  //! A class to send XML RPC requests to a server and return the results.
  class XmlRpcCurlClient {
  public:
    // Static data
    static const char REQUEST_BEGIN[];
    static const char REQUEST_END_METHODNAME[];
    static const char PARAMS_TAG[];
    static const char PARAMS_ETAG[];
    static const char PARAM_TAG[];
    static const char PARAM_ETAG[];
    static const char REQUEST_END[];
    // Result tags
    static const char METHODRESPONSE_TAG[];
    static const char FAULT_TAG[];

    //! Construct a client to connect to the server at the specified host:port address
    //!  @param host The name of the remote machine hosting the server
    //!  @param port The port on the remote machine where the server is listening
    //!  @param uri  An optional string to be sent as the URI in the HTTP GET header
    XmlRpcCurlClient(CURL* curl, const char* host, int port, const char* uri=0);

    //! Destructor
    virtual ~XmlRpcCurlClient();

    //! Execute the named procedure on the remote server.
    //!  @param method The name of the remote procedure to execute
    //!  @param params An array of the arguments for the method
    //!  @param result The result value to be returned to the client
    //!  @return true if the request was sent and a result received
    //!   (although the result might be a fault).
    //!
    //! Currently this is a synchronous (blocking) implementation (execute
    //! does not return until it receives a response or an error). Use isFault()
    //! to determine whether the result is a fault response.
    bool execute(const char* method, XmlRpcValue const& params, XmlRpcValue& result);

    //! Returns true if the result of the last execute() was a fault response.
    bool isFault() const { return _isFault; }


    // XmlRpcSource interface implementation
    //! Close the connection
    virtual void close();

    //! Handle server responses. Called by the event dispatcher during execute.
    //!  @param eventType The type of event that occurred.
    //!  @see XmlRpcDispatch::EventType
    virtual unsigned handleEvent(unsigned eventType) {return true;}

  protected:
    // Execution processing helpers
    virtual bool doConnect() { return true; }
    virtual bool generateRequest(const char* method, XmlRpcValue const& params);
    static size_t readResponse(void *ptr, size_t size, size_t nmemb, XmlRpcCurlClient *userdata);
    virtual bool parseResponse(XmlRpcValue& result);

    std::string _uri;

    // The xml-encoded request, http header of response, and response xml
    std::string _request;
    std::string _header;
    std::string _response;

    // True if a fault response was returned by the server
    bool _isFault;


    // curl handle
    CURL* _curl;

  };	// class XmlRpcCurlClient

}	// namespace XmlRpc

#endif	// _XmlRpcCurlClient_H_

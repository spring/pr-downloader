#ifndef LSL_NET_ENUMS_H
#define LSL_NET_ENUMS_H

namespace LSL
{
namespace Enum
{

enum SocketState {
	SS_Closed,
	SS_Connecting,
	SS_Open
};

enum SocketError {
	SE_No_Error,
	SE_NotConnected,
	SE_Resolve_Host_Failed,
	SE_Connect_Host_Failed
};

typedef int Protocolerror;

} //namespace Enum {
} //namespace LSL {


#endif // LSL_NET_ENUMS_H

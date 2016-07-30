/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#ifndef LSL_SOCKET_H
#define LSL_SOCKET_H

#include <boost/signals2/signal.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "enums.h"

namespace LSL
{

//! a wrapper around asio tcp-socket, mostly borrowed from Engine's lobby/connection, but with signals
class Socket
{
public:
	//! cmd_name,params
	boost::signals2::signal<void(std::string, std::string)> sig_dataReceived;
	//! connect_success,msg_if_failed
	boost::signals2::signal<void(bool, std::string)> sig_doneConnecting;
	//! the actual asio::tcp::socket got disconnected
	boost::signals2::signal<void()> sig_socketDisconnected;
	//! error_msg
	boost::signals2::signal<void(std::string)> sig_networkError;

	Enum::SocketState State() const;

	Socket();
	virtual ~Socket();

	void Connect(const std::string& server, int port);
	void Disconnect()
	{
	}

	bool SendData(const std::string& msg);

	void SetSendRateLimit(int Bps = -1);
	int GetSendRateLimit() const
	{
		return m_rate;
	}
	std::string GetHandle() const;
	bool InTimeout(int timeout_seconds) const;
	std::string GetLocalAddress() const;

private:
	void ConnectCallback(const boost::system::error_code& error);
	void ReceiveCallback(const boost::system::error_code& error, size_t bytes);

	boost::asio::io_service m_netservice;
	boost::asio::ip::tcp::socket m_sock;
	boost::asio::streambuf m_incoming_buffer;
	int m_rate;
	time_t m_last_net_packet;
};

} //namespace LSL

#endif // LSL_SOCKET_H

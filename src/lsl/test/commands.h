#ifndef COMMANDS_H
#define COMMANDS_H

#include <boost/signals2.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/format.hpp>
#include <boost/typeof/typeof.hpp>
#include <string>
#include <sstream>
#include <assert.h>
#include <iostream>
#include <lsl/networking/commands.h>

struct ServerEvents {
	typedef boost::signals2::signal<void()> BattleSigType;
	static BattleSigType battleSig;
};

struct User {
	User()
	{
		ServerEvents::battleSig.connect(*this);
	}
	void operator()() const
	{
		std::cout << "I was called" << std::endl;
	}
};

struct Server {
	Server();
	void ExecuteCommand(const std::string& cmd, const std::string& inparams, int replyid);
	void OnNewUser(const std::string& nick, const std::string& country, const int& cpu, const int& id);
};

Server::Server()
{
}

void Server::ExecuteCommand(const std::string& cmd, const std::string& inparams, int replyid)
{
	std::string params = inparams;
	int pos, cpu, id, nat, port, maxplayers, rank, specs, units, top, left, right, bottom, ally, type;
	bool haspass, lanmode = false;
	std::string hash;
	std::string nick, contry, host, map, title, mod, channel, error, msg, owner, ai, supported_spring_version, topic;
	//NatType ntype;
	//    UserStatus cstatus;
	//    UTASClientStatus tasstatus;
	//    UTASBattleStatus tasbstatus;
	//    UserBattleStatus bstatus;
	//    UTASColor color;
}

void Server::OnNewUser(const std::string& nick, const std::string& country, const int& cpu, const int& id)
{
	std::cerr << boost::format("I'm being called bro!\n nick %s -- country %s -- cpu %i -- id %i\n") % nick % country % cpu % id;
}

#endif // COMMANDS_H

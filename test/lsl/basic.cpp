/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#include <lsl/container/userlist.h>
#include <lsl/container/battlelist.h>
#include <lsl/container/channellist.h>
#include <lsl/networking/iserver.h>
#include <lslutils/misc.h>

#include "common.h"
#include "commands.h"

#include <iostream>

#define TESTLIST(name)                                                                          \
	{                                                                                       \
		name instance;                                                                  \
		name::ItemType* k = new name::ItemType();                                       \
		name::PointerType p(k);                                                         \
		instance.Add(p);                                                                \
		assert(instance.size() == 1);                                                   \
		instance.Remove(p->key());                                                      \
		assert(instance.size() == 0);                                                   \
		try {                                                                           \
			instance.Get(p->key());                                                 \
			throw TestFailedException("Get should've thrown MissingItemException"); \
		} catch (LSL::name::MissingItemException & e) {                                 \
			std::cerr << e.what() << std::endl;                                     \
		} catch (std::exception & b) {                                                  \
			std::cerr << b.what() << std::endl;                                     \
			throw TestFailedException("Get should've thrown MissingItemException"); \
		}                                                                               \
	}

//#include <unitsync++/c_api.h>
void dummySync();
int main(int argc, char** argv)
{
	using namespace LSL;
	//    TESTLIST(UserList)
	//    TESTLIST(Battle::BattleList)
	//    TESTLIST(ChannelList)
	/*
          LSL::Server* tas = new LSL::Server();

          tas->Connect("lobby.springrts.com", "lobby.springrts.com", 8200);
          if (argc > 2)
                  tas->Login(argv[1], argv[2]);
          else
                  tas->Login("dummy", "password");
          //    sleep(5);
          tas->JoinChannel("springlobby", "");
  */
	dummySync();

	//	std::string sequence("/root/path/jijij.png");
	//	std::cout	<< "\n\nBeforeLast: " << LSL::Util::BeforeLast(
	//sequence,
	//"/" ) << "\n"
	//				<< "AfterLast: " << LSL::Util::AfterLast( sequence, "/" )
	//<<
	//"\n"
	//				<< "BeforeFirst: " << LSL::Util::BeforeFirst( sequence,
	//"/"
	//) << "\n"
	//				<< "AfterFirst: " << LSL::Util::AfterFirst( sequence, "/"
	//)
	//<< "\n";

	return 0;
}

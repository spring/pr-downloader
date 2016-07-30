/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#include <boost/signals2.hpp>
#include <boost/function.hpp>

#include <iostream>

namespace LSL
{
boost::signals2::signal<void()> battleSig;

struct User
{
	User(const std::string& id, const std::string& nick,
	     const std::string& country, const int cpu)
	{
		battleSig.connect(*this);
	}
	void operator()() const
	{
		std::cout << "I was called" << std::endl;
	}
};

struct Battle
{
	void update()
	{
		battleSig();
	}
};
}

int main(int, char**)
{
	using namespace LSL;
	Battle battle;
	battle.update();
}

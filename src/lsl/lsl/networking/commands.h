#ifndef ISERVER_PRIVATE_H
#define ISERVER_PRIVATE_H

#include <string>
#include <sstream>
#include <map>
#include <boost/tuple/tuple.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <lslutils/conversion.h>
#include "tasserver.h"

namespace BT = boost::tuples;

namespace LSL
{
/** \param params pre: string with N >= 0 seperators
 *                post: N==0: empty string, N>0: everything after the first seperator
 * \param sep the character params is split at
 * \return N==0: params, N>0:everything before the first seperator
 **/
inline std::string GetParamByChar(std::string& params, const char sep)
{
	const std::string::size_type pos = params.find(sep);
	std::string ret;
	if (pos != std::string::npos) {
		ret = params.substr(0, pos); //inclusive??
		params = params.substr(pos + 1);
	} else {
		ret = params;
		params = "";
	}
	return ret;
}

//! convenience wrapper around GetParamByChar for single whitespace delimited words
inline std::string GetWordParam(std::string& params)
{
	return GetParamByChar(params, ' ');
}

//! convenience wrapper around GetParamByChar for tabulator delimited sentences
inline std::string GetSentenceParam(std::string& params)
{
	return GetParamByChar(params, '\t');
}

//! convenience wrapper around GetParamByChar for whitespace delimited integers
inline long GetIntParam(std::string& params)
{
	const std::string d = GetParamByChar(params, ' ');
	return Util::FromIntString(d);
}

//! convenience wrapper around GetParamByChar for whitespace delimited booleans
inline bool GetBoolParam(std::string& params)
{
	return (bool)GetIntParam(params);
}

namespace Tokens
{
//! base class for all tokens with a call operator that yields the actual value
template <class TypeImp>
struct Basic
{
	typedef TypeImp
	    real_type;
	const real_type value;

	Basic(real_type v)
	    : value(v)
	{
	}

	real_type operator()() const
	{
		return value;
	}
};
struct Word : public Basic<std::string>
{
	Word(std::string& params)
	    : Basic<std::string>(GetWordParam(params))
	{
	}
};
struct Sentence : public Basic<std::string>
{
	Sentence(std::string& params)
	    : Basic<std::string>(GetWordParam(params))
	{
	}
};
struct Int : public Basic<int>
{
	Int(std::string& params)
	    : Basic<int>(GetIntParam(params))
	{
	}
};
struct Float : public Basic<float>
{
	Float(std::string& params)
	    : Basic<float>(GetIntParam(params))
	{
	}
};
struct Double : public Basic<double>
{
	Double(std::string& params)
	    : Basic<double>(GetIntParam(params))
	{
	}
};
//! this effectively ends further parsing by consuming all params
struct All : public Basic<std::string>
{
	All(std::string& params)
	    : Basic<std::string>(params)
	{
		params = "";
	}
};
//struct NoToken{
//	template < class T > NoToken( T& ){}
//};

#define NoToken boost::tuples::null_type

} //namespace Tokens
} //namespace LSL


#include "function_templates.h"

namespace LSL
{

/** \brief base class for all Command subtypes
 * Since every Command is a succinct type we need a
 * base class with a virtual porcess call to be able to keep them in
 * a map in CommandDictionary
 */
struct CommandBase
{
	virtual void process(std::string& /*params*/)
	{
		assert(false); //means we've called a non-mapped command
	}
};

/** \brief Protocol command handler abstraction
 * encapsulate constructing and calling a given function pointer into a boost::function for
 * handling a command with max. 9 Tokens as paramters. Using Signature::call
 * automates parsing a given input string in order of the input Token Types.
 * \todo should prolly be named CommandHandler instead
 **/
template <class T0 = NoToken, class T1 = NoToken, class T2 = NoToken, class T3 = NoToken, class T4 = NoToken,
	  class T5 = NoToken, class T6 = NoToken, class T7 = NoToken, class T8 = NoToken, class T9 = NoToken>
struct Command : public CommandBase
{
	typedef boost::tuples::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>
	    TokenTuple;
	typedef Signature<TokenTuple, boost::tuples::length<TokenTuple>::value>
	    SignatureType;
	typedef typename SignatureType::Type
	    CallBackType;
	CallBackType func;
	/** \tparam F the member function pointer type to be called
      * \tparam X the Tasserver type
      * \param x a TASServer type instance on which f will be called
      * \param f the member function pointer
      */
	template <class F, class X>
	Command(F f, X* x)
	    : func(SignatureType::make(f, x))
	{
	}
	virtual void process(std::string& params)
	{
		SignatureType::call(func, params);
	}
};

/**
 * A mapping from Tasserver commands onto CommandBase instances
 */
class CommandDictionary
{
private:
	//! only TASSERVER can construct a CommandDictionary
	friend class ServerImpl;
	CommandDictionary(ServerImpl* tas);

	ServerImpl* m_tas;
	typedef std::map<std::string, boost::shared_ptr<CommandBase> >
	    MapType;
	MapType cmd_map_;

public:
	void Process(const std::string& cmd, std::string& params) const;
};

} //namespace LSL

#endif // ISERVER_PRIVATE_H

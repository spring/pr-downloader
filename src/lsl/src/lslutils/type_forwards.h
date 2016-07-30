#ifndef LSL_DATATYPES_H
#define LSL_DATATYPES_H

//there's no good way to forward these
#include <string>
#include <map>
#include <set>
#include <vector>

#ifdef HAVE_WX
class wxArrayString;
#endif

namespace boost
{
template <class T>
class shared_ptr;
}

namespace std
{
}

namespace LSL
{
namespace Battle
{
class IBattle;
class Battle;
struct BattleOptions;
}

template <class T, bool TDestroy>
class RefcountedPointer;

namespace TDF
{
class Node;
typedef RefcountedPointer<Node, true> PNode;
class DataList;
typedef RefcountedPointer<DataList, true> PDataList;
class DataLeaf;
typedef RefcountedPointer<DataLeaf, true> PDataLeaf;
}

class lslColor;
class CommonUser;
class User;
class Channel;
class Server;
struct UnitsyncMap;
struct UnitsyncGame;
struct UserBattleStatus;
struct UserPosition;
class OptionsWrapper;
struct GameOptions;
class Spring;

//! @brief map used internally by the iServer class to calculate ping roundtimes.
typedef std::map<int, long long> PingList;

typedef std::map<std::string, std::string> StringMap;
typedef std::vector<std::string> StringVector;
typedef std::set<std::string> StringSet;

typedef boost::shared_ptr<User> UserPtr;
typedef boost::shared_ptr<const User> ConstUserPtr;

typedef boost::shared_ptr<Battle::IBattle> IBattlePtr;
typedef boost::shared_ptr<const Battle::IBattle> ConstIBattlePtr;

typedef boost::shared_ptr<Battle::Battle> BattlePtr;
typedef boost::shared_ptr<const Battle::Battle> ConstBattlePtr;

typedef boost::shared_ptr<Channel> ChannelPtr;
typedef boost::shared_ptr<const Channel> ConstChannelPtr;

typedef boost::shared_ptr<Server> IServerPtr;
typedef boost::shared_ptr<const Server> ConstIServerPtr;

typedef boost::shared_ptr<CommonUser> CommonUserPtr;
typedef boost::shared_ptr<const CommonUser> ConstCommonUserPtr;

typedef std::vector<UserPtr> UserVector;
typedef std::vector<ConstUserPtr> ConstUserVector;

typedef std::vector<CommonUserPtr> CommonUserVector;
typedef std::vector<ConstCommonUserPtr> ConstCommonUserVector;

typedef boost::shared_ptr<OptionsWrapper> OptionsWrapperPtr;
typedef boost::shared_ptr<const OptionsWrapper> ConstOptionsWrapperPtr;

typedef boost::shared_ptr<Spring> SpringPtr;

struct mmOptionBool;
struct mmOptionFloat;
struct mmOptionString;
struct mmOptionList;
struct mmOptionSection;

typedef std::map<std::string, mmOptionBool> OptionMapBool;
typedef std::map<std::string, mmOptionFloat> OptionMapFloat;
typedef std::map<std::string, mmOptionString> OptionMapString;
typedef std::map<std::string, mmOptionList> OptionMapList;
typedef std::map<std::string, mmOptionSection> OptionMapSection;

typedef std::map<std::string, mmOptionBool>::iterator OptionMapBoolIter;
typedef std::map<std::string, mmOptionFloat>::iterator OptionMapFloatIter;
typedef std::map<std::string, mmOptionString>::iterator OptionMapStringIter;
typedef std::map<std::string, mmOptionList>::iterator OptionMapListIter;
typedef std::map<std::string, mmOptionSection>::iterator OptionMapSectionIter;

typedef std::map<std::string, mmOptionBool>::const_iterator OptionMapBoolConstIter;
typedef std::map<std::string, mmOptionFloat>::const_iterator OptionMapFloatConstIter;
typedef std::map<std::string, mmOptionString>::const_iterator OptionMapStringConstIter;
typedef std::map<std::string, mmOptionList>::const_iterator OptionMapListConstIter;
typedef std::map<std::string, mmOptionSection>::const_iterator OptionMapSectionConstIter;

} //namespace LSL
#endif //LSL_DATATYPES_H

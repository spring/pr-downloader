/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#ifndef LSL_MMOPTIONSORAPPER_H_
#define LSL_MMOPTIONSORAPPER_H_

#include <vector>
#include <utility>
#include <map>
#include <string>

#include <lslutils/type_forwards.h>
#include "enum.h"

namespace LSL
{

struct mmOptionSection;
struct GameOptions;

struct dummyConfig
{
};


class OptionsWrapper
{
public:
	//! public types
	typedef std::pair<std::string, std::string> stringPair;
	typedef std::pair<std::string, stringPair> stringTriple;
	typedef std::vector<stringPair> stringPairVec;
	typedef std::vector<stringTriple> stringTripleVec;
	typedef std::map<std::string, std::string> stringMap;
	typedef std::map<int, GameOptions> GameOptionsMap;

	//does nothing
	OptionsWrapper();
	virtual ~OptionsWrapper();

	bool loadAIOptions(const std::string& gamename, int aiindex, const std::string& ainick);

	int GetAIOptionIndex(const std::string& nick) const;

	//! load corresponding options through unitsync calls
	/*!
	 * the containers for corresponding flag are recreated and then gets the number of options from unitsync
	 * and adds them one by one  to the appropriate container
	 * \param flag decides which type of option to load
	 * \param name Mod/Mapname
	 * \return true if load successful, false otherwise
	 */
	bool loadOptions(Enum::GameOption flag, const std::string& name);
	//! checks if given key can be found in specified container
	/*!
	 * \param key the key that should be checked for existance in containers
	 * \param flag which GameOption conatiner should be searched
	 * \param showError if true displays a messagebox if duplicate key is found
	 * \param optType will contain the corresponding OptionType if key is found, opt_undefined otherwise
	 * \return true if key is found, false otherwise
	 */
	bool keyExists(const std::string& key, const LSL::Enum::GameOption flag, const bool showError, Enum::OptionType& optType) const;
	//! checks if given key can be found in all containers
	/*!
	 * \param key the key that should be checked for existance in containers
	 * \return true if key is found, false otherwise
	 */
	bool keyExists(const std::string& key) const;
	//! checks which container this key belongs to
	/*!
	 * \param key the key that should be checked for existance in containers
	 * \return they container section
	 */
	LSL::Enum::GameOption GetSection(const std::string& key) const;
	//! given a vector of key/value pairs sets the appropriate options to new values
	/*!	Every new value is tested for meeting boundary conditions, type, etc.
	 * If test fails error is logged and false is returned.
	 * \param values the std::stringPairVec containing key / new value pairs
	 * \param flag which OptionType is to be processed
	 * \return false if ANY error occured, true otherwise
	 */
	bool setOptions(stringPairVec* values, Enum::GameOption flag);
	//! get all options of one GameOption
	/*! the output has the following format: < std::string , Pair < std::string , std::string > >
	 * meaning < key , < name , value > >
	 * \param triples this will contain the options after the function
	 * \param flag which OptionType is to be processed
	 */
	stringTripleVec getOptions(Enum::GameOption flag) const;
	//! similar to getOptions, instead of vector a map is used and the name is not stored
	std::map<std::string, std::string> getOptionsMap(Enum::GameOption) const;

	//! returns value of specified key
	/*! searches all containers for key
	 * \return value of key if key found, "" otherwise
	 */
	std::string getSingleValue(const std::string& key) const;
	//! returns value of specified key
	/*! searches containers of type flag for key
	 * \return value of key if key found, "" otherwise
	 */

	std::string getSingleValue(const std::string& key, Enum::GameOption flag) const;

	std::string getDefaultValue(const std::string& key, Enum::GameOption flag) const;

	//! sets a single option in specified container
	/*! \return true if success, false otherwise */
	bool setSingleOption(const std::string& key, const std::string& value, Enum::GameOption modmapFlag);
	//! same as before, but tries all containers
	bool setSingleOption(const std::string& key, const std::string& value);

	//! returns the option type of specified key (all containers are tried)
	Enum::OptionType GetSingleOptionType(const std::string& key) const;

	//!returns the cbx_choice associated w current listoption
	std::string GetNameListOptValue(const std::string& key, Enum::GameOption flag) const;

	//! returns the listitem key associated with listitem name
	std::string GetNameListOptItemKey(const std::string& optkey, const std::string& itemname, Enum::GameOption flag) const;

	GameOptionsMap m_opts;

private:
	//! recreates ALL containers
	void unLoadOptions();
	//! recreates the containers of corresponding flag
	void unLoadOptions(Enum::GameOption flag);

	//! used for code clarity in setOptions()
	bool setSingleOptionTypeSwitch(const std::string& key, const std::string& value, Enum::GameOption modmapFlag, Enum::OptionType optType);

	//! a map that connects the ai nick with it's set of options
	std::map<std::string, int> m_ais_indexes;

	typedef GameOptionsMap::const_iterator
	    GameOptionsMapCIter;
};

} // namespace LSL {

#endif /*LSL_MMOPTIONSORAPPER_H_*/

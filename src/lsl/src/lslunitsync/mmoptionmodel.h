/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#ifndef LSL_MMOPTIONMODEL_H_
#define LSL_MMOPTIONMODEL_H_

#include <string>
#include <vector>

#include "enum.h"

namespace LSL
{

namespace Constants
{
const std::string nosection_name("none");
}

//! used to hold an item in an option list
/*! An option list is made of a variable number of theses items.
 * Each item itself (should) contain a key, name and description.
 */
struct listItem {
	listItem(std::string key_, std::string name_, std::string desc_);

	std::string key;
	std::string name;
	std::string desc;
};

//! Used in option list
typedef std::vector<listItem> ListItemVec;

//! The base class for all option types
/*! All members and functions are public (also in derived classes).
 * Therefore no sanity checks whatsoever are performed when changing a member.
 * Default constructors are mostly provided for compability with stl containers.
 */
struct mmOptionModel {
	//! sets members accordingly
	///* this ctor sets controltype enum according to string *///
	mmOptionModel(std::string name_, std::string key_, std::string description_, Enum::OptionType type_ = Enum::opt_undefined,
		      std::string section_ = Constants::nosection_name);

	virtual ~mmOptionModel();
	//! all members are set to empty strings, type to opt_undefined
	mmOptionModel();

	std::string name, key, description;
	Enum::OptionType type;
	std::string section;
};

//! Holds a bool option
/*! difference from parent: members def and value are bool */
struct mmOptionBool : public mmOptionModel {
	//! sets members accordingly
	mmOptionBool(std::string name_, std::string key_, std::string description_, bool def_,
		     std::string section_ = Constants::nosection_name);
	//! sets wxstring member to "" and bool members to false
	mmOptionBool();
	bool def;
	//! this will always represent the current value, also the only member that should change after creation
	bool value;
};

//! Holds a float option
struct mmOptionFloat : public mmOptionModel {
	//! sets members accordingly
	mmOptionFloat(std::string name_, std::string key_, std::string description_, float def_, float stepping_, float min_, float max_,
		      std::string section_ = Constants::nosection_name);
	//! sets wxstring member to "" and float members to 0.0
	mmOptionFloat();

	float def;
	//! this will always represent the current value, also the only member that should change after creation
	float value;

	//! the increment with that value may change in min,max boundaries
	float stepping;
	float min, max;
};

//! Holds a string option
struct mmOptionString : public mmOptionModel {
	//! sets members accordingly
	mmOptionString(std::string name_, std::string key_, std::string description_, std::string def_, unsigned int max_len_,
		       std::string section_ = Constants::nosection_name);
	//! sets wxstring member to "" and max_len to 0
	mmOptionString();

	//! should not exceed max length (not ensured)
	std::string def;
	//! this will always represent the current value,
	/*! the only member that should change after creation, before set check if new value exceeds max_len*/
	std::string value;
	//! the maximum lentgh the value string may have
	unsigned int max_len;
};

//! Holds a an option list (a vector of listItems)
/*! Most complex of option types. A convenience method for adding new Listitems is provided,
 * as well as a StringVector that contains the names of the added Listitems (useful for comboboxes)
 */
struct mmOptionList : public mmOptionModel {
	//! sets members accordingly; listitems,cbx_choices remain empty
	mmOptionList(std::string name_, std::string key_, std::string description_, std::string def_,
		     std::string section_ = Constants::nosection_name);
	//! def, value are set to ""; listitems,cbx_choices remain empty
	mmOptionList();

	//! creates a new listitem from params, pushes it on the vector and adds name_ to cbx_choices
	void addItem(std::string key_, std::string name_, std::string desc_);

	std::string def;
	//! will always reflect the name of the currently "selected" listitem
	std::string value;
	//! index of current value in cbx_choices, so one can assign new combox value
	int cur_choice_index;
	//! holds a variable amount of ListItems
	ListItemVec listitems;
	//! for easy mapping to a combobox
	std::vector<std::string> cbx_choices;
};


struct mmOptionSection : public mmOptionModel {
	mmOptionSection(std::string name_, std::string key_, std::string description_,
			std::string section_ = Constants::nosection_name);
	mmOptionSection();
};

} //end namespace LSL

#endif /*LSL_MMOPTIONMODEL_H_*/

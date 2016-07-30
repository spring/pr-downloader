#ifndef LSL_USYNC_ENUMS_H
#define LSL_USYNC_ENUMS_H

namespace LSL
{
namespace Enum
{

//! enum that lets us differentiate option types at runtime
/*! opt_undefined will be returned/set if the type could not be determined, others respectively */
enum OptionType {
	opt_undefined = 0,
	opt_bool = 1,
	opt_list = 2,
	opt_float = 3,
	opt_string = 4,
	opt_section = 5
};

//! enum to differentiate option category easily at runtime
enum GameOption {
	PrivateOptions = 3,
	EngineOption = 2,
	MapOption = 1,
	ModOption = 0,
	LastOption = 6
}; // should reflect: optionCategoriesCount


} //namespace Enum {
} //namespace LSL {


#endif // LSL_USYNC_ENUMS_H

#ifndef BASE64_H
#define BASE64_H

namespace LSL
{

struct base64
{
	template <class T>
	static std::string encode(const T&, int)
	{
		return "";
	}
};

} // namespace LSL {

#endif // BASE64_H

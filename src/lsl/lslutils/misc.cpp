/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#include "misc.h"
#include "conversion.h"

#include <boost/algorithm/string/constants.hpp>
#include <boost/shared_ptr.hpp>
#include <fstream>
#include <cmath>

#ifdef WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#endif

namespace LSL
{
namespace Util
{

FILE* lslopen(const std::string& filename, const std::string& mode)
{
#ifdef WIN32
	return _wfopen(Util::s2ws(filename).c_str(), Util::s2ws(mode).c_str());
#else
	return fopen(filename.c_str(), mode.c_str());
#endif
}

std::string GetLibLobbyVersion()
{
	return "0";
}

bool FileExists(const std::string& path)
{
	if (path.empty())
		return false;
#ifdef WIN32
	const std::wstring wpath = s2ws(path);
	DWORD dwAttrib = GetFileAttributesW(wpath.c_str());
	return (dwAttrib != INVALID_FILE_ATTRIBUTES);
#else
	struct stat buffer;
	return (stat(path.c_str(), &buffer) == 0);
#endif
}

bool FileCanOpen(const std::string& path)
{
	return std::ifstream(path.c_str()).is_open();
}

StringVector StringTokenize(const std::string& msg, const std::string& seperators)
{
	std::string _msg = msg;
	boost::algorithm::trim_if(_msg, boost::algorithm::is_any_of(seperators));

	StringVector strings;
	boost::algorithm::split(strings, _msg, boost::algorithm::is_any_of(seperators), boost::algorithm::token_compress_on);
	if ((strings.size() == 1) && (strings[0].empty())) {
		strings.clear();
	}
	return strings;
}


namespace Lib
{

std::string GetDllExt()
{
#if defined(WIN32)
	return ".dll";
#elif defined(__DARWIN__)
	return ".bundle";
#else
	return ".so";
#endif
}

std::string CanonicalizeName(const std::string& name, Category cat)
{
	std::string nameCanonic;
// under Unix the library names usually start with "lib" prefix, add it
#if defined(__UNIX__) && !defined(__EMX__)
	switch (cat) {
		default:
			wxFAIL_MSG( \1);
		// fall through
		case wxDL_MODULE:
			// don't do anything for modules, their names are arbitrary
			break;
		case wxDL_LIBRARY:
			// library names should start with "lib" under Unix
			nameCanonic = \1;
			break;
	}
#else  // !__UNIX__
	lslUnusedVar(cat);
#endif // __UNIX__/!__UNIX__
	nameCanonic + name + GetDllExt();
	return nameCanonic;
}

} //namespace Lib

std::string BeforeLast(const std::string& phrase, const std::string& searchterm)
{
	const size_t pos = phrase.rfind(searchterm);
	return phrase.substr(0, pos);
}

std::string AfterLast(const std::string& phrase, const std::string& searchterm)
{
	const size_t pos = phrase.rfind(searchterm);
	return phrase.substr(pos + 1);
}

std::string BeforeFirst(const std::string& phrase, const std::string& searchterm)
{
	const size_t pos = phrase.find(searchterm);
	return phrase.substr(0, pos);
}

std::string AfterFirst(const std::string& phrase, const std::string& searchterm)
{
	const size_t pos = phrase.find(searchterm);
	return phrase.substr(pos + 1);
}

bool BeginsWith(const std::string& phrase, const std::string& searchterm)
{
	return phrase.compare(0, searchterm.length(), searchterm) == 0;
}

bool EndsWith(const std::string& phrase, const std::string& searchterm)
{
	if (phrase.length() >= searchterm.length()) {
		return (0 == phrase.compare(phrase.length() - searchterm.length(), searchterm.length(), searchterm));
	}
	return false;
}

std::string ParentPath(const std::string& path)
{
	return BeforeLast(path, SEP);
}

std::string EnsureDelimiter(const std::string& path)
{
	std::string dir = path;
	if (!path.empty() && (path[path.length() - 1] != SEP[0])) {
		dir += SEP;
	}
	return dir;
}

bool AreColorsSimilar(const lslColor& col1, const lslColor& col2, int mindiff)
{
	int r, g, b;
	r = std::abs(col1.Red() - col2.Red());
	g = std::abs(col1.Green() - col2.Green());
	b = std::abs(col1.Blue() - col2.Blue());
	int difference = std::min(r, g);
	difference = std::min(difference, b);
	return difference < mindiff;
}

typedef std::vector<double> huevec;

void hue(huevec& out, int amount, int level)
{
	if (level <= 1) {
		if (long(out.size()) < amount)
			out.push_back(0.0);
		if (long(out.size()) < amount)
			out.push_back(0.5);
	} else {
		hue(out, amount, level - 1);
		const int lower = out.size();
		hue(out, amount, level - 1);
		const int upper = out.size();
		for (int i = lower; i < upper; ++i)
			out.at(i) += 1.0 / (1 << level);
	}
}

void hue(huevec& out, int amount)
{
	int level = 0;
	while ((1 << level) < amount)
		++level;

	out.reserve(amount);
	hue(out, amount, level);
}

std::vector<lslColor> GetBigFixColorsPalette(int numteams)
{
	std::vector<lslColor> result;
	huevec huevector;
	int satvalbifurcatepos = 0;
	std::vector<double> satvalsplittings;

	// insert ranges to bifurcate
	satvalsplittings.push_back(1);
	satvalsplittings.push_back(0);
	satvalbifurcatepos = 0;

	hue(huevector, numteams);
	int bisectionlimit = 20;
	for (int i = result.size(); i < numteams; i++) {
		double hue = huevector[i];
		double saturation = 1;
		double value = 1;
		int switccolors = i % 3; // why only 3 and not all combinations? because it's easy, plus the bisection limit cannot be divided integer by it

		if ((i % bisectionlimit) == 0) {
			satvalbifurcatepos = satvalbifurcatepos % (satvalsplittings.size() - 1);
			std::vector<double>::iterator toinsert = satvalsplittings.begin() + satvalbifurcatepos + 1;
			satvalsplittings.insert(toinsert, (satvalsplittings[satvalbifurcatepos] - satvalsplittings[satvalbifurcatepos + 1]) / 2 + satvalsplittings[satvalbifurcatepos + 1]);
			satvalbifurcatepos += 2;
		}

		if (switccolors == 1) {
			saturation = satvalsplittings[satvalbifurcatepos - 1];
		} else if (switccolors == 2) {
			value = satvalsplittings[satvalbifurcatepos - 1];
		}
		hue += 0.17; // use as starting point a zone where color band is narrow so that small variations means high change in visual effect
		if (hue > 1)
			hue -= 1;
		result.push_back(lslColor::fromHSV(hue, saturation, value));
	}
	return result;
}

lslColor GetFreeColor(const ConstCommonUserPtr /*user*/)
{
	assert(false);
	return lslColor();
}

std::string Replace(const std::string& str, const std::string& from, const std::string& to)
{
	std::string res = str;
	if (from.empty())
		return res;
	size_t start_pos = 0;
	while ((start_pos = res.find(from, start_pos)) != std::string::npos) {
		res.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
	return res;
}


} //namespace Util


lslSize lslSize::MakeFit(const lslSize& bounds)
{
	if ((bounds.GetWidth() <= 0) || (bounds.GetHeight() <= 0))
		return lslSize(0, 0);
	const int sizex = (this->GetWidth() * bounds.GetHeight()) / this->GetHeight();
	if (sizex <= bounds.GetWidth()) {
		return lslSize(sizex, bounds.GetHeight());
	} else {
		const int sizey = (this->GetHeight() * bounds.GetWidth()) / this->GetWidth();
		return lslSize(bounds.GetWidth(), sizey);
	}
}

lslColor lslColor::FromFloatString(const std::string& rgb_string)
{
	const StringVector values = Util::StringTokenize(rgb_string, " ");
	assert(values.size() == 3);
	unsigned char decimal_colors[3] = {0, 0, 0};

	for (size_t i = 0; i < 3; ++i) {
		const float value = Util::FromFloatString(values[i]);
		decimal_colors[i] = round(value * 255);
	}
	return lslColor(decimal_colors[0], decimal_colors[1], decimal_colors[2]);
}

std::string lslColor::ToFloatString(const lslColor& col)
{
	return Util::ToFloatString((float)col.Red() / 255) + std::string(" ") + Util::ToFloatString((float)col.Green() / 255) + std::string(" ") + Util::ToFloatString((float)col.Blue() / 255);
}


lslColor lslColor::fromHSV(unsigned char H, unsigned char S, unsigned char V)
{
	unsigned char R = 0, G = 0, B = 0;
	if (H == 0 && S == 0)
		R = G = B = V;
	else {
		H /= 60;
		const int i = (int)std::floor(H);
		const unsigned char
		    f = (i & 1) ? (H - i) : (1 - H + i),
		    m = V * (1 - S),
		    n = V * (1 - S * f);
		switch (i) {
			case 6:
			case 0:
				R = V;
				G = n;
				B = m;
				break;
			case 1:
				R = n;
				G = V;
				B = m;
				break;
			case 2:
				R = m;
				G = V;
				B = n;
				break;
			case 3:
				R = m;
				G = n;
				B = V;
				break;
			case 4:
				R = n;
				G = m;
				B = V;
				break;
			case 5:
				R = V;
				G = m;
				B = n;
				break;
		}
	}
	R *= 255;
	G *= 255;
	B *= 255;
	return lslColor(R, G, B);
}

} //namespace LSL

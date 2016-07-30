/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#ifndef LSL_IMAGE_H
#define LSL_IMAGE_H

#include <string>

//we really, really don't want to include the cimg
// header here, it's 2.1MB of template magic :)
namespace cimg_library
{
template <class T>
struct CImg;
}

#ifdef HAVE_WX
class wxBitmap;
class wxImage;
#endif

namespace LSL
{

namespace Util
{
template <class T>
class uninitialized_array;
}

/** we use this class mostly to hide the cimg implementation details
 * \todo decide/implement COW
 */
class UnitsyncImage
{
private:
	typedef unsigned short RawDataType;
	typedef cimg_library::CImg<RawDataType> PrivateImageType;

public:
	UnitsyncImage(const UnitsyncImage& other);
	UnitsyncImage();
	~UnitsyncImage();
	UnitsyncImage& operator=(const UnitsyncImage& other);
	UnitsyncImage(int width, int height);
	UnitsyncImage(const std::string& filename);

	//! delegates save to cimg library, format is deducted from last path compoment (ie. after the last dot)
	bool Save(const std::string& path) const;
	//! same principle as \ref Save
	bool Load(const std::string& path) const;

	/** \name factory functions
   * \brief creating UnitsyncImage from raw data pointers
   **/
	///@{
	static UnitsyncImage FromMinimapData(const RawDataType* data, int width, int height);
	static UnitsyncImage FromHeightmapData(const Util::uninitialized_array<unsigned short>& data, int width, int height);
	static UnitsyncImage FromMetalmapData(const Util::uninitialized_array<unsigned char>& data, int width, int height);
	static UnitsyncImage FromVfsFileData(Util::uninitialized_array<char>& data, size_t size, const std::string& fn, bool useWhiteAsTransparent = true);
///@}

#ifdef HAVE_WX
	wxBitmap wxbitmap() const;
	wxImage wximage() const;
#endif
	int GetWidth() const;
	int GetHeight() const;
	void Rescale(const int new_width, const int new_height);
	//rescale image to a max resolution 512x512 with keeping aspect ratio
	void RescaleIfBigger(const int maxwidth = 512, const int maxheight = 512);

	bool isValid() const
	{
		return ((m_data_ptr != nullptr) && (GetWidth() > 0) && (GetHeight() > 0));
	}
	// makes given color transparent
	void MakeTransparent(unsigned short r = 255, unsigned short g = 255, unsigned short b = 255);

private:
	UnitsyncImage(PrivateImageType* ptr);
	static PrivateImageType* NewImagePtr(int width = 0, int height = 0);
	PrivateImageType* m_data_ptr;
};

} //namespace LSL

#endif // LSL_IMAGE_H

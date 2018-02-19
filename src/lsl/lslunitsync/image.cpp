/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#include "image.h"

#include <cstdio>

//these need to go before cimg
#ifdef HAVE_WX
#include <wx/image.h>
#include <wx/bitmap.h>
#endif
#define cimg_display 0
#define cimg_verbosity 1
//#define cimg_use_openmp 1
#define cimg_use_png 1 //libpng
//#define cimg_use_jpeg 1 //jpeg
//#define cimg_use_tiff 1
#include <cimg/CImg.h>
#include <lslutils/misc.h>
#include <lslutils/logging.h>
#include <lslutils/conversion.h>
#include <stdio.h> //fmemopen


#if defined(WIN32)

std::FILE* fmemopen(void* data, size_t size, const char* mode)
{
	wchar_t buf[MAX_PATH];
	const size_t len = GetTempPathW(MAX_PATH, buf);
	if (len <= 0) {
		return nullptr;
	}

	std::wstring tempFile(buf, len);
	tempFile += L"tempFile-fmemopen";

	FILE* f = _wfopen(tempFile.c_str(), L"wb+");
	if (f == nullptr)
		return nullptr;
	fwrite(data, size, 1, f);
	rewind(f);
	return f;
}

#elif defined(__APPLE__)
//! we need our own fmemopen implementation since its posix only
std::FILE* fmemopen(void* data, size_t size, const char* /*mode*/)
{
	FILE* f = std::tmpfile();
	if (f == nullptr) {
		return nullptr;
	}
	fwrite(data, size, 1, f);
	rewind(f);
	return f;
}
#endif // !defined(HAVE_FMEMOPEN)

namespace cimg_library
{

template <class T>
//! extends cimg to loading images from in-memory buffer
void load_mem(LSL::Util::uninitialized_array<char>& data, size_t size, const std::string& fn, CImg<T>& img)
{
	const char* filename = fn.c_str();

	std::FILE* file = fmemopen((void*)data, size, "rb");
	if (file == nullptr) {
		LslError("fmemopen(): couldn't create tmpfile for %s!", fn.c_str());
		return;
	}

	const char* const ext = cimg::split_filename(filename);
	cimg::exception_mode() = 0;
#ifdef cimg_load_plugin
	cimg_load_plugin(filename);
#endif
#ifdef cimg_load_plugin1
	cimg_load_plugin1(filename);
#endif
#ifdef cimg_load_plugin2
	cimg_load_plugin2(filename);
#endif
#ifdef cimg_load_plugin3
	cimg_load_plugin3(filename);
#endif
#ifdef cimg_load_plugin4
	cimg_load_plugin4(filename);
#endif
#ifdef cimg_load_plugin5
	cimg_load_plugin5(filename);
#endif
#ifdef cimg_load_plugin6
	cimg_load_plugin6(filename);
#endif
#ifdef cimg_load_plugin7
	cimg_load_plugin7(filename);
#endif
#ifdef cimg_load_plugin8
	cimg_load_plugin8(filename);
#endif
	// ASCII formats
	if (!cimg::strcasecmp(ext, "asc"))
		img.load_ascii(file);
	else if (!cimg::strcasecmp(ext, "dlm") ||
		 !cimg::strcasecmp(ext, "txt"))
		img.load_dlm(file);

	// 2d binary formats
	else if (!cimg::strcasecmp(ext, "bmp"))
		img.load_bmp(file);
	else if (!cimg::strcasecmp(ext, "jpg") ||
		 !cimg::strcasecmp(ext, "jpeg") ||
		 !cimg::strcasecmp(ext, "jpe") ||
		 !cimg::strcasecmp(ext, "jfif") ||
		 !cimg::strcasecmp(ext, "jif"))
		img.load_jpeg(file);
	else if (!cimg::strcasecmp(ext, "png"))
		img.load_png(file);
	else if (!cimg::strcasecmp(ext, "ppm") ||
		 !cimg::strcasecmp(ext, "pgm") ||
		 !cimg::strcasecmp(ext, "pnm") ||
		 !cimg::strcasecmp(ext, "pbm") ||
		 !cimg::strcasecmp(ext, "pnk"))
		img.load_pnm(file);
	else if (!cimg::strcasecmp(ext, "pfm"))
		img.load_pfm(file);

	// 3d binary formats
	else if (!cimg::strcasecmp(ext, "hdr") ||
		 !cimg::strcasecmp(ext, "nii"))
		img.load_analyze(file);
	else if (!cimg::strcasecmp(ext, "inr"))
		img.load_inr(file);
	else if (!cimg::strcasecmp(ext, "pan"))
		img.load_pandore(file);
	else if (!cimg::strcasecmp(ext, "cimg") ||
		 !cimg::strcasecmp(ext, "cimgz") ||
		 !*ext)
		img.load_cimg(file);
	else
		throw CImgIOException("CImg<%s>::load()", img.pixel_type());
	cimg::exception_mode() = 0;
}
} // namespace cimg_library

namespace LSL
{

UnitsyncImage::UnitsyncImage(int width, int height)
    : m_data_ptr(NewImagePtr(width, height))
{
}

UnitsyncImage::UnitsyncImage(const std::string& filename)
    : m_data_ptr(NewImagePtr(1, 1))
{
	Load(filename);
}

UnitsyncImage::UnitsyncImage(PrivateImageType* ptr)
{
	m_data_ptr = new PrivateImageType(*ptr);
}

UnitsyncImage::UnitsyncImage(const UnitsyncImage& other)
{
	m_data_ptr = new PrivateImageType(*other.m_data_ptr);
}


UnitsyncImage::~UnitsyncImage()
{
	if (m_data_ptr != nullptr) {
		delete m_data_ptr;
		m_data_ptr = nullptr;
	}
}

UnitsyncImage::PrivateImageType* UnitsyncImage::NewImagePtr(int width, int height)
{
	try {
		return new PrivateImageType(width, height, 1, 3);
	} catch (std::exception& e) {
		LslError("%s:%d (%s) alloc mem for %dx%d image failed: %s", __FILE__, __LINE__, __FUNCTION__, width, height, e.what());
	}
	return NULL;
}

UnitsyncImage UnitsyncImage::FromMetalmapData(const Util::uninitialized_array<unsigned char>& data, int width, int height)
{
	UnitsyncImage newimg(width, height);
	PrivateImageType& img = *newimg.m_data_ptr;
	cimg_forXY(img, x, y)
	{
		img(x, y, 0, 0) = 0;
		img(x, y, 0, 1) = data[x + (y * width)];
		img(x, y, 0, 2) = 0;
	}
	return newimg;
}

UnitsyncImage UnitsyncImage::FromVfsFileData(Util::uninitialized_array<char>& data, size_t size,
					     const std::string& fn, bool useWhiteAsTransparent)
{
	UnitsyncImage newimg(1, 1);
	PrivateImageType& img = *newimg.m_data_ptr;
	try {
		cimg_library::load_mem(data, size, fn, img);
	} catch (std::exception& e) {
		LslError("%s:%d (%s) %s failed: %s", __FILE__, __LINE__, __FUNCTION__, fn.c_str(), e.what());
	}
	if (useWhiteAsTransparent) {
		newimg.MakeTransparent();
	}
	return newimg;
}

UnitsyncImage::UnitsyncImage()
    : m_data_ptr(NewImagePtr(1, 1))
{
}

bool UnitsyncImage::Save(const std::string& path) const
{
	if (!isValid()) {
		LslError("%s:%d (%s) %s failed, invalid image", __FILE__, __LINE__, __FUNCTION__, path.c_str());
		return false;
	}
	FILE* f = Util::lslopen(path, "wb+");
	if (f == NULL) {
		LslError("%s:%d (%s) error creating file %s", __FILE__, __LINE__, __FUNCTION__, path.c_str());
		return false;
	}
	m_data_ptr->save_png(f);
	fclose(f);
	return true;
}

bool UnitsyncImage::Load(const std::string& path) const
{
	try {
		FILE* f = Util::lslopen(path, "rb");
		if (f == NULL) {
			LslError("%s:%d (%s) could not open file %s", __FILE__, __LINE__, __FUNCTION__, path.c_str());
			return false;
		}
		m_data_ptr->load_png(f);
		fclose(f);
	} catch (cimg_library::CImgIOException& c) {
		m_data_ptr->clear();
		LslWarning("%s:%d (%s) %s failed: %s", __FILE__, __LINE__, __FUNCTION__, path.c_str(), c.what());
		return false;
	} catch (cimg_library::CImgException& c) {
		m_data_ptr->clear();
		LslWarning("%s:%d (%s) %s failed: %s", __FILE__, __LINE__, __FUNCTION__, path.c_str(), c.what());
		return false;
	}
	return true;
}

UnitsyncImage UnitsyncImage::FromMinimapData(const UnitsyncImage::RawDataType* colors, int width, int height)
{
	UnitsyncImage newimg(width, height);
	PrivateImageType& img = *newimg.m_data_ptr;
	cimg_forXY(img, x, y)
	{
		int at = x + (y * width);
		img(x, y, 0, 0) = (unsigned char)(((colors[at] >> 11) / 31.0) * 255.0);
		img(x, y, 0, 1) = (unsigned char)((((colors[at] >> 5) & 63) / 63.0) * 255.0);
		img(x, y, 0, 2) = (unsigned char)(((colors[at] & 31) / 31.0) * 255.0);
	}
	return newimg;
}

UnitsyncImage UnitsyncImage::FromHeightmapData(const Util::uninitialized_array<unsigned short>& grayscale, int width, int height)
{
	assert(width > 0);
	assert(height > 0);

	UnitsyncImage newimg(width, height);
	PrivateImageType& img = *newimg.m_data_ptr;
	// the height is mapped to this "palette" of colors
	// the colors are linearly interpolated
	const unsigned char points[][3] = {
	    {0, 0, 0},
	    {0, 0, 255},
	    {0, 255, 255},
	    {0, 255, 0},
	    {255, 255, 0},
	    {255, 0, 0},
	};
	const int numPoints = sizeof(points) / sizeof(points[0]);

	// find range of values present in the height data returned by unitsync
	int min = 65536;
	int max = 0;

	for (int i = 0; i < width * height; i++) {
		if (grayscale[i] < min)
			min = grayscale[i];
		if (grayscale[i] > max)
			max = grayscale[i];
	}

	// prevent division by zero -- heightmap wouldn't contain any information anyway
	if (min == max) {
		return UnitsyncImage(1, 1);
	}

	// perform the mapping From 16 bit grayscale to 24 bit true color
	const double range = max - min + 1;
	//	for ( int i = 0; i < width*height; i++ ) {
	cimg_forXY(img, x, y)
	{
		const int i = x + (y * width);
		const double value = (grayscale[i] - min) / (range / (numPoints - 1));
		const int idx1 = int(value);
		const int idx2 = idx1 + 1;
		const int t = int(256.0 * (value - std::floor(value)));

		//assert(idx1 >= 0 && idx1 < numPoints-1);
		//assert(idx2 >= 1 && idx2 < numPoints);
		//assert(t >= 0 && t <= 255);
		cimg_forC(img, j)
		{
			img(x, y, 0, j) = (points[idx1][j] * (255 - t) + points[idx2][j] * t) / 255;
		}
	}

	return newimg;
}

int UnitsyncImage::GetHeight() const
{
	return m_data_ptr->height();
}

void UnitsyncImage::Rescale(const int new_width, const int new_height)
{
	if (!isValid()) {
		LslError("%s:%d (%s) %s failed, invalid image", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	if ((GetWidth() == new_width) && (GetHeight() == new_height))
		return; //no size change
	m_data_ptr->resize(new_width, new_height, 1 /*z*/, 3 /*c*/, 5 /*interpolation type*/);
}

void UnitsyncImage::MakeTransparent(unsigned short r, unsigned short g, unsigned short b)
{
	if (!isValid()) {
		LslError("%s:%d (%s) %s failed, invalid image", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	if (m_data_ptr->spectrum() == 4) { //image has already alpha channel
		return;
	}

	m_data_ptr->channels(0, 3); //add 4th channel
	cimg_forXY(*m_data_ptr, x, y)
	{
		if ((*m_data_ptr->data(x, y, 0, 0) == r) && (*m_data_ptr->data(x, y, 0, 1) == g) && (*m_data_ptr->data(x, y, 0, 2) == b)) { //pixel is white, make transparent
			*m_data_ptr->data(x, y, 0, 3) = 0;
		} else {
			*m_data_ptr->data(x, y, 0, 3) = 255;
		}
	}
}

int UnitsyncImage::GetWidth() const
{
	return m_data_ptr->width();
}

void UnitsyncImage::RescaleIfBigger(const int maxwidth, const int maxheight)
{
	if (!isValid())
		return;

	int height = GetHeight();
	int width = GetWidth();
	bool rescale = false;
	if (height > maxheight) {
		width = (float(maxheight) / height) * width;
		height = maxheight;
		rescale = true;
	}
	if (width > maxwidth) {
		height = (float(maxwidth) / width) * height;
		width = maxwidth;
		rescale = true;
	}
	if (rescale) {
		Rescale(width, height);
	}
}

UnitsyncImage& UnitsyncImage::operator=(const UnitsyncImage& other)
{
	if (this != &other) {
		if (isValid()) {
			delete m_data_ptr;
		}
		m_data_ptr = new PrivateImageType(*other.m_data_ptr);
	}
	return *this;
}


#ifdef HAVE_WX
wxBitmap UnitsyncImage::wxbitmap() const
{
	return wxBitmap(wximage());
}

wxImage UnitsyncImage::wximage() const
{
	if ((m_data_ptr == 0) || (m_data_ptr->width() <= 0) || (m_data_ptr->height() <= 0)) { //return empty image if m_data_ptr isn't initialized/valid
		return wxImage(1, 1);
	}
	wxImage img(m_data_ptr->width(), m_data_ptr->height());
	const auto ptr = *m_data_ptr;
	cimg_forXY(ptr, x, y)
	{
		img.SetRGB(x, y, ptr(x, y, 0, 0), ptr(x, y, 0, 1), ptr(x, y, 0, 2));
	}
	if (m_data_ptr->spectrum() == 4) {
		img.InitAlpha();
		cimg_forXY(ptr, x, y)
		{
			img.SetAlpha(x, y, ptr(x, y, 0, 3));
		}
	}
	return img;
}
#endif

} // namespace LSL

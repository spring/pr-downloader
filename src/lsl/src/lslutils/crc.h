/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#ifndef LSL_CRC_H
#define LSL_CRC_H

#include <string>

//namespace LSL {

/** @brief Object representing an updateable CRC-32 checksum. */
class CRC
{
public:
	CRC();

	void UpdateData(const unsigned char* buf, unsigned bytes);
	void UpdateData(const std::string& buf);
	bool UpdateFile(const std::string& filename);

	void ResetCRC();

	unsigned int GetCRC() const
	{
		return crc ^ 0xFFFFFFFF;
	}

private:
	static unsigned int crcTable[256];
	static void GenerateCRCTable();

	unsigned int crc;
};

//} // namespace LSL

#endif // !LSL_CRC_H

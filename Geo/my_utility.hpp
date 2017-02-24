#pragma once

// #include <windows.h>
#include <cstdint>
#include <fstream>

namespace kato
{
	
static void writeBMP(char const *const filename, std::uint32_t const width, std::uint32_t const height, void const *ptr)
{
	// HANDLE hFile = CreateFileA(filename, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	std::ofstream ofs(filename, std::ios::binary);
	// if(hFile == INVALID_HANDLE_VALUE) return;
	if(!ofs) return;
	std::uint32_t const pixel_num = width * height;
#define UINT16TO2BYTE(NUM) (std::uint16_t)(NUM) & 0xFF, ((std::uint16_t)(NUM) >> 8) & 0xFF
#define UINT32TO4BYTE(NUM) (std::uint32_t)(NUM) & 0xFF, ((std::uint32_t)(NUM) >> 8) & 0xFF, ((std::uint32_t)(NUM) >> 16) & 0xFF, ((std::uint32_t)(NUM) >> 24) & 0xFF
	std::uint8_t const padsize = (4 - (width * 3) % 4) % 4;
	std::uint8_t const header[14] =
	{
		'B', 'M',
		UINT32TO4BYTE( 14 + 40 + width * height * 3 + padsize * height ),
		UINT16TO2BYTE(0),
		UINT16TO2BYTE(0),
		UINT32TO4BYTE(14 + 40)
	};
	// DWORD written;
	// WriteFile(hFile, header, sizeof(header), &written, nullptr);
	ofs.write(reinterpret_cast<char const *>(header), sizeof(header));
	std::uint8_t const infoheader[40] =
	{
		UINT32TO4BYTE(40),
		UINT32TO4BYTE(width),
		UINT32TO4BYTE(height),
		UINT16TO2BYTE(1),
		UINT16TO2BYTE(24),
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
		0, 0, 0, 0,
	};
	// WriteFile(hFile, infoheader, sizeof(infoheader), &written, nullptr);
	ofs.write(reinterpret_cast<char const *>(infoheader), sizeof(infoheader));
	constexpr std::uint8_t zeros[ 3 ]{};
	if( padsize == 0 )
	{
		// WriteFile(hFile, ptr, pixel_num * 3, &written, nullptr);
	ofs.write(reinterpret_cast<char const *>(ptr), pixel_num * 3);
	}
	else
	{
		for( std::uint32_t i = 0u; i < height; ++i )
		{
			// WriteFile( hFile, (std::uint8_t *)ptr + width * i, width * 3, &written, nullptr);
	        ofs.write( reinterpret_cast<char const *>((std::uint8_t *)ptr + width * i), width * 3);
			// WriteFile( hFile, zeros, padsize, &written, nullptr);
	        ofs.write( reinterpret_cast<char const *>(zeros), padsize);
		}
	}
	// CloseHandle(hFile);
#undef UINT16TO2BYTE
#undef UINT32TO4BYTE
}

}

#include "stdafx.h"
#include "MurmurHash3.h"

DWORD GenerateHash(const char *buf)
{
	DWORD dw;
	MurmurHash3_x86_32(buf, strlen(buf), 0x88664422, &dw);
	return dw;
}

DWORD GenerateHash(const char *buf, size_t size)
{
	DWORD dw;
	MurmurHash3_x86_32(buf, size, 0x88664422, &dw);
	return dw;
}
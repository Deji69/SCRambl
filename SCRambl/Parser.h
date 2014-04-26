#pragma once

namespace SCRambl
{
	int IsDirectivePrefix(int);
	int IsIdentifierStart(int);
	int IsIdentifier(int);
	int IsSpace(int);
	int BothAreSpaces(int, int);
	int IsSeparator(int);
	int BothAreSeparators(int, int);
	int IsUselessSeparator(int, int);
}
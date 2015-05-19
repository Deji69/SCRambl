#pragma once
#include "..\SCRambl.h"

struct SCRamblInstance
{
	std::vector<std::string> InputFiles;
	SCRambl::Engine Engine;
};
#pragma once
#include "..\SCRambl.h"

struct SCRamblInstance
{
	std::vector<std::string> InputFiles;
	SCRambl::Build* Build = nullptr;
	SCRambl::Engine Engine;
};
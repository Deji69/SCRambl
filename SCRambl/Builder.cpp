#include "stdafx.h"
#include "Builder.h"

using namespace SCRambl;

void Builder::Run()
{
	switch (m_State)
	{
	case init:
		Init();
		break;
	case preprocess:
	case parse:
	case compile:
	case link:
		break;
	default:
		break;
	}
}

void Builder::Init()
{

}
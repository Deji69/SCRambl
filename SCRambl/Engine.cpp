#include "stdafx.h"
#include "Engine.h"

using namespace SCRambl;

Engine::Engine()
{
}

Task & Engine::Run()
{
	m_pBuild->Run();
	return *this;
}
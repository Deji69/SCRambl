#include "stdafx.h"
#include "Engine.h"

using namespace SCRambl;

Engine::Engine()
{
}

RunningState Engine::Run()
{
	m_pBuild->Run();
}
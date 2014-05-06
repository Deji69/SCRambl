#include "stdafx.h"
#include "Builder.h"

using namespace SCRambl;

void Builder::RunTask(RunningState & running)
{
	switch (running.State)
	{
	case RunningState::finished:

		break;
	}
	switch (State)
	{
	case init:
		Init()
		break;
	}
}

void Builder::Init()
{

}
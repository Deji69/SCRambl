#include "stdafx.h"
#include "Builder.h"

using namespace SCRambl;

Task & Builder::RunTask()
{
	TaskState() = Task::running;

	try
	{
		switch (GetState())
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
			TaskState() = Task::finished;
			break;
		}
	}
	catch (...)
	{
		TaskState() = Task::error;
	}
	
	return *this;
}

void Builder::Init()
{

}
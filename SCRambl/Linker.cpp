#include "stdafx.h"
#include "Linker.h"
#include "Builder.h"

using namespace SCRambl;
using namespace SCRambl::Linking;

void Linker::Link() {
			
}
void Linker::Init() {
	//auto scriptName = m_Build->CreateOutputFile();
			
}
void Linker::Reset() {
}
void Linker::Run() {
	switch (m_State) {
	case init:
		Init();
	case linking:
		Link();
		break;
	}
}
Linker::Linker(Task& task, Engine& engine, Build* build) : m_Engine(engine), m_Task(task), m_Build(build)
{ }
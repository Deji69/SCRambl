#include "stdafx.h"
#include "Linker.h"
#include "Builder.h"

namespace SCRambl
{
	namespace Linker
	{
		void Linker::Link() {
			
		}
		void Linker::Init() {

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
		Linker::Linker(Task& task, Engine& engine, Build::Shared build) : m_Engine(engine), m_Task(task), m_Build(build)
		{ }
	}
}
#pragma once

namespace SCRambl
{
	enum eBuildTask
	{
		BUILD_TASK_PRINT,
		BUILD_TASK_MKDIR,
	};

	class CBuildSystem
	{
		void echo(const char * msg);
		void copy(const char * src, const char * dst);
		void mkdir(const char * dir);
	};
}
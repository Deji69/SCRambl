#include "stdafx.h"
#include "SCRambl.h"
#include "..\SCRambl.h"
#include "SCRambl\XML.h"
#include "Instance.h"

SCRambl::Engine g_Engine;
std::vector<std::string> g_InputFiles;

SCRamblStatus MakeStatus(SCRamblResultCode rc) {
	SCRamblStatus status;
	status.RC = rc;
	return status;
}

SCRAMBLAPI bool SCRambl_Init(SCRamblInst** out) {
	if (auto inst = new SCRamblInst) {
		if (inst->Inst = new SCRamblInstance()) {
			inst->Status = MakeStatus(SCRAMBLRC_OK);
			*out = inst;
			return true;
		}
		else delete inst;
	}
	return false;
}
SCRAMBLAPI bool SCRambl_Free(SCRamblInst** inst) {
	if (inst) {
		if (*inst) {
			if ((*inst)->Inst) {
				delete (*inst)->Inst;
				(*inst)->Inst = nullptr;
			}
			delete *inst;
		}
		*inst = nullptr;
		return true;
	}
	return false;
}
SCRAMBLAPI bool SCRambl_AddInputFile(SCRamblInst* instance, const char* path) {
	instance->Inst->InputFiles.emplace_back(path);
	return true;
}
SCRAMBLAPI void SCRambl_ClearInputFiles(SCRamblInst* instance) {
	instance->Inst->InputFiles.clear();
}
SCRAMBLAPI bool SCRambl_LoadBuildConfig(SCRamblInst* inst, const char* file, const char* config) {
	if (!inst->Inst->Engine.LoadBuildFile(file, config)) {
		inst->Status = MakeStatus(SCRAMBLRC_BUILD_FILE_NOT_FOUND);
		return false;
	}
	inst->Status = MakeStatus(SCRAMBLRC_OK);
	return true;
}
SCRAMBLAPI bool SCRambl_Build(SCRamblInst* inst) {
	auto& engine = inst->Inst->Engine;
	auto build = engine.InitBuild(inst->Inst->InputFiles);
	while (true) {
		engine.BuildScript(build);
	}
	engine.FreeBuild(build);
	return true;
}
#include "stdafx.h"
#include "States.h"

using namespace SCRambl;

const BuilderState::State BuilderState::state_branch[] = {
	preprocess, parse, link, compile, max_state
};
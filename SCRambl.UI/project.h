#ifndef PROJECT_H
#define PROJECT_H
#include "SCRambl.h"

class Project
{
	SCRambl::Script				m_Script;

public:
	Project();


};

extern Project *			CurrentProject;
extern QVector<Project *>	Projects;

#endif
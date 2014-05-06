cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

SET(ProjectName SCRambl)
SET(Files 
	stdafx.h 
	stdafx.cpp
        Builder.cpp
	Directives.cpp
	Engine.cpp
	Exception.cpp
	FileSystem.cpp
	Identifiers.cpp
	Literals.cpp
	Macros.cpp
	Parser.cpp
	Preprocessor.cpp
	ProjectManager.cpp
	Reporting.cpp
	Scripts.cpp
	States.cpp
	stdafx.cpp
)

include_directories("../")

add_executable(${ProjectName} ${Files})
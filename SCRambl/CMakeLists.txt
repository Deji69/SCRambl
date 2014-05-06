cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

SET(CMAKE_LEGACY_CYGWIN_WIN32 0)

SET(ProjectName SCRambl)
SET(Files 
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
)

#SET(CXX_FLAGS "")
IF(CMAKE_COMPILER_IS_GNUCXX)
    SET(CXX_FLAGS "${CXX_FLAGS} -std=gnu++11")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_FLAGS}")
#SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${CXX_FLAGS}")

#include_directories("../")

add_definitions(${CXX_FLAGS})
add_executable(${ProjectName} ${Files})
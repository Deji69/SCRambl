CXX=g++
CXXFLAGS=-std=gnu++11
OBJS=stdafx.o

scrambl : $(OBJS)
		$(CXX) $(CXXFLAGS) -o scrambl $(OBJS)

stdafx.o : stdafx.h
#main.o : main.h ../SCRambl.h

.PHONY : clean
clean : -rm stdafx $(OBJS)
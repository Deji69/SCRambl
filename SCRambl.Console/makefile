CXX=g++
CXXFLAGS=-I=./
OBJS=main.o

scrambl : $(OBJS)
		CXX $(CXXFLAGS) -o scrambl $(OBJS)
          
main.o : SCRambl.h main.h

.PHONY : clean
clean : -rm main $(OBJS)
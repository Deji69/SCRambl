CXX=g++
CXXFLAGS=-I../ -std=c++11 -Dscrambl
OBJS=main.o

scrambl : $(OBJS)
		$(CXX) $(CXXFLAGS) -o scrambl $(OBJS)
          
main.o : main.h ../SCRambl.h

.PHONY : clean
clean : -rm main $(OBJS)
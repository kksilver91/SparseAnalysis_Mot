CC = g++
CXXFLAGS=-DNO_STORAGE -Wall -DDEBUG_BUILD -std=c++11 -pthread
MKLROOT=/usr/local/mklml/mklml_lnx_2018.0.3.20180406
# CXXFLAGS=-DNO_STORAGE -Wall -DDEBUG_BUILD -std=c++11 -pthread -fopenmp -m64 -I${MKLROOT}/include -Wl,--no-as-needed -L${MKLROOT}/lib/intel64 -lmkl_intel_lp64 -lmkl_gnu_thread -lmkl_core -lgomp -lpthread -lm -ldl
OPTFLAGS=-O3 
OBJS = main.o analysis.o

ifdef DEBUG
ifeq ($(DEBUG), 1)
OPTFLAGS= -O0 -g
endif
endif
CXXFLAGS+=$(OPTFLAGS)


%.o: %.cc %.h
	$(CC) $(CXXFLAGS) -c $<

main : $(OBJS)
	$(CC) $(CXXFLAGS) $(OBJS) -o SPARSE_ANALYSIS

.PHONY: clean
clean:
	rm -f $(OBJS) main

%.o: %.cc %.h
	$(CC) $(CXXFLAGS) -c $<



CC = mpic++

#Choose MPI Compiler based on architecture
ifeq ($(shell uname -m), ppc64le)   #AiMOS architecture
	FLAGS = -std=c++2a -O3				
else								#Debug when not on AiMOS
	FLAGS = -std=c++2a -O3 -Wall -g
endif

SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(SOURCES:src/%.cpp=bin/%.opp)

parallel: $(filter-out bin/SerialVerifier.opp, $(OBJECTS))
	$(CC) $(FLAGS) -o bin/parallelVerif.exe $^

serial: $(filter-out bin/ParallelVerifier.opp, $(OBJECTS))
	$(CC) $(FLAGS) -o bin/serialVerif.exe $^

bin/%.opp : src/%.cpp src/%.hpp
	$(CC) $(FLAGS) -c -o $@ $<

bin/%.opp : src/%.cpp
	$(CC) $(FLAGS) -c -o $@ $<

clean:
	rm -f bin/*.opp
	rm -f *.exe

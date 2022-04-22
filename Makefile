
CC = mpic++

FLAGS = -g

SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(SOURCES:src/%.cpp=bin/%.opp)

serial: $(filter-out bin/ParallelVerifier.opp, $(OBJECTS))
	$(CC) $(FLAGS) -o serialVerif.exe $^

parallel: $(filter-out bin/SerialVerifier.opp, $(OBJECTS))
	$(CC) $(FLAGS) -o parallelVerif.exe $^

bin/%.opp : src/%.cpp src/%.hpp makeBin
	$(CC) $(FLAGS) -c -o $@ $<

bin/%.opp : src/%.cpp makeBin
	$(CC) $(FLAGS) -c -o $@ $<

makeBin:
	mkdir -p bin

clean:
	rm -f bin/*.opp
	rm -f bin/*.exe
	rm -f *.exe
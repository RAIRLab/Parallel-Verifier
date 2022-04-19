
FLAGS = -g

SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(SOURCES:src/%.cpp=bin/%.opp)

all: $(OBJECTS)
	g++ $(FLAGS) -o bin/verif.exe $^

bin/%.opp : src/%.cpp src/%.hpp makeBin
	g++ $(FLAGS) -c -o $@ $<

bin/%.opp : src/%.cpp makeBin
	g++ $(FLAGS) -c -o $@ $<

makeBin:
	mkdir -p bin

clean:
	rm bin/*.opp
	rm bin/*.exe
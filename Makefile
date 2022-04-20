
CC = mpic++

FLAGS = -g

SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(SOURCES:src/%.cpp=bin/%.opp)

all: $(OBJECTS)
	$(CC) $(FLAGS) -o verif.exe $^

bin/%.opp : src/%.cpp src/%.hpp makeBin
	$(CC) $(FLAGS) -c -o $@ $<

bin/%.opp : src/%.cpp makeBin
	$(CC) $(FLAGS) -c -o $@ $<

makeBin:
	mkdir -p bin

clean:
	rm bin/*.opp
	rm bin/*.exe
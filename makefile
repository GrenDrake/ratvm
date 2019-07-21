CXXFLAGS= -std=c++11 -g -Wall

BUILD_OBJS=build/build.o build/general.o build/lexer.o \
		   build/parse_main.o build/translate.o build/gamedata.o \
		   build/value.o build/parse_functions.o build/parsestate.o \
		   build/generate.o build/bytestream.o build/dump.o build/opcode.o \
		   build/expression.o
BUILD=./gbuild

RUNNER_OBJS=runner/runner.o runner/gameloop.o runner/gamedata.o \
			runner/formatter.o runner/runfunction.o runner/stack.o \
			runner/loadgame.o runner/dump.o  \
			runner/bytestream.o runner/value.o
RUNNER=./grun

FILESCAN_OBJS=build/filescan.o build/value.o build/bytestream.o
FILESCAN=./filescan

TEST_BYTESTREAM_OBJS=tests/bytestream.o build/bytestream.o
TEST_BYTESTREAM=./test_bytestream

TESTSRC=examples/tests.src

all: $(BUILD) $(FILESCAN) $(RUNNER) tests game.bin

tests: $(TEST_BYTESTREAM)

$(BUILD): $(BUILD_OBJS)
	$(CXX) $(BUILD_OBJS) -o $(BUILD)

$(FILESCAN): $(FILESCAN_OBJS)
	$(CXX) $(FILESCAN_OBJS) -o $(FILESCAN)

$(RUNNER): $(RUNNER_OBJS)
	$(CXX) $(RUNNER_OBJS) -o $(RUNNER)

$(TEST_BYTESTREAM): $(BUILD) $(TEST_BYTESTREAM_OBJS)
	$(CXX) $(TEST_BYTESTREAM_OBJS) -o $(TEST_BYTESTREAM)
	$(TEST_BYTESTREAM)

game.bin: $(BUILD) $(TESTSRC)
	$(BUILD) -data -functions -bytecode -asm -ir $(TESTSRC)
	cp game.bin ../gtrpge-javascript/


clean:
	$(RM) build/*.o runner/*.o tests/*.o
	$(RM) $(BUILD) $(FILESCAN) $(RUNNER) $(TEST_BYTESTREAM)

.PHONY: all clean tests
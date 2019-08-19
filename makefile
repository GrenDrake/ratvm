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

TEST_BYTESTREAM_OBJS=tests/bytestream.o build/bytestream.o
TEST_BYTESTREAM=./test_bytestream

AUTOTESTS_SRC=examples/auto_tests.qc
AUTOTESTS=./examples/auto_tests.qvm
USERTESTS_SRC=examples/user_tests.qc
USERTESTS=./examples/user_tests.qvm
FIBTEST_SRC=examples/fibonacci.qc
FIBTEST=./examples/fibonacci.qvm

all: $(BUILD) $(RUNNER) tests examples

tests: $(TEST_BYTESTREAM)

$(BUILD): $(BUILD_OBJS)
	$(CXX) $(BUILD_OBJS) -o $(BUILD)

$(RUNNER): $(RUNNER_OBJS)
	$(CXX) $(RUNNER_OBJS) -o $(RUNNER)

$(TEST_BYTESTREAM): $(BUILD) $(TEST_BYTESTREAM_OBJS)
	$(CXX) $(TEST_BYTESTREAM_OBJS) -o $(TEST_BYTESTREAM)
	$(TEST_BYTESTREAM)

examples: $(AUTOTESTS) $(USERTESTS) $(FIBTEST)
	cp ./examples/*.qvm ../playquoll/games/
$(AUTOTESTS): $(BUILD) $(AUTOTESTS_SRC)
	$(BUILD) $(AUTOTESTS_SRC) -o $(AUTOTESTS)
	cp $(AUTOTESTS) ../gtrpge-javascript/games/
$(USERTESTS): $(BUILD) $(USERTESTS_SRC)
	$(BUILD) $(USERTESTS_SRC) -o $(USERTESTS)
$(FIBTEST): $(BUILD) $(FIBTEST_SRC)
	$(BUILD) $(FIBTEST_SRC) -o $(FIBTEST)


clean: clean_runner
	$(RM) build/*.o runner/*.o tests/*.o
	$(RM) $(BUILD) $(TEST_BYTESTREAM)

clean_runner:
	$(RM) runner/*.o $(RUNNER)

.PHONY: all clean clean_runner tests examples

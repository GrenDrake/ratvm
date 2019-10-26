PLAYQUOLL=../playquoll/
CXXFLAGS= -std=c++11 -g -Wall

BUILD_OBJS=builder/build.o builder/general.o builder/lexer.o \
		   builder/parse_main.o builder/translate.o builder/gamedata.o \
		   builder/value.o builder/parse_functions.o builder/parsestate.o \
		   builder/generate.o builder/bytestream.o builder/dump.o \
		   builder/opcode.o builder/expression.o
BUILD=./build

RUNNER_OBJS=runner/runner.o runner/gameloop.o runner/gamedata.o \
			runner/formatter.o runner/runfunction.o runner/stack.o \
			runner/loadgame.o runner/dump.o  \
			runner/bytestream.o runner/value.o
RUNNER=./run

TEST_BYTESTREAM_OBJS=tests/bytestream.o builder/bytestream.o
TEST_BYTESTREAM=./test_bytestream
TEST_GENERAL_OBJS=tests/general.o builder/general.o
TEST_GENERAL=./test_general

AUTOTESTS_SRC=examples/auto_tests.qc
AUTOTESTS=./examples/auto_tests.qvm
USERTESTS_SRC=examples/user_tests.qc
USERTESTS=./examples/user_tests.qvm
FIBTEST_SRC=examples/fibonacci.qc
FIBTEST=./examples/fibonacci.qvm

all: $(BUILD) $(RUNNER) tests examples

tests: $(TEST_BYTESTREAM) $(TEST_GENERAL)

$(BUILD): $(BUILD_OBJS)
	$(CXX) $(BUILD_OBJS) -o $(BUILD)

$(RUNNER): $(RUNNER_OBJS)
	$(CXX) $(RUNNER_OBJS) -o $(RUNNER)

$(TEST_BYTESTREAM): $(BUILD) $(TEST_BYTESTREAM_OBJS)
	$(CXX) $(TEST_BYTESTREAM_OBJS) -o $(TEST_BYTESTREAM)
	$(TEST_BYTESTREAM)

$(TEST_GENERAL): $(BUILD) $(TEST_GENERAL_OBJS)
	$(CXX) $(TEST_GENERAL_OBJS) -o $(TEST_GENERAL)
	$(TEST_GENERAL)

examples: $(BUILD) $(AUTOTESTS) $(USERTESTS) $(FIBTEST)
	cp ./examples/*.qvm $(PLAYQUOLL)games/
$(AUTOTESTS): $(BUILD) $(AUTOTESTS_SRC)
	$(BUILD) $(AUTOTESTS_SRC) -o $(AUTOTESTS)
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

PLAYQUOLL=../playquoll/
CFLAGS= -std=c99 -g -Wall
CXXFLAGS= -std=c++11 -g -Wall -I../utf8proc-2.4.0/

UTF8PROC_LIB=-L../utf8proc-2.4.0/ -lutf8proc

BUILD_OBJS=builder/build.o builder/general.o builder/lexer.o \
		   builder/parse_main.o builder/translate.o builder/gamedata.o \
		   builder/value.o builder/parse_functions.o builder/parsestate.o \
		   builder/generate.o builder/bytestream.o builder/dump.o \
		   builder/opcode.o builder/expression.o builder/textutil.o
BUILD=./build

RUNNER_OBJS=runner/runner.o runner/gameloop.o runner/gamedata.o \
			runner/formatter.o runner/runfunction.o runner/stack.o \
			runner/loadgame.o runner/dump.o runner/fileio.o \
			runner/bytestream.o runner/value.o runner/textutil.o
RUNNER=./run

TEST_BYTESTREAM_OBJS=tests/bytestream.o builder/bytestream.o
TEST_BYTESTREAM=./test_bytestream
TEST_TEXTUTIL_OBJS=tests/textutil.o builder/textutil.o builder/general.o
TEST_TEXTUTIL=./test_textutil
TEST_FIBONACCI_OBJS=tests/fibonacci.o
TEST_FIBONACCI=./test_fibonacci

AUTOTESTS_SRC=examples/auto_tests.qc
AUTOTESTS=./examples/auto_tests.qvm
STACKTEST_SRC=./examples/syntax_stack.qc
STACKTEST=./examples/syntax_stack.qvm
USERTESTS_SRC=examples/user_tests.qc
USERTESTS=./examples/user_tests.qvm
FIBTEST_SRC=examples/fibonacci.qc
FIBTEST=./examples/fibonacci.qvm

all: $(BUILD) $(RUNNER) tests examples

tests: $(TEST_BYTESTREAM) $(TEST_TEXTUTIL) $(TEST_FIBONACCI)

$(BUILD): $(BUILD_OBJS)
	$(CXX) $(BUILD_OBJS) $(UTF8PROC_LIB) -o $(BUILD)

$(RUNNER): $(RUNNER_OBJS)
	$(CXX) $(RUNNER_OBJS) $(UTF8PROC_LIB) -lsqlite3 -o $(RUNNER)

$(TEST_BYTESTREAM): $(BUILD) $(TEST_BYTESTREAM_OBJS)
	$(CXX) $(TEST_BYTESTREAM_OBJS) -o $(TEST_BYTESTREAM)
	$(TEST_BYTESTREAM)

$(TEST_TEXTUTIL): $(BUILD) $(TEST_TEXTUTIL_OBJS)
	$(CXX) $(TEST_TEXTUTIL_OBJS) $(UTF8PROC_LIB) -o $(TEST_TEXTUTIL)
	$(TEST_TEXTUTIL)
$(TEST_FIBONACCI): $(BUILD) $(TEST_FIBONACCI_OBJS)
	$(CC) $(TEST_FIBONACCI_OBJS) -o $(TEST_FIBONACCI)

examples: $(BUILD) $(AUTOTESTS) $(STACKTEST) $(USERTESTS) $(FIBTEST)
	cp ./examples/*.qvm $(PLAYQUOLL)games/
$(AUTOTESTS): $(BUILD) $(AUTOTESTS_SRC)
	$(BUILD) $(AUTOTESTS_SRC) -o $(AUTOTESTS)
	$(RUNNER) $(AUTOTESTS) -silent
$(STACKTEST): $(BUILD) $(STACKTEST_SRC)
	$(BUILD) $(STACKTEST_SRC) -o $(STACKTEST)
	$(RUNNER) $(STACKTEST) -silent
$(USERTESTS): $(BUILD) $(USERTESTS_SRC)
	$(BUILD) $(USERTESTS_SRC) -o $(USERTESTS)
$(FIBTEST): $(BUILD) $(FIBTEST_SRC)
	$(BUILD) $(FIBTEST_SRC) -o $(FIBTEST)


clean: clean_runner
	$(RM) builder/*.o runner/*.o tests/*.o
	$(RM) $(BUILD) $(TEST_BYTESTREAM) $(TEST_TEXTUTIL) $(TEST_FIBONACCI)

clean_runner:
	$(RM) runner/*.o $(RUNNER)

.PHONY: all clean clean_runner tests examples

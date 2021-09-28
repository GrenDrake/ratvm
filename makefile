CC=gcc
PLAYQUOLL=./playrat/
CFLAGS= -std=c99 -g -Wall
CXXFLAGS= -std=c++11 -g -Wall -I../utf8proc/ -I./common/ -DUTF8PROC_STATIC

UTF8PROC_LIB=-L../utf8proc/ -lutf8proc

BUILD_OBJS=builder/build.o builder/general.o builder/lexer.o \
		   builder/parse_main.o builder/translate.o builder/gamedata.o \
		   builder/value.o builder/parse_functions.o builder/parsestate.o \
		   builder/generate.o builder/bytestream.o builder/dump.o \
		   builder/opcode.o builder/expression.o common/textutil.o
BUILD=./build

RUNNER_OBJS=runner/runner.o runner/gameloop.o runner/gamedata.o \
			runner/formatter.o runner/runfunction.o runner/stack.o \
			runner/loadgame.o runner/dump.o runner/fileio.o \
			runner/bytestream.o runner/value.o common/textutil.o
RUNNER=./run

TEST_BYTESTREAM_OBJS=tests/bytestream.o builder/bytestream.o
TEST_BYTESTREAM=./test_bytestream
TEST_TEXTUTIL_OBJS=tests/textutil.o common/textutil.o
TEST_TEXTUTIL=./test_textutil
TEST_FIBONACCI_OBJS=tests/fibonacci.o
TEST_FIBONACCI=./test_fibonacci

all: $(BUILD) $(RUNNER) tests examples tests_ratc

tests: $(TEST_BYTESTREAM) $(TEST_TEXTUTIL) $(TEST_FIBONACCI)

$(BUILD): $(BUILD_OBJS)
	$(CXX) $(BUILD_OBJS) $(UTF8PROC_LIB) -o $(BUILD)

$(RUNNER): $(RUNNER_OBJS)
	$(CXX) $(RUNNER_OBJS) $(UTF8PROC_LIB) -o $(RUNNER)

$(TEST_BYTESTREAM): $(BUILD) $(TEST_BYTESTREAM_OBJS)
	$(CXX) $(TEST_BYTESTREAM_OBJS) -o $(TEST_BYTESTREAM)
	$(TEST_BYTESTREAM)

$(TEST_TEXTUTIL): $(BUILD) $(TEST_TEXTUTIL_OBJS)
	$(CXX) $(TEST_TEXTUTIL_OBJS) $(UTF8PROC_LIB) -o $(TEST_TEXTUTIL)
	$(TEST_TEXTUTIL)

$(TEST_FIBONACCI): $(BUILD) $(TEST_FIBONACCI_OBJS)
	$(CC) $(TEST_FIBONACCI_OBJS) -o $(TEST_FIBONACCI)

examples: $(BUILD)
	cd examples && make
	cp ./examples/*.rvm $(PLAYQUOLL)games/
tests_ratc: $(BUILD) $(RUNNER)
	cd tests_ratc && make
	cp ./tests_ratc/*.rvm $(PLAYQUOLL)games/

clean: clean_runner
	$(RM) builder/*.o runner/*.o tests/*.o tests_ratc/*.rvm
	$(RM) $(BUILD) $(TEST_BYTESTREAM) $(TEST_TEXTUTIL) $(TEST_FIBONACCI)

clean_runner:
	$(RM) runner/*.o $(RUNNER)

.PHONY: all clean clean_runner tests examples tests_ratc

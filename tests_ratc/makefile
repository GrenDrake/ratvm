BUILD=../build
RUNNER=../run

TEST_ALL_SRC=test_all.ratc \
			 ./test_values.ratc ./test_stack.ratc ./test_explode.ratc \
			 ./test_math.ratc ./test_maps.ratc ./test_lists.ratc \
			 ./test_objects.ratc ./test_strings.ratc ./test_jumps.ratc \
			 ./test_comparisons.ratc ./test_fileio.ratc ./test_dynamic.ratc \
			 ./test_vocab.ratc ./test_objtree.ratc
TEST_ALL=./test_all.rvm
TEST_VALUES_SRC=test_values.ratc
TEST_VALUES=./test_values.rvm
TEST_STACK_SRC=./test_stack.ratc
TEST_STACK=./test_stack.rvm
TEST_EXPLODE_SRC=./test_explode.ratc
TEST_EXPLODE=./test_explode.rvm
TEST_MATH_SRC=./test_math.ratc
TEST_MATH=./test_math.rvm
TEST_MAPS_SRC=./test_maps.ratc
TEST_MAPS=./test_maps.rvm
TEST_LISTS_SRC=./test_lists.ratc
TEST_LISTS=./test_lists.rvm
TEST_OBJECTS_SRC=./test_objects.ratc
TEST_OBJECTS=./test_objects.rvm
TEST_STRINGS_SRC=./test_strings.ratc
TEST_STRINGS=./test_strings.rvm
TEST_JUMPS_SRC=./test_jumps.ratc
TEST_JUMPS=./test_jumps.rvm
TEST_COMPARISONS_SRC=./test_comparisons.ratc
TEST_COMPARISONS=./test_comparisons.rvm
TEST_FILEIO_SRC=./test_fileio.ratc
TEST_FILEIO=./test_fileio.rvm
TEST_DYNAMIC_SRC=./test_dynamic.ratc
TEST_DYNAMIC=./test_dynamic.rvm
TEST_VOCAB_SRC=./test_vocab.ratc
TEST_VOCAB=./test_vocab.rvm
TEST_OBJTREE_SRC=./test_objtree.ratc
TEST_OBJTREE=./test_objtree.rvm


all:  $(TEST_COMPARISONS) $(TEST_DYNAMIC) $(TEST_EXPLODE) $(TEST_FILEIO) \
	  $(TEST_JUMPS) $(TEST_LISTS) $(TEST_MAPS) $(TEST_MATH) $(TEST_OBJECTS) \
	  $(TEST_STACK) $(TEST_STRINGS) $(TEST_VALUES) $(TEST_VOCAB) \
	  $(TEST_OBJTREE) $(TEST_ALL)


$(TEST_ALL): $(BUILD) $(TEST_ALL_SRC)
	$(BUILD) $(TEST_ALL_SRC) -o $(TEST_ALL)
	$(RUNNER) $(TEST_ALL) -silent
$(TEST_COMPARISONS): $(BUILD) $(TEST_COMPARISONS_SRC)
	$(BUILD) $(TEST_COMPARISONS_SRC) -o $(TEST_COMPARISONS)
	$(RUNNER) $(TEST_COMPARISONS) -silent
$(TEST_DYNAMIC): $(BUILD) $(TEST_DYNAMIC_SRC)
	$(BUILD) $(TEST_DYNAMIC_SRC) -o $(TEST_DYNAMIC)
	$(RUNNER) $(TEST_DYNAMIC) -silent
$(TEST_EXPLODE): $(BUILD) $(TEST_EXPLODE_SRC)
	$(BUILD) $(TEST_EXPLODE_SRC) -o $(TEST_EXPLODE)
	$(RUNNER) $(TEST_EXPLODE) -silent
$(TEST_FILEIO): $(BUILD) $(TEST_FILEIO_SRC)
	$(BUILD) $(TEST_FILEIO_SRC) -o $(TEST_FILEIO)
	$(RUNNER) $(TEST_FILEIO) -silent
$(TEST_JUMPS): $(BUILD) $(TEST_JUMPS_SRC)
	$(BUILD) $(TEST_JUMPS_SRC) -o $(TEST_JUMPS)
	$(RUNNER) $(TEST_JUMPS) -silent
$(TEST_LISTS): $(BUILD) $(TEST_LISTS_SRC)
	$(BUILD) $(TEST_LISTS_SRC) -o $(TEST_LISTS)
	$(RUNNER) $(TEST_LISTS) -silent
$(TEST_MAPS): $(BUILD) $(TEST_MAPS_SRC)
	$(BUILD) $(TEST_MAPS_SRC) -o $(TEST_MAPS)
	$(RUNNER) $(TEST_MAPS) -silent
$(TEST_MATH): $(BUILD) $(TEST_MATH_SRC)
	$(BUILD) $(TEST_MATH_SRC) -o $(TEST_MATH)
	$(RUNNER) $(TEST_MATH) -silent
$(TEST_OBJECTS): $(BUILD) $(TEST_OBJECTS_SRC)
	$(BUILD) $(TEST_OBJECTS_SRC) -o $(TEST_OBJECTS)
	$(RUNNER) $(TEST_OBJECTS) -silent
$(TEST_STACK): $(BUILD) $(TEST_STACK_SRC)
	$(BUILD) $(TEST_STACK_SRC) -o $(TEST_STACK)
	$(RUNNER) $(TEST_STACK) -silent
$(TEST_STRINGS): $(BUILD) $(TEST_STRINGS_SRC)
	$(BUILD) $(TEST_STRINGS_SRC) -o $(TEST_STRINGS)
	$(RUNNER) $(TEST_STRINGS) -silent
$(TEST_VALUES): $(BUILD) $(TEST_VALUES_SRC)
	$(BUILD) $(TEST_VALUES_SRC) -o $(TEST_VALUES)
	$(RUNNER) $(TEST_VALUES) -silent
$(TEST_VOCAB): $(BUILD) $(TEST_VOCAB_SRC)
	$(BUILD) $(TEST_VOCAB_SRC) -o $(TEST_VOCAB)
	$(RUNNER) $(TEST_VOCAB) -silent
$(TEST_OBJTREE): $(BUILD) $(TEST_OBJTREE_SRC)
	$(BUILD) $(TEST_OBJTREE_SRC) -o $(TEST_OBJTREE)
	$(RUNNER) $(TEST_OBJTREE) -silent

clean:
	$(RM) *.rvm

.PHONY: all clean

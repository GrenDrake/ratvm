BUILD=../build

USERTESTS_SRC=user_tests.ratc
USERTESTS=./user_tests.rvm
FIBTEST_SRC=fibonacci.ratc
FIBTEST=./fibonacci.rvm
FIZZBUZZ_SRC=./fizzbuzz.ratc
FIZZBUZZ=./fizzbuzz.rvm
TINY_SRC=./tiny.ratc
TINY=./tiny.rvm


all:  $(USERTESTS) $(FIBTEST) $(FIZZBUZZ) $(TINY)

$(USERTESTS): $(BUILD) $(USERTESTS_SRC)
	$(BUILD) $(USERTESTS_SRC) -o $(USERTESTS)

$(FIBTEST): $(BUILD) $(FIBTEST_SRC)
	$(BUILD) $(FIBTEST_SRC) -o $(FIBTEST)

$(FIZZBUZZ): $(BUILD) $(FIZZBUZZ_SRC)
	$(BUILD) $(FIZZBUZZ_SRC) -o $(FIZZBUZZ)

$(TINY): $(BUILD) $(TINY_SRC)
	$(BUILD) $(TINY_SRC) -o $(TINY)


clean:
	$(RM) *.rvm

.PHONY: all clean

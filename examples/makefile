BUILD=../build

USERTESTS_SRC=user_tests.qc
USERTESTS=./user_tests.qvm
FIBTEST_SRC=fibonacci.qc
FIBTEST=./fibonacci.qvm
FIZZBUZZ_SRC=./fizzbuzz.qc
FIZZBUZZ=./fizzbuzz.qvm


all:  $(USERTESTS) $(FIBTEST) $(FIZZBUZZ)

$(USERTESTS): $(BUILD) $(USERTESTS_SRC)
	$(BUILD) $(USERTESTS_SRC) -o $(USERTESTS)

$(FIBTEST): $(BUILD) $(FIBTEST_SRC)
	$(BUILD) $(FIBTEST_SRC) -o $(FIBTEST)

$(FIZZBUZZ): $(BUILD) $(FIZZBUZZ_SRC)
	$(BUILD) $(FIZZBUZZ_SRC) -o $(FIZZBUZZ)


clean:
	$(RM) *.qvm

.PHONY: all clean

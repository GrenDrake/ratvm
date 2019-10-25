#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <sstream>

#include "../build/builderror.h"
#include "testing.h"



int parseAsInt(const std::string &text);


void test_parseInt_basic() {
    int result;

    result = parseAsInt("123456");
    assert_equal(result, 123456, "parseInt_basic: 123456 wrong");
    result = parseAsInt("-98765");
    assert_equal(result, -98765, "parseInt_basic: -98765 wrong");

    result = parseAsInt("0x5AbCdEf");
    assert_equal(result, 0x5AbCdEf, "parseInt_basic: 0x5AbCdEf wrong");
    result = parseAsInt("0XFe5DcB4a");
    assert_equal(result, 0XFe5DcB4a, "parseInt_basic: 0x5AbCdEf wrong");

    result = parseAsInt("0b10110010");
    assert_equal(result, 178, "parseInt_basic: 0b10110010 wrong");
    result = parseAsInt("0B10110010");
    assert_equal(result, 178, "parseInt_basic: 0B10110010 wrong");
}

void test_parseInt_commas() {
    int result;

    result = parseAsInt("6,123,456");
    assert_equal(result, 6123456, "parseInt_basic: 6,123,456 wrong");
    result = parseAsInt("-2_498_765");
    assert_equal(result, -2498765, "parseInt_basic: -2_498_765 wrong");

    result = parseAsInt("0x5,AbC,dEf");
    assert_equal(result, 0x5AbCdEf, "parseInt_basic: 0x5,AbC,dEf wrong");
    result = parseAsInt("0XF_e5Dc_B4a");
    assert_equal(result, 0XFe5DcB4a, "parseInt_basic: 0XF_e5Dc_B4a wrong");

    result = parseAsInt("0b10,110,010");
    assert_equal(result, 178, "parseInt_basic: 0b10,110,010 wrong");
    result = parseAsInt("0B10_110_010");
    assert_equal(result, 178, "parseInt_basic: 0B10_110_010 wrong");
}

int main() {

    try {
        test_parseInt_basic();
        test_parseInt_commas();
    } catch (BuildError &e) {
        std::cerr << "Test Failed (BuildError): " << e.getMessage() << '\n';
        return 1;
    } catch (IntParseError &e) {
        std::cerr << "Test Failed (IntParseError): " << e.what() << '\n';
        return 1;
    } catch (TestFailed &e) {
        std::cerr << "Test Failed: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <sstream>

#include "../builder/builderror.h"
#include "../builder/textutil.h"
#include "testing.h"


static void assert_equal(IntParseError left, IntParseError right, const std::string &message) {
    if (left != right) {
        std::stringstream ss;
        ss << message << ": " << left << " does not equal " << right << '.';
        throw TestFailed(ss.str());
    }
}

void test_parseInt_basic() {
    IntParseError rc;
    int result = 0;

    rc = parseAsInt("123456", result);
    assert_equal(rc, IntParseError::OK, "parseInt_basic: 123456 failed to parse");
    assert_equal(result, 123456, "parseInt_basic: 123456 wrong");
    rc = parseAsInt("-98765", result);
    assert_equal(rc, IntParseError::OK, "parseInt_basic: -98765 failed to parse");
    assert_equal(result, -98765, "parseInt_basic: -98765 wrong");

    rc = parseAsInt("0x5AbCdEf", result);
    assert_equal(rc, IntParseError::OK, "parseInt_basic: 0x5AbCdEf failed to parse");
    assert_equal(result, 0x5AbCdEf, "parseInt_basic: 0x5AbCdEf wrong");
    rc = parseAsInt("0XFe5DcB4a", result);
    assert_equal(rc, IntParseError::OK, "parseInt_basic: 0XFe5DcB4a failed to parse");
    assert_equal(result, static_cast<int>(0XFe5DcB4a), "parseInt_basic: 0x5AbCdEf wrong");

    rc = parseAsInt("0b10110010", result);
    assert_equal(rc, IntParseError::OK, "parseInt_basic: 0b10110010 failed to parse");
    assert_equal(result, 178, "parseInt_basic: 0b10110010 wrong");
    rc = parseAsInt("0B10110010", result);
    assert_equal(rc, IntParseError::OK, "parseInt_basic: 0B10110010 failed to parse");
    assert_equal(result, 178, "parseInt_basic: 0B10110010 wrong");
}
// 


void test_parseInt_extents() {
    IntParseError rc;
    int result = 0;

    rc = parseAsInt("2147483647", result);
    assert_equal(rc, IntParseError::OK, "parseInt_basic: 2147483647 failed to parse");
    assert_equal(result, 2147483647, "parseInt_basic: 2147483647 wrong");

    rc = parseAsInt("-2147483648", result);
    assert_equal(rc, IntParseError::OK, "parseInt_basic: -2147483648 failed to parse");
    assert_equal(result, -2147483648, "parseInt_basic: -2147483648 wrong");

    rc = parseAsInt("0xFFFFFFFF", result);
    assert_equal(rc, IntParseError::OK, "parseInt_basic: 0xFFFFFFFF failed to parse");
    assert_equal(result, 0xFFFFFFFF, "parseInt_basic: 0xFFFFFFFF wrong");

    rc = parseAsInt("0b11111111111111111111111111111111", result);
    assert_equal(rc, IntParseError::OK, "parseInt_basic: 0b11111111111111111111111111111111 failed to parse");
    assert_equal(result, 0xFFFFFFFF, "parseInt_basic: 0b11111111111111111111111111111111 wrong");


    rc = parseAsInt("2147483648", result);
    assert_equal(rc, IntParseError::OUT_OF_RANGE, "parseInt_basic: 2147483647 not reported as out of range");

    rc = parseAsInt("-2147483649", result);
    assert_equal(rc, IntParseError::OUT_OF_RANGE, "parseInt_basic: -2147483648 not reported as out of range");

    rc = parseAsInt("0x1FFFFFFFF", result);
    assert_equal(rc, IntParseError::OUT_OF_RANGE, "parseInt_basic: 0x1FFFFFFFF not reported as out of range");

    rc = parseAsInt("0b111111111111111111111111111111111", result);
    assert_equal(rc, IntParseError::OUT_OF_RANGE, "parseInt_basic: 0b111111111111111111111111111111111 not reported as out of range");
}

void test_parseInt_commas() {
    IntParseError rc;
    int result = 0;

    rc = parseAsInt("-2_498_765", result);
    assert_equal(rc, IntParseError::OK, "parseInt_basic: -2_498_765 failed to parse");
    assert_equal(result, -2498765, "parseInt_basic: -2_498_765 wrong");

    rc = parseAsInt("0XF_e5Dc_B4a", result);
    assert_equal(rc, IntParseError::OK, "parseInt_basic: 0XF_e5Dc_B4a failed to parse");
    assert_equal(result, 0XFe5DcB4a, "parseInt_basic: 0XF_e5Dc_B4a wrong");

    rc = parseAsInt("0B1011_0010", result);
    assert_equal(rc, IntParseError::OK, "parseInt_basic: 0B1011_0010 failed to parse");
    assert_equal(result, 178, "parseInt_basic: 0B1011_0010 wrong");
}

int main() {

    try {
        test_parseInt_basic();
        test_parseInt_extents();
        test_parseInt_commas();
    } catch (BuildError &e) {
        std::cerr << "Test Failed (BuildError): " << e.getMessage() << '\n';
        return 1;
    } catch (TestFailed &e) {
        std::cerr << "Test Failed: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
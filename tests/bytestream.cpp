#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <sstream>

#include "../src/bytestream.h"
#include "testing.h"


void test_add_8() {
    ByteStream bs;
    bs.add_8(25);
    assert_equal(bs.size(), 1, "add_8: wrong result size");
    assert_equal(bs.read_8(0), 25, "add_8: read wrong data");
}

void test_add_16() {
    ByteStream bs;
    bs.add_16(1000);
    assert_equal(bs.size(), 2, "add_16: wrong result size");
    assert_equal(bs.read_8(0), 232, "add_16: first byte incorrect (wrong byte order?)");
    assert_equal(bs.read_16(0), 1000, "add_16: read wrong data");
}

void test_add_32() {
    ByteStream bs;
    bs.add_32(1000000);
    assert_equal(bs.size(), 4, "add_32: wrong result size");
    assert_equal(bs.read_8(0), 64, "add_32: first byte incorrect (wrong byte order?)");
    assert_equal(bs.read_32(0), 1000000, "add_32: read wrong data");
}

void test_read_middle() {
    ByteStream bs;
    for (int i = 0; i < 12; ++i) {
        bs.add_8(i);
    }
    uint32_t value = bs.read_32(4);

    assert_equal(value, 117835012, "test_read_middle: read wrong data");
}

void test_padding() {
    ByteStream bs;
    bs.padTo(4);
    assert_equal(bs.size(), 4, "test_padding: padded to wrong size");
    bs.add_8(4);
    bs.padTo(256);
    assert_equal(bs.size(), 256, "test_padding: padded to wrong size");
    bs.add_8(4);
    bs.padTo(512);
    assert_equal(bs.size(), 512, "test_padding: padded to wrong size");
    bs.padTo(542);
    assert_equal(bs.size(), 542, "test_padding: padded to wrong size");
    bs.padTo(2);
    assert_equal(bs.size(), 542, "test_padding: padded to wrong size");
}

void test_append() {
    ByteStream bs1;
    bs1.add_8(5);
    bs1.add_8(4);
    bs1.add_8(56);
    assert_equal(bs1.size(), 3, "test_append: first stream has wrong size");

    ByteStream bs2;
    bs2.add_8(74);
    bs2.add_8(38);
    bs2.add_8(56);
    bs2.add_8(200);
    bs2.add_8(251);
    assert_equal(bs2.size(), 5, "test_append: second stream has wrong size");

    bs1.append(bs2);
    assert_equal(bs1.size(), 8, "test_append: first stream has wrong size post append");
    assert_equal(bs2.size(), 5, "test_append: second stream has wrong size post append");

    assert_equal(bs1.read_8(0), 5, "test_append: result array has bad first value");
    assert_equal(bs1.read_8(1), 4, "test_append: result array has bad second value");
    assert_equal(bs1.read_8(2), 56, "test_append: result array has bad third value");
    assert_equal(bs1.read_8(3), 74, "test_append: result array has bad fourth value");
    assert_equal(bs1.read_8(4), 38, "test_append: result array has bad fifth value");
    assert_equal(bs1.read_8(5), 56, "test_append: result array has bad sixth value");
    assert_equal(bs1.read_8(6), 200, "test_append: result array has bad seventh value");
    assert_equal(bs1.read_8(7), 251, "test_append: result array has bad eighth value");
}

int main() {

    try {
        test_add_8();
        test_add_16();
        test_add_32();
        test_read_middle();
        test_padding();
        test_append();
    } catch (TestFailed &e) {
        std::cerr << "Test Failed: " << e.what() << '\n';
    }

    return 0;
}
#include <iostream>
#pragma warning(disable:4251 4275)
#include <gtest/gtest.h>

int main(int argc, char **argv) {
    std::cout << "TestUtilities\n";

    testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();

    return ret;
}
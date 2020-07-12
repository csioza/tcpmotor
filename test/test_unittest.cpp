#include "tcpmotor.h"
#include "gtest/gtest.h"


int fun(int a) {
  return a + 1;
}

TEST(FunTest, HandlesZeroInput) {
  EXPECT_EQ(1, fun(0));
  EXPECT_EQ(1, 1+0);
}

TEST(FunTest2, HandlesZeroInput) {
  EXPECT_EQ(1, fun(0));
  EXPECT_EQ(1, 1+0);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
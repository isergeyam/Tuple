#include "../src/Tuple.hpp"
#include <gtest/gtest.h>
TEST(TupleTest, Constructor) {
  Tuple<int, int, double> mtpl(1, 1, 1.);
  EXPECT_EQ(get<0>(mtpl), 1);
  EXPECT_EQ(get<0>(mtpl), 1);
  EXPECT_EQ(get<0>(mtpl), 1.);
}
TEST(TupleTest, Assignment) {
  Tuple<int, double, double> mtpl(1, 2.4, 2.5);
  get<1>(mtpl) = 2.3;
  EXPECT_EQ(get<1>(mtpl), 2.3);
}
TEST(TupleTest, GetByType) {
  Tuple<int, double, double> mtpl(3, 2, 1);
  EXPECT_EQ(get<int>(mtpl), 3);
  // get<double>(mtpl); <- static assertion failed.
}
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

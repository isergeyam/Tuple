#include "../src/Tuple.hpp"
#include <gtest/gtest.h>
TEST(TupleTest, Constructor) {
  Tuple<int, int, double> mtpl(1, 1, 1.);
  EXPECT_EQ(get<0>(mtpl), 1);
  EXPECT_EQ(get<0>(mtpl), 1);
  EXPECT_EQ(get<0>(mtpl), 1.);
  Tuple<int, int, double> other(mtpl);
  EXPECT_EQ(mtpl, other);
  Tuple<int> t1(0);
  Tuple<int> t2(t1);
  EXPECT_EQ(t1, t2);
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
TEST(TupleTest, TupleCat) {
  Tuple<int, int, double> mt1(1, 1, 1.);
  Tuple<float, float, int> mt2(1., 1., 1);
  EXPECT_TRUE(TwoTuplesCat(mt1, mt2) == MakeTuple(1, 1, 1., 1., 1., 1));
}
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

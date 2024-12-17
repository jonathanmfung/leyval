#include <catch2/catch_test_macros.hpp>

#include "../src/fixed_point.hpp"

SCENARIO("Fixed supports arithmetic", "[fixed_point]")
{
  using namespace leyval;

  constexpr int a{ 4 };
  constexpr int b{ 3 };
  constexpr int c{ -3 };
  constexpr int d{ -4 };

  WHEN("adding, it acts like an integer")
  {
    REQUIRE(Fixed<a>{ 1 } + Fixed<a>{ 2 } == Fixed<a>{ 3 });
    REQUIRE(Fixed<a>{ 1 } + Fixed<a>{ 20 } == Fixed<a>{ 21 });
  }
  WHEN("subtracting, it acts like a (signed) integer")
  {
    REQUIRE(Fixed<a>{ 3 } - Fixed<a>{ 2 } == Fixed<a>{ 1 });
    REQUIRE(Fixed<a>{ 30 } - Fixed<a>{ 2 } == Fixed<a>{ 28 });
    REQUIRE(Fixed<a>{ 30 } - Fixed<a>{ -20 } == Fixed<a>{ 50 });
    REQUIRE(Fixed<a>{ -30 } - Fixed<a>{ 20 } == Fixed<a>{ -50 });
  }
  WHEN("multiplying, it acts like an integer")
  {
    REQUIRE(Fixed<a>{ 2 } * Fixed<a>{ 3 } == Fixed<a>{ 6 });
    REQUIRE(Fixed<a>{ 2 } * Fixed<a>{ 6 } == Fixed<a>{ 12 });
    REQUIRE(Fixed<c>{ 3 } * Fixed<c>{ 2 } == Fixed<c>{ 6 });
    REQUIRE(Fixed<c>{ -3 } * Fixed<c>{ 2 } == Fixed<c>{ -6 });
    REQUIRE(Fixed<c>{ -2 } * Fixed<c>{ -3 } == Fixed<c>{ 6 });
  }
  WHEN("dividing, sub_unit is rounded")
  {
    CHECK(Fixed<a>{ 10 } / Fixed<a>{ 2 } == Fixed<a>{ 5 });
    CHECK(Fixed<a>{ 1 } / Fixed<a>{ 2 } == Fixed<b>{ 5 });
    CHECK(Fixed<a>{ -10 } / Fixed<a>{ 2 } == Fixed<a>{ -5 });
    CHECK(Fixed<a>{ 10 } / Fixed<a>{ -2 } == Fixed<a>{ -5 });
    CHECK(Fixed<a>{ -10 } / Fixed<a>{ -2 } == Fixed<a>{ 5 });
  }

  THEN("it can be rescaled")
  {
    REQUIRE(Fixed<a>{ 123 }.template rescale<a>() == Fixed<a>{ 123 });
    REQUIRE(Fixed<a>{ 123 }.template rescale<b>() == Fixed<b>{ 1230 });
    REQUIRE(Fixed<c>{ 123 }.template rescale<d>() == Fixed<d>{ 1230 });
    REQUIRE(Fixed<b>{ 123 }.template rescale<a>() == Fixed<a>{ 12 });
    REQUIRE(Fixed<d>{ 123 }.template rescale<c>() == Fixed<c>{ 12 });
  }
  GIVEN("different ScaleExps")
  {
    THEN("all binary operations appropriately rescale")
    {
      REQUIRE(Fixed<b>{ 1 } + Fixed<a>{ 2 } == Fixed<b>{ 21 });
      REQUIRE(Fixed<a>{ 1 } + Fixed<b>{ 2 } == Fixed<b>{ 12 });
      REQUIRE(Fixed<c>{ 1 } + Fixed<d>{ 2 } == Fixed<d>{ 12 });
      REQUIRE(Fixed<d>{ 1 } + Fixed<c>{ 2 } == Fixed<d>{ 21 });
      REQUIRE(Fixed<a>{ 3 } - Fixed<b>{ 2 } == Fixed<b>{ 28 });
      REQUIRE(Fixed<a>{ 3 } * Fixed<b>{ 2 } == Fixed<a>{ 6 });
      REQUIRE(Fixed<a>{ 1 } / Fixed<b>{ 2 } == Fixed<b>{ 5 });
      // TODO: test narrowing/rounding errors
      REQUIRE(Fixed<b>{ 10 } == Fixed<a>{ 1 });
      REQUIRE(Fixed<a>{ 1 } == Fixed<b>{ 10 });
    }
  }
}

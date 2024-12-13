#include <catch2/catch_test_macros.hpp>
#include <vector>

// TODO: fix include dir location for tests (https://stackoverflow.com/a/62602181/28633986)
#include "../src/money.hpp"

unsigned int
Factorial(unsigned int number)
{
  return number <= 1 ? number : Factorial(number - 1) * number;
}

TEST_CASE("Factorials are computed", "[factorial]")
{
  REQUIRE(Factorial(1) == 1);
  REQUIRE(Factorial(2) == 2);
  REQUIRE(Factorial(3) == 6);
  REQUIRE(Factorial(10) == 3628800);
}

SCENARIO("Money is calculated", "[money]")
{
  using namespace leyval;
  Money m{10, 99};
  Money min {0, 1};
  GIVEN("A Money close to overflow")
  {
    THEN("Adding to it will increment the main_unit") {
      REQUIRE(m.get_main() < (m + min).get_main());
    }

  }
}

SCENARIO("vector can be sized and resized")
{
  GIVEN("An empty vector")
  {
    auto v = std::vector<std::string>{};

    // Validate assumption of the GIVEN clause
    THEN("The size and capacity start at 0")
    {
      REQUIRE(v.size() == 0);
      REQUIRE(v.capacity() == 0);
    }

    // Validate one use case for the GIVEN object
    WHEN("push_back() is called")
    {
      v.push_back("hullo");

      THEN("The size changes")
      {
        REQUIRE(v.size() == 1);
        REQUIRE(v.capacity() >= 1);
      }
    }
  }
}

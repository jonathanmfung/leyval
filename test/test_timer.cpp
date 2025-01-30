#include <catch2/catch_test_macros.hpp>

#include "../src/util/timer.hpp"

SCENARIO("Timer", "[timer]")
{
  using namespace leyval;

  WHEN("initialized with an unsigned int")
  {
    Timer t{ 1u };
    THEN("it is converted to functor")
    {
      REQUIRE(t.tick_and_check());
    }
  }

  WHEN("initialized with 0")
  {
    THEN("it cannot be constructed")
    {
      REQUIRE_THROWS(Timer{ 0u });
    }
  }

  WHEN("initialized with a functor")
  {
    Timer t{ []() { return 1u; } };
    THEN("the functor is copied and it generates the starting value")
    {
      REQUIRE(t.tick_and_check());
    }
  }

  WHEN("reset() is called")
  {
    struct CumulativeAdd
    {
      unsigned int x;
      unsigned int val{ 0 };
      unsigned int operator()() { return val += x; };
    };

    THEN("the generator is called again,.")
    {
      CumulativeAdd functor{ 2 };
      Timer t{ functor }; // functor is copied: 0 + 2
      REQUIRE(functor() == 2);
      REQUIRE_FALSE(t.tick_and_check());
      REQUIRE(t.tick_and_check());

      t.reset(); // m_gen.val = 2 + 2
      REQUIRE_FALSE(t.tick_and_check());
      REQUIRE(t.tick_and_check());
      REQUIRE(functor() == 4);

      t.reset(); // m_gen.val = 4 + 2
      REQUIRE_FALSE(t.tick_and_check());
      REQUIRE_FALSE(t.tick_and_check());
      REQUIRE_FALSE(t.tick_and_check());
      REQUIRE(t.tick_and_check());
    }
  }

  GIVEN("a functor with capture")
  {
    THEN("reset() will remember reference captures")
    {
      unsigned int x{ 2u };
      auto gen{ [&x]() { return x + 1u; } };
      Timer t{ gen };

      x = 0u;
      t.reset(); // m_timer = 1
      REQUIRE(t.tick_and_check());
      REQUIRE_THROWS(t.tick_and_check());

      x = 1u;
      t.reset(); // m_timer = 2
      REQUIRE_FALSE(t.tick_and_check());
      REQUIRE(t.tick_and_check());

      x = 2u;
      t.reset();
      REQUIRE_FALSE(t.tick_and_check());
    }
  }
}

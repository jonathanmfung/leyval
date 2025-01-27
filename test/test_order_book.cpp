#include <catch2/catch_test_macros.hpp>

#include "../src/order_book.hpp"

SCENARIO("OrderBook is essentially a wrapper over Ask/BidContainer",
         "[order_book]")
{
  using namespace leyval;
  OrderBook ob{};
  constexpr int AGENT_ID{ 0 };

  GIVEN("an empty OrderBook")
  {
    THEN("the size of both Ask/BidContainer starts at 0")
    {
      REQUIRE(ob.update_get_state().num_orders_ask == 0);
    }
  }

  WHEN("insert() is called")
  {
    ob.insert(LimitOrderReq{ .volume = 0,
                             .agent_id = AGENT_ID,
                             .price = 1,
                             .order_dir = OrderDir::Ask });

    THEN("The corresponding container size increments")
    {
      REQUIRE(ob.update_get_state().num_orders_ask == 1);

      WHEN("remove_earliest_order() is called")
      {
        ob.remove_earliest_order(AGENT_ID, OrderDir::Ask);

        THEN("The size reverts back to 0")
        {
          REQUIRE(ob.update_get_state().num_orders_ask == 0);

          WHEN("remove_earliest_order() is called again")
          {
            THEN("there are no more orders to remove")
            {
              REQUIRE_FALSE(ob.remove_earliest_order(AGENT_ID, OrderDir::Ask));
            }
          }
        }
      }
    }
  }
}

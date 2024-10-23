#include "agent.hpp"
#include "exchange.hpp"
#include "matching_system.hpp"
#include "order_book.hpp"

#include <iostream>
#include <format>

// TODO: add global project namespace

int
main()
{
  Agent a1{ 100 };
  Agent a2{ 80 };

  // TODO: Need to initially saturate orderbook
  //       Empty Bid/AskContainers is making current_best_price throw.

  // Can saturate by using m_order_book.insert(lor.to_full()) with random spread

  Exchange exch{ OrderBook{},
                 std::vector{ a1, a2 },
                 MatchingSystem{ MatchingSystem::fifo } };

  exch.saturate();


  exch.run();
  std::cout << std::format("{}", exch);

  exch.run();
  std::cout << std::format("{}", exch);

  return 0;
}

// TODO: Addd methods for candlestick/line chart data generation (open,close, low,high)

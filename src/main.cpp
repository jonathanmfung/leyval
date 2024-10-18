#include "agent.hpp"
#include "exchange.hpp"
#include "matching_system.hpp"
#include "order_book.hpp"

// TODO: add global project namespace

int
main()
{
  Agent a1{ 100 };
  Agent a2{ 80 };

  // TODO: Need to initially saturate orderbook
  Exchange exch{ OrderBook{},
                 std::vector{ a1, a2 },
                 MatchingSystem{ MatchingSystem::fifo } };

  exch.run();

  return 0;
}

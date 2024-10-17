#include <algorithm>
#include <functional>
#include <memory>
#include <optional>

#include "agent.hpp"
#include "matching_system.hpp"
#include "order_book.hpp"
#include "exchange.hpp"

// TODO: add global project namespace

int
main()
{
  Agent a1{ 100 };
  Agent a2{ 80 };

  const MatchingSystem ms{ MatchingSystem::fifo };
  OrderBook ob{ ms };

  return 0;
}

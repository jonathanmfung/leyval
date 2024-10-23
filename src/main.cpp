#include <format>
#include <iostream>
#include <random>
#include <ranges>

#include "agent.hpp"
#include "exchange.hpp"
#include "matching_system.hpp"
#include "order_book.hpp"

// TODO: add global project namespace

int
main()
{
  std::random_device rd;  // a seed source for the random number engine
  std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<> capital(800, 1'200);

  std::vector<Agent> agents{};

  const int n_agents{ 10 };
  for (const int _ : std::views::iota(0, n_agents)) {
    agents.emplace_back(Agent{ capital(gen) });
  }

  Exchange exch{ OrderBook{}, agents, MatchingSystem{ MatchingSystem::fifo } };

  exch.saturate();

  for (const int i : std::views::iota(1, 6)) {
    std::cout << std::format("\n***********************\nRun #{}\n", i);
    exch.run();
    std::cout << std::format("{}", exch);
  }

  return 0;
}

// TODO: Addd methods for candlestick/line chart data generation (open,close,
// low,high)

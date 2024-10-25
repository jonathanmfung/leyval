#include <random>
#include <ranges>

#include "my_spdlog.hpp"

#include "agent.hpp"
#include "exchange.hpp"
#include "matching_system.hpp"
#include "order_book.hpp"

// TODO: add global project namespace

int
main()
{
  // Same format as default, but with YYMMDD instead of YYYY-MM-DD, and source function
  spdlog::set_pattern("[%C%m%d %T.%e] [%^%-8l%$] [%s:%# (%!)] %v");
  spdlog::set_level(spdlog::level::trace); // Set global log level

  std::random_device rd;  // a seed source for the random number engine
  std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<> capital(800, 1'200);

  std::vector<Agent> agents{};

  const int n_agents{ 10 };
  for (const int _ : std::views::iota(0, n_agents)) {
    agents.emplace_back(Agent{ capital(gen) });
  }

  Exchange exch{ OrderBook{}, agents, MatchingSystem{ MatchingSystem::fifo } };

  // TODO: Allow dumping exch state to json (order_book, agents)
  //       For consumption by python (plotting)

  exch.saturate();

  for ([[maybe_unused]] const int i : std::views::iota(1, 6)) {
    SPDLOG_INFO("Run #{} ***********************", i);
    exch.run();
    SPDLOG_INFO("{}", exch);
  }

  return 0;
}

// TODO: Addd methods for candlestick/line chart data generation (open,close,
// low,high)

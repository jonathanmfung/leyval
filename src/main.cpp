#include <memory>
#include <random>
#include <ranges>
#include <filesystem>

#include "my_spdlog.hpp"
#include "serializable.hpp"
#include <spdlog/common.h>

#include "agent.hpp"
#include "exchange.hpp"
#include "matching_system.hpp"
#include "order_book.hpp"

// TODO: add global project namespace
// TODO: add Constants namespace

int
main()
{
  // Same format as default, but with YYMMDD instead of YYYY-MM-DD, and source
  // function
  spdlog::set_pattern("[%C%m%d %T.%e] [%^%-8l%$] [%s:%# (%!)] %v");
  spdlog::set_level(spdlog::level::trace); // Set global log level

  std::random_device rd;  // a seed source for the random number engine
  std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<> capital(800, 1'200);

  std::vector<Exchange::Agent_t> agents{};

  const int n_agents{ 10 };
  for ([[maybe_unused]] const int _ : std::views::iota(0, n_agents)) {
    agents.emplace_back(std::make_unique<Agent_JFProvider>(capital(gen)));
  }

  Exchange exch{ OrderBook{}, std::move(agents), MatchingSystem{ MatchingSystem::fifo } };

  exch.saturate();

  json exchange_states;
  exchange_states.push_back(exch);

  for ([[maybe_unused]] const int i : std::views::iota(1, 6)) {
    SPDLOG_INFO("Run #{} ***********************", i);
    exch.run();
    exchange_states.push_back(exch);
    SPDLOG_INFO("{}", exch);
  }

  const std::filesystem::path DATA_DIR {"data"};
  std::filesystem::create_directory(DATA_DIR);
  std::ofstream out_file(DATA_DIR / "pretty.json");
  out_file << std::setw(2) << exchange_states << std::endl;

  return 0;
}

// TODO: Addd methods for candlestick/line chart data generation (open,close,
// low,high)

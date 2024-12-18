#include <algorithm>
#include <filesystem>
#include <memory>
#include <random>
#include <ranges>

#include "my_spdlog.hpp"
#include "serializable.hpp"

#include "agent.hpp"
#include "constants.hpp"
#include "exchange.hpp"
#include "matching_system.hpp"
#include "order_book.hpp"

int
main()
{
  using namespace leyval;
  // Same format as default, but with YYMMDD instead of YYYY-MM-DD, and source
  // function
  spdlog::set_pattern("[%C%m%d %T.%e] [%^%-8l%$] [%s:%# (%!)] %v");
  spdlog::set_level(spdlog::level::trace); // Set global log level

  using PRNG = std::mt19937;
  std::random_device rd;
  std::seed_seq seed{ rd(), rd(), rd(), rd(), rd(), rd() };
  PRNG rng(seed);

  std::uniform_int_distribution<> capital(80'000, 120'000);
  std::vector<Exchange<PRNG>::Agent_t> agents{};

  for (const int _ : std::views::iota(0, constants::n_providers)) {
    agents.emplace_back(
      std::make_unique<Agent_JFProvider<PRNG>>(capital(rng), rng));
  }

  for (const int _ : std::views::iota(0, constants::n_takers)) {
    agents.emplace_back(
      std::make_unique<Agent_JFTaker<PRNG>>(capital(rng), rng));
  }
  std::ranges::shuffle(
    agents, rng); // NOTE: may want to not shuffle when grouping in python

  Exchange exch{
    OrderBook{}, std::move(agents), MatchingSystem{ MatchingSystem::fifo }, rng
  };

  exch.saturate();

  nlohmann::json exchange_states;
  exchange_states.push_back(exch);

  for ([[maybe_unused]] const int i : std::views::iota(0, constants::n_runs)) {
    SPDLOG_INFO("Run #{} ***********************", i + 1);
    exch.run();
    exchange_states.push_back(exch);
    SPDLOG_INFO("{}", exch);
  }

  std::filesystem::create_directory(constants::data_dir);
  std::ofstream out_file(constants::data_dir / "pretty.json");
  out_file << std::setw(2) << exchange_states << std::endl;
  SPDLOG_INFO("SIMULATION FINISHED");

  return 0;
}

// TODO: Addd methods for candlestick/line chart data generation (open,close,
// low,high)

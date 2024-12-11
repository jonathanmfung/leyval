#pragma once

#include <filesystem>

namespace leyval::constants {
const std::filesystem::path data_dir{ "data" };
constexpr int n_providers{ 70 };
constexpr int n_takers{ 100 };
constexpr int n_runs{ 200 };

namespace saturate {
constexpr int n_contracts_per_side{ 50 };
constexpr int price_center{ 100 };
constexpr int price_close_offset{ 2 };
constexpr int price_far_offset{ 10 };
static_assert(price_close_offset < price_far_offset);
}
}

#pragma once

#include <filesystem>

namespace leyval::constants {
const std::filesystem::path data_dir{ "data" };
constexpr int n_providers{ 70 };
constexpr int n_takers{ 100 };
constexpr int n_runs{ 200 };

namespace saturate {
constexpr int n_contracts_per_side{ 50 };
constexpr int price_center{ 100'00 };
constexpr int price_close_offset{ 1'00 };
constexpr int price_far_offset{ 2'50 };
static_assert(price_close_offset < price_far_offset);
}

namespace simulation_jericevich {
constexpr float nu{ 1.55 };
constexpr float taker_lambda_min{ 1.0 };
constexpr float taker_lambda_val{ 1.5 };
constexpr float taker_lambda_max{ 2.0 };
constexpr float provider_lambda_min{ 1.0 };
constexpr float provider_lambda_val{ 1.5 };
constexpr float provider_lambda_max{ 2.0 };
constexpr float fundamentalist_sigma{ 2.0 };
}
}

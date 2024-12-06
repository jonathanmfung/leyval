#pragma once

#include <nlohmann/json.hpp>

template<typename T>
concept Serializable = requires(const T& a, nlohmann::json& j) {
  { to_json(j, a) } -> std::same_as<void>;
};

namespace leyval {
template<typename T>
void
to_json(nlohmann::json& j, const std::unique_ptr<T>& unq)
{
  if (unq) {
    j = *unq;
  } else {
    j = nullptr;
  }
}
}

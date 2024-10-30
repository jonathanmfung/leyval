#pragma once

#include <nlohmann/json.hpp>
using json = nlohmann::json;

template<typename T>
concept Serializable = requires(const T& a, json& j) {
  { to_json(j, a) } -> std::same_as<void>;
};


template<typename T>
void
to_json(json& j, const std::unique_ptr<T>& unq)
{
  if (unq) {
    j = *unq;
  } else {
    j = nullptr;
  }
}

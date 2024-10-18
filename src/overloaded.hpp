#pragma once

// Helper struct for visit on variants of Concept.
// https://en.cppreference.com/w/cpp/utility/variant/visit
template<class... Ts>
struct overloaded : Ts...
{
  using Ts::operator()...;
};

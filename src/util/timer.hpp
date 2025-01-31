#pragma once

#include <functional>
#include <stdexcept>

namespace leyval {
class Timer
{
public:
  using num_t = unsigned int;
  using gen_t = std::function<num_t()>;

  explicit Timer(num_t num)
    : Timer{ [=]() { return num; } }
  {
  }

  // Please supply this with as little state as possible
  // Ideally is a pure function, or just RNG as state.
  Timer(gen_t gen)
    : m_gen{ gen }
    , m_timer{ gen() }
  {
    zero_check();
  }

  void reset()
  {
    m_timer = m_gen();
    zero_check();
  };

  [[nodiscard]] bool tick_and_check()
  {
    zero_check();
    m_timer -= 1;
    return m_timer == 0;
  }

private:
  gen_t m_gen;
  num_t m_timer;
  void zero_check()
  {
    if (m_timer == 0)
      throw std::logic_error("gen produced 0");
  }
};
}

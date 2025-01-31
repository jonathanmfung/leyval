#pragma once

namespace leyval {

// emulate named requirement RandomNumberDistribution
template<class Distribution, class PRNG>
class TruncatedDistribution
{
  using result_type = Distribution::result_type;

public:
  TruncatedDistribution(Distribution dist)
    : m_dist{ dist }
  {
  }

  // Julia Truncated Rand
  // orig:
  //   https://github.com/JuliaStats/Distributions.jl/commit/b94d291e317ce8a0f3f0433f4af20e83b9984773
  // 2: https://github.com/JuliaStats/Distributions.jl/pull/1553
  // 3:
  //   https://github.com/JuliaStats/Distributions.jl/blob/1e6801da6678164b13330cc1f16e670768d27330/src/truncate.jl#L216
  // This implementation:
  //   Simple rejection sampleing.
  //   Does not give a special case for small mass.
  result_type operator()(PRNG& prng, result_type lower, result_type upper)
  {
    typename Distribution::result_type result{ m_dist(prng) };
    while (!((lower <= result) && (result <= upper))) {
      result = m_dist(prng);
    }
    return result;
  }

private:
  Distribution m_dist;
};
}

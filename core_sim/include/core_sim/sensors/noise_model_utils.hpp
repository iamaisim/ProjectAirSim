// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI Consulting Corporation.  

// MIT License. All rights reserved.

#ifndef CORE_SIM_INCLUDE_CORE_SIM_SENSORS_NOISE_MODEL_UTILS_HPP_
#define CORE_SIM_INCLUDE_CORE_SIM_SENSORS_NOISE_MODEL_UTILS_HPP_

#include <random>

#include "core_sim/clock.hpp"

namespace microsoft {
namespace projectairsim {

using RealT = double;

template <typename TReturn, typename TDistribution, unsigned int Seed = 42>
class RandomGenerator {
 public:
  // for uniform distribution supply min and max (inclusive)
  // for gaussian distribution supply mean and sigma
  template <typename... DistArgs>
  explicit RandomGenerator(DistArgs... dist_args)
      : dist_(dist_args...), rand_(Seed) {}

  void seed(int val) { rand_.seed(val); }

  TReturn next() { return static_cast<TReturn>(dist_(rand_)); }

  void reset() {
    rand_.seed(Seed);
    dist_.reset();
  }

 private:
  TDistribution dist_;
  std::mt19937 rand_;
};

typedef RandomGenerator<RealT, std::normal_distribution<double>, 1>
    RandomGeneratorGaussianXT;
typedef RandomGenerator<RealT, std::normal_distribution<double>, 2>
    RandomGeneratorGaussianYT;
typedef RandomGenerator<RealT, std::normal_distribution<double>, 3>
    RandomGeneratorGaussianZT;
typedef RandomGenerator<float, std::normal_distribution<float>>
    RandomGeneratorGaussianF;

class RandomVectorGaussianR {
 public:
  RandomVectorGaussianR() {}
  RandomVectorGaussianR(RealT mean, RealT stddev)
      : rx_(mean, stddev), ry_(mean, stddev), rz_(mean, stddev) {}
  RandomVectorGaussianR(const Vector3& mean, const Vector3& stddev)
      : rx_(mean.x(), stddev.x()),
        ry_(mean.y(), stddev.y()),
        rz_(mean.z(), stddev.z()) {}

  void reset() {
    rx_.reset();
    ry_.reset();
    rz_.reset();
  }

  Vector3 next() { return Vector3(rx_.next(), ry_.next(), rz_.next()); }

 private:
  RandomGeneratorGaussianXT rx_;
  RandomGeneratorGaussianYT ry_;
  RandomGeneratorGaussianZT rz_;
};

class GaussianMarkov {
 public:
  GaussianMarkov() {}
  GaussianMarkov(float tau, float sigma,
                 float initial_output = 0)  // in seconds
  {
    Initialize(tau, sigma, initial_output);
  }
  void Initialize(float tau, float sigma,
                  float initial_output = 0)  // in seconds
  {
    tau_ = tau;
    sigma_ = sigma;
    rand_ = RandomGeneratorGaussianF(0.0f, 1.0f);

    if (std::isnan(initial_output))
      initial_output_ = getNextRandom() * sigma_;
    else
      initial_output_ = initial_output;

    Reset();
  }

  void Reset() {
    output_ = initial_output_;
    rand_.reset();
  }

  void Update(TimeSec dt) {
    /*
    Ref:
        A Comparison between Different Error Modeling of MEMS Applied to GPS/INS
    Integrated Systems Quinchia, sec 3.2,
    https://www.ncbi.nlm.nih.gov/pmc/articles/PMC3812568/

        A Study of the Effects of Stochastic Inertial Sensor Errors in
    Dead-Reckoning Navigation John H Wall, 2007, eq 2.5, pg 13,
    http://etd.auburn.edu/handle/10415/945
    */

    double alpha = exp(-dt / tau_);
    output_ = static_cast<float>(alpha * output_ +
                                 (1 - alpha) * getNextRandom() * sigma_);
  }

  float getNextRandom() { return rand_.next(); }

  float getOutput() const { return output_; }

 private:
  RandomGeneratorGaussianF rand_;
  float tau_, sigma_;
  float output_, initial_output_;
  TimeNano last_time_;  // TODO this seems to not be used
};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // CORE_SIM_INCLUDE_CORE_SIM_SENSORS_NOISE_MODEL_UTILS_HPP_

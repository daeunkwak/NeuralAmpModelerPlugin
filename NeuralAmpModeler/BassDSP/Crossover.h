#pragma once

#include <algorithm>
#include <cmath>

namespace bass_nam
{

// Two cascaded Butterworth sections per path form a fourth-order Linkwitz-Riley
// crossover. Separate states are essential: clean and wet contain different audio.
class Crossover
{
public:
  void Reset(double sampleRate, double frequency = 150.0, bool enabled = false)
  {
    mSampleRate = std::isfinite(sampleRate) && sampleRate > 0.0 ? sampleRate : 48000.0;
    mSmoothing = -std::expm1(-1.0 / (0.01 * mSampleRate));
    mModeStep = 1.0 / std::max(1.0, 0.01 * mSampleRate);
    SetFrequency(frequency);
    mG = mTargetG;
    mMode = enabled ? 1.0 : 0.0;
    Clear();
  }

  void SetFrequency(double frequency)
  {
    if (!std::isfinite(frequency))
      frequency = 150.0;
    frequency = std::clamp(frequency, std::min(60.0, 0.45 * mSampleRate), std::min(500.0, 0.45 * mSampleRate));
    mTargetG = std::tan(3.14159265358979323846 * frequency / mSampleRate);
  }

  void Clear()
  {
    for (auto& section : mLow)
      section = {};
    for (auto& section : mHigh)
      section = {};
  }

  // Keep filters running while bypassed so enabling does not expose stale state.
  // The mode ramp has an exact endpoint, preserving the original bypass result.
  double Process(double clean, double wet, double cleanGain, double wetGain, bool enabled)
  {
    mG += mSmoothing * (mTargetG - mG);
    const double a1 = 1.0 / (1.0 + mG * (mG + kButterworthQInverse));
    const double a2 = mG * a1;
    const double a3 = mG * a2;
    double low = clean;
    double high = wet;
    for (auto& section : mLow)
      low = section.Process(low, a1, a2, a3, false);
    for (auto& section : mHigh)
      high = section.Process(high, a1, a2, a3, true);

    mMode = enabled ? std::min(1.0, mMode + mModeStep) : std::max(0.0, mMode - mModeStep);
    const double fullBand = cleanGain * clean + wetGain * wet;
    if (mMode == 0.0)
      return fullBand;
    const double split = cleanGain * low + wetGain * high;
    if (mMode == 1.0)
      return split;
    return fullBand + mMode * (split - fullBand);
  }

private:
  static constexpr double kButterworthQInverse = 1.4142135623730950488;
  struct Section
  {
    double s1 = 0.0;
    double s2 = 0.0;
    double Process(double input, double a1, double a2, double a3, bool highPass)
    {
      const double v3 = input - s2;
      const double v1 = a1 * s1 + a2 * v3;
      const double v2 = s2 + a2 * s1 + a3 * v3;
      s1 = 2.0 * v1 - s1;
      s2 = 2.0 * v2 - s2;
      return highPass ? input - kButterworthQInverse * v1 - v2 : v2;
    }
  };
  Section mLow[2];
  Section mHigh[2];
  double mSampleRate = 48000.0;
  double mSmoothing = 1.0;
  double mModeStep = 1.0;
  double mG = 0.0;
  double mTargetG = 0.0;
  double mMode = 0.0;
};

} // namespace bass_nam

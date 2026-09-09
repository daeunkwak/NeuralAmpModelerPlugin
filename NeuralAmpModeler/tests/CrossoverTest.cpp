#include "../BassDSP/Crossover.h"

#include <cassert>
#include <cmath>
#include <limits>

constexpr double pi = 3.14159265358979323846;

double Response(double sampleRate, double cutoff, double frequency, double lowGain, double highGain)
{
  bass_nam::Crossover crossover;
  crossover.Reset(sampleRate, cutoff, true);
  double inputEnergy = 0.0;
  double outputEnergy = 0.0;
  for (int i = 0; i < static_cast<int>(sampleRate * 2); ++i)
  {
    const double input = std::sin(2.0 * pi * frequency * i / sampleRate);
    const double output = crossover.Process(input, input, lowGain, highGain, true);
    if (i >= sampleRate)
    {
      inputEnergy += input * input;
      outputEnergy += output * output;
    }
  }
  return std::sqrt(outputEnergy / inputEnergy);
}

int main()
{
  for (double rate : {44100.0, 48000.0, 96000.0, 192000.0})
  {
    for (double cutoff : {60.0, 150.0, 500.0})
    {
      // Each branch is -6 dB at crossover; the unity-weight sum is flat in
      // magnitude (an all-pass phase response, not a sample-identical signal).
      assert(std::abs(Response(rate, cutoff, cutoff, 1.0, 0.0) - 0.5) < 0.001);
      assert(std::abs(Response(rate, cutoff, cutoff, 0.0, 1.0) - 0.5) < 0.001);
      for (double frequency : {30.0, 150.0, 1000.0, 5000.0})
        assert(std::abs(Response(rate, cutoff, frequency, 1.0, 1.0) - 1.0) < 0.002);
      assert(Response(rate, cutoff, cutoff * 0.1, 0.0, 1.0) < 0.001);
      assert(Response(rate, cutoff, cutoff * 10.0, 1.0, 0.0) < 0.001);
    }

    bass_nam::Crossover crossover;
    crossover.Reset(rate, 150.0, true);
    for (int i = 0; i < 2000; ++i)
      assert(crossover.Process(1.0, 0.0, 0.0, 1.0, true) == 0.0);
    crossover.Reset(rate, 150.0, true);
    for (int i = 0; i < 2000; ++i)
      assert(crossover.Process(0.0, 1.0, 1.0, 0.0, true) == 0.0);

    // On a steady signal, toggling split must ramp rather than make a step.
    crossover.Reset(rate);
    for (int i = 0; i < static_cast<int>(rate); ++i)
      crossover.Process(1.0, 1.0, 0.5, 0.5, false);
    double previous = 1.0;
    for (int i = 0; i < static_cast<int>(rate / 10); ++i)
    {
      const double output = crossover.Process(1.0, 1.0, 0.5, 0.5, true);
      assert(std::abs(output - previous) < 0.002);
      previous = output;
    }
    assert(std::abs(previous - 0.5) < 0.00001);

    crossover.Reset(rate);
    for (int i = 0; i < 10000; ++i)
    {
      const double clean = std::sin(i * 0.01);
      const double wet = std::cos(i * 0.03);
      // Exact original arithmetic while OFF, including model output gain.
      assert(crossover.Process(clean, wet, 0.3, 0.7 * 1.2, false) == 0.3 * clean + 0.7 * 1.2 * wet);
    }

    // Abrupt automation, then return to an exact bypass endpoint.
    for (int i = 0; i < 100000; ++i)
    {
      if (i % 64 == 0)
        crossover.SetFrequency(i % 128 == 0 ? 60.0 : 500.0);
      const double output = crossover.Process(std::sin(i * 0.01), std::cos(i * 0.02), 0.5, 0.5, i % 1000 < 500);
      assert(std::isfinite(output));
      assert(std::abs(output) < 3.0);
    }
    for (int i = 0; i < static_cast<int>(rate); ++i)
      crossover.Process(0.25, -0.5, 0.3, 0.7, false);
    assert(crossover.Process(0.25, -0.5, 0.3, 0.7, false) == 0.3 * 0.25 + 0.7 * -0.5);

    crossover.Clear();
    for (int i = 0; i < 2000; ++i)
      assert(crossover.Process(0.0, 0.0, 0.5, 0.5, true) == 0.0);
    crossover.Reset(rate, 150.0, true);
    crossover.SetFrequency(std::numeric_limits<double>::quiet_NaN());
    assert(std::isfinite(crossover.Process(1.0, 1.0, 0.5, 0.5, true)));
  }
}

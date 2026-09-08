#include "../BassDSP/SampleDelay.h"

#include <cassert>

int main()
{
  bass_nam::SampleDelay<double> delay;

  // A zero-sample delay is a transparent pass-through.
  assert(delay.Process(1.0) == 1.0);
  assert(delay.Process(-0.5) == -0.5);

  // The ring buffer preserves its state across processing calls.
  delay.SetDelaySamples(3);
  assert(delay.GetDelaySamples() == 3);
  assert(delay.Process(1.0) == 0.0);
  assert(delay.Process(2.0) == 0.0);
  assert(delay.Process(3.0) == 0.0);
  assert(delay.Process(4.0) == 1.0);
  assert(delay.Process(5.0) == 2.0);

  // Reconfiguration clears audio from the previous model or sample rate.
  delay.SetDelaySamples(3);
  assert(delay.Process(9.0) == 0.0);
  delay.SetDelaySamples(1);
  assert(delay.Process(7.0) == 0.0);
  assert(delay.Process(8.0) == 7.0);

  delay.SetDelaySamples(0);
  assert(delay.Process(6.0) == 6.0);
}

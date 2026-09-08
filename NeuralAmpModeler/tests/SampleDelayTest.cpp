#include "../BassDSP/SampleDelay.h"

#include <cassert>

int main()
{
  bass_nam::SampleDelay<double> delay;

  // A zero-sample delay is a transparent pass-through.
  assert(delay.Process(1.0) == 1.0);
  assert(delay.Process(-0.5) == -0.5);

  // The ring buffer preserves its state across processing calls.
  bool configured = delay.SetDelaySamples(3);
  assert(configured);
  assert(delay.GetDelaySamples() == 3);
  assert(delay.Process(1.0) == 0.0);
  assert(delay.Process(2.0) == 0.0);
  assert(delay.Process(3.0) == 0.0);
  assert(delay.Process(4.0) == 1.0);
  assert(delay.Process(5.0) == 2.0);

  delay.Clear();
  assert(delay.Process(6.0) == 0.0);

  // Reconfiguration clears audio from the previous model or sample rate.
  configured = delay.SetDelaySamples(3);
  assert(configured);
  assert(delay.Process(9.0) == 0.0);
  configured = delay.SetDelaySamples(1);
  assert(configured);
  assert(delay.Process(7.0) == 0.0);
  assert(delay.Process(8.0) == 7.0);

  configured = delay.SetDelaySamples(0);
  assert(configured);
  assert(delay.Process(6.0) == 6.0);

  // Oversized requests are reported and safely clamped without allocating.
  bass_nam::SampleDelay<double, 2> smallDelay;
  assert(smallDelay.GetCapacity() == 2);
  configured = smallDelay.SetDelaySamples(3);
  assert(!configured);
  assert(smallDelay.GetDelaySamples() == 2);
  assert(smallDelay.Process(1.0) == 0.0);
  assert(smallDelay.Process(2.0) == 0.0);
  assert(smallDelay.Process(3.0) == 1.0);
}

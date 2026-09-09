# Bass DSP checks

Before running the tests in a fresh clone, initialize the submodules from the
repository root. The state compatibility test requires iPlug2 and the NAM Core
dependencies:

```sh
git submodule update --init --recursive
```

Run from the repository root (Clang with AddressSanitizer/UndefinedBehaviorSanitizer):

```sh
clang++ -std=c++17 -O1 -Wall -Wextra -pedantic -fsanitize=address,undefined NeuralAmpModeler/tests/CrossoverTest.cpp -o /tmp/namp-crossover-test
/tmp/namp-crossover-test
clang++ -std=c++17 -O1 -fsanitize=address,undefined NeuralAmpModeler/tests/SampleDelayTest.cpp -o /tmp/namp-delay-test
/tmp/namp-delay-test
clang++ -std=c++17 -O1 -fsanitize=address,undefined NeuralAmpModeler/tests/CrossoverStateTest.cpp iPlug2/IPlug/IPlugParameter.cpp -I iPlug2/IPlug -I iPlug2/WDL -I NeuralAmpModelerCore/Dependencies/nlohmann -o /tmp/namp-crossover-state-test
/tmp/namp-crossover-state-test
```

The crossover tests cover magnitude response, independent clean/wet inputs,
exact bypass arithmetic, automation stability, reset, and 44.1/48/96/192 kHz.
The state test runs the production reader with real iPlug byte chunks and
parameters, substituting only the host and model/IR loader. It checks 0.8.2
restoration and defaults when loading older states into a configured instance.
These checks do not replace plugin-host or UI testing.

## Listening and host checks

- Start with Split OFF. Existing full-band Blend behavior should be unchanged.
- Turn Split ON and start at Blend 50%, Split Hz 150. Clean is low-passed after
  latency compensation; wet is high-passed after NAM, EQ and IR. Filters are LR4.
- Blend still controls the linear clean/wet balance: 0% is clean low only, 100%
  is wet high only, and 50% applies 0.5 gain to each band. Adjust Output as needed;
  no automatic makeup gain is applied.
- The cutoff spans 60–500 Hz with logarithmic control. Frequency changes are
  smoothed; Split changes crossfade over 10 ms. The knob is disabled when OFF.
- A flat LR4 magnitude sum applies to identical aligned input signals at equal
  unity gains. NAM/EQ/IR change the wet phase and spectrum, so real model blends
  are not guaranteed to sum flat. This is not automatic phase alignment.
- Automate Split, frequency and Blend; change the host sample rate and model;
  check for clicks, stale audio and unexpected level changes.
- Save/reopen a project with Split ON and a non-default cutoff. Load a pre-0.8.2
  preset afterward: Split should revert to OFF and cutoff to 150 Hz.
- Open/reopen the editor and verify the Split control and disabled knob state,
  including after host automation and preset recall.

The 0.8.2 state appends two parameters without renumbering existing IDs. Older
fork readers ignore the appended values; they cannot reproduce the split tone.

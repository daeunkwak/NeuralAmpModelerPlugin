#pragma once

#include <array>
#include <cstddef>

namespace bass_nam
{

// A fixed, integer-sample delay used to align parallel signal paths.
template <typename T, size_t Capacity = 4096>
class SampleDelay
{
public:
  // This only updates scalar state, so it is safe to call from the audio thread.
  // Returns false when the requested delay exceeds the fixed buffer capacity.
  bool SetDelaySamples(const size_t delaySamples)
  {
    const bool withinCapacity = delaySamples <= Capacity;
    mDelaySamples = withinCapacity ? delaySamples : Capacity;
    mWriteIndex = 0;
    mSamplesUntilReady = mDelaySamples;
    return withinCapacity;
  }

  void Clear()
  {
    mWriteIndex = 0;
    // Suppress reads until every active slot has been overwritten with new audio.
    mSamplesUntilReady = mDelaySamples;
  }

  T Process(const T input)
  {
    if (mDelaySamples == 0)
      return input;

    T output{};
    if (mSamplesUntilReady == 0)
      output = mBuffer[mWriteIndex];
    else
      --mSamplesUntilReady;

    mBuffer[mWriteIndex] = input;
    if (++mWriteIndex == mDelaySamples)
      mWriteIndex = 0;
    return output;
  }

  size_t GetDelaySamples() const { return mDelaySamples; }
  static constexpr size_t GetCapacity() { return Capacity; }

private:
  std::array<T, Capacity> mBuffer{};
  size_t mDelaySamples = 0;
  size_t mWriteIndex = 0;
  size_t mSamplesUntilReady = 0;
};

} // namespace bass_nam

#pragma once

#include <algorithm>
#include <cstddef>
#include <vector>

namespace bass_nam
{

// A fixed, integer-sample delay used to align parallel signal paths.
template <typename T>
class SampleDelay
{
public:
  void SetDelaySamples(const size_t delaySamples)
  {
    mBuffer.resize(delaySamples);
    Clear();
  }

  void Clear()
  {
    std::fill(mBuffer.begin(), mBuffer.end(), T{});
    mWriteIndex = 0;
  }

  T Process(const T input)
  {
    if (mBuffer.empty())
      return input;

    const T output = mBuffer[mWriteIndex];
    mBuffer[mWriteIndex] = input;
    mWriteIndex = (mWriteIndex + 1) % mBuffer.size();
    return output;
  }

  size_t GetDelaySamples() const { return mBuffer.size(); }

private:
  std::vector<T> mBuffer;
  size_t mWriteIndex = 0;
};

} // namespace bass_nam

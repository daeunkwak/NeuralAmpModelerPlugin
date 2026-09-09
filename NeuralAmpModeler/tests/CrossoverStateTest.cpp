// Exercise the production state reader using real iPlug chunks and parameters.
// Only the plugin host/model loader is replaced, so this runs without a DAW.
#include "IPlugStructs.h"
#include "IPlugParameter.h"
#include "json.hpp"

#include <array>
#include <cassert>
#include <sstream>
#include <unordered_map>

using namespace iplug;
constexpr int kNumParams = 16;
const std::string kCalibrateInputParamName = "CalibrateInput";
const bool kDefaultCalibrateInput = false;
const std::string kInputCalibrationLevelParamName = "InputCalibrationLevel";
const double kDefaultInputCalibrationLevel = 12.0;

class NeuralAmpModeler
{
public:
  std::array<IParam, kNumParams> params;
  WDL_String mNAMPath, mIRPath;
  std::string stagedModel, stagedIR;
  IParam* GetParam(int i) { return &params[i]; }
  void OnParamReset(EParamSource) {}
  void _StageModel(const WDL_String& path) { stagedModel = path.Get(); }
  void _StageIR(const WDL_String& path) { stagedIR = path.Get(); }
  void _UnserializeApplyConfig(nlohmann::json& config);
  int _UnserializeStateWithKnownVersion(const IByteChunk& chunk, int startPos);
  int _UnserializeStateWithUnknownVersion(const IByteChunk& chunk, int startPos);

  NeuralAmpModeler()
  {
    const char* names[] = {"Input",
                           "Threshold",
                           "Bass",
                           "Middle",
                           "Treble",
                           "Output",
                           "NoiseGateActive",
                           "ToneStack",
                           "IRToggle",
                           "CalibrateInput",
                           "InputCalibrationLevel",
                           "OutputMode",
                           "Slim",
                           "Blend",
                           "CrossoverEnabled",
                           "CrossoverFrequency"};
    for (int i = 0; i < kNumParams; ++i)
      params[i].InitDouble(names[i], 0.0, -1000.0, 1000.0, 0.01);
  }
};

#include "../Unserialization.cpp"

IByteChunk State(const char* version, int count)
{
  IByteChunk chunk;
  chunk.PutStr(version);
  chunk.PutStr("test.nam");
  chunk.PutStr("test.wav");
  for (int i = 0; i < count; ++i)
  {
    const double value = i == 13 ? 37.0 : i == 14 ? 1.0 : i == 15 ? 230.0 : static_cast<double>(i);
    chunk.Put(&value);
  }
  return chunk;
}

int main()
{
  NeuralAmpModeler plugin;
  auto current = State("0.8.2", 16);
  int pos = plugin._UnserializeStateWithKnownVersion(current, 0);
  assert(pos == current.Size());
  assert(plugin.params[13].Value() == 37.0);
  assert(plugin.params[14].Value() == 1.0);
  assert(plugin.params[15].Value() == 230.0);
  assert(plugin.stagedModel == "test.nam" && plugin.stagedIR == "test.wav");

  // Loading an older state into an already-enabled instance must disable split.
  auto legacy = State("0.8.1", 14);
  pos = plugin._UnserializeStateWithKnownVersion(legacy, 0);
  assert(pos == legacy.Size());
  assert(plugin.params[13].Value() == 37.0);
  assert(plugin.params[14].Value() == 0.0);
  assert(plugin.params[15].Value() == 150.0);

  auto preBlend = State("0.7.14", 13);
  pos = plugin._UnserializeStateWithKnownVersion(preBlend, 0);
  assert(pos == preBlend.Size());
  assert(plugin.params[13].Value() == 100.0);
  assert(plugin.params[14].Value() == 0.0);
  assert(plugin.params[15].Value() == 150.0);
}

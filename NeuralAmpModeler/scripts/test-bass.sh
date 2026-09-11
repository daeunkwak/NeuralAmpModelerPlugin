#!/bin/bash
set -euo pipefail

repo=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
cd "$repo"
cxx=${CXX:-clang++}
command -v "$cxx" >/dev/null || { echo "C++ compiler not found: $cxx" >&2; exit 1; }
for dependency in iPlug2/IPlug/IPlugParameter.cpp NeuralAmpModelerCore/Dependencies/nlohmann/json.hpp; do
  if [[ ! -f "$dependency" ]]; then
    echo "Missing $dependency. Run: git submodule update --init --recursive" >&2
    exit 1
  fi
done
test_dir=$(mktemp -d "${TMPDIR:-/tmp}/bassnam-tests.XXXXXX")
# Keep binaries for debugging; never remove a user-provided directory.
echo "Test binaries: $test_dir"
flags=(-std=c++17 -O1 -g -Wall -Wextra -fsanitize=address,undefined)
for test in SampleDelay Crossover; do
  "$cxx" "${flags[@]}" "NeuralAmpModeler/tests/${test}Test.cpp" -o "$test_dir/$test"
  "$test_dir/$test"
  echo "PASS: $test"
done
"$cxx" "${flags[@]}" NeuralAmpModeler/tests/CrossoverStateTest.cpp \
  iPlug2/IPlug/IPlugParameter.cpp -I iPlug2/IPlug -I iPlug2/WDL \
  -I NeuralAmpModelerCore/Dependencies/nlohmann -o "$test_dir/CrossoverState"
"$test_dir/CrossoverState"
echo "PASS: state compatibility"
python3 NeuralAmpModeler/tests/test_mac_identity.py

#!/bin/bash
set -euo pipefail

repo=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
target=${1:-APP}
case "$target" in
  APP|VST3|AU) ;;
  --help)
    echo "Usage: bash NeuralAmpModeler/scripts/build-local-mac.sh [APP|VST3|AU]"
    echo "Builds an ad-hoc signed local bundle in build-local/products; does not install it."
    exit 0 ;;
  *) echo "Unsupported target: $target (use APP, VST3 or AU)" >&2; exit 2 ;;
esac
if [[ $(uname -s) != Darwin ]]; then
  echo "This script requires macOS." >&2
  exit 1
fi
# Respect an explicit toolchain. Otherwise find a full Xcode when the system
# selection still points to Command Line Tools, without changing global settings.
if [[ -z "${DEVELOPER_DIR:-}" ]] && ! xcodebuild -version >/dev/null 2>&1; then
  for candidate in /Applications/Xcode.app "$HOME/Downloads/Xcode.app"; do
    if [[ -x "$candidate/Contents/Developer/usr/bin/xcodebuild" ]]; then
      export DEVELOPER_DIR="$candidate/Contents/Developer"
      break
    fi
  done
fi
if ! xcodebuild -version >/dev/null 2>&1; then
  echo "Full Xcode is required; Command Line Tools alone cannot build the GUI." >&2
  echo "Install and open Xcode to finish setup, then run with:" >&2
  echo "DEVELOPER_DIR=/Applications/Xcode.app/Contents/Developer bash NeuralAmpModeler/scripts/build-local-mac.sh $target" >&2
  exit 1
fi
for tool in ibtool actool metal; do
  if ! xcrun --find "$tool" >/dev/null 2>&1; then
    echo "Xcode tool missing: $tool. Complete Xcode's component setup first." >&2
    exit 1
  fi
done
for dependency in iPlug2/IPlug/IPlugPluginBase.h NeuralAmpModelerCore/NAM/dsp.h eigen/Eigen/Dense \
  AudioDSPTools/dsp/dsp.h iPlug2/Dependencies/IGraphics/NanoVG/src/nanovg.h; do
  if [[ ! -f "$repo/$dependency" ]]; then
    echo "Missing $dependency. Run: git submodule update --init --recursive" >&2
    exit 1
  fi
done
if [[ "$target" == VST3 && ! -f "$repo/iPlug2/Dependencies/IPlug/VST3_SDK/pluginterfaces/base/funknown.h" ]]; then
  echo "VST3 SDK missing. Follow iPlug2/Dependencies/IPlug/README.md to install the SDK." >&2
  exit 1
fi
build_dir="$repo/build-local"
mkdir -p "$build_dir/products"
cd "$repo/NeuralAmpModeler/projects"
export BASSNAM_LOCAL_BUILD=1
xcodebuild -project NeuralAmpModeler-macOS.xcodeproj -target "$target" -configuration Debug \
  "SYMROOT=$build_dir/objects" "OBJROOT=$build_dir/intermediates" \
  "CONFIGURATION_BUILD_DIR=$build_dir/products" DEPLOYMENT_LOCATION=NO \
  CODE_SIGNING_ALLOWED=NO CODE_SIGN_IDENTITY= DEVELOPMENT_TEAM= \
  ENABLE_HARDENED_RUNTIME=NO ENABLE_USER_SCRIPT_SANDBOXING=NO \
  "ARCHS=$(uname -m)" ONLY_ACTIVE_ARCH=YES build
case "$target" in
  APP) extension=app ;;
  VST3) extension=vst3 ;;
  AU) extension=component ;;
esac
bundle="$build_dir/products/BassNAM.$extension"
[[ -d "$bundle" ]] || { echo "Expected product missing: $bundle" >&2; exit 1; }
codesign --force --deep --sign - "$bundle"
codesign --verify --deep --strict "$bundle"
echo "Built: $bundle"
if [[ "$target" == APP ]]; then
  echo "Launch: open \"$bundle\""
else
  echo "See NeuralAmpModeler/LOCAL_TESTING.md for user-level installation and DAW checks."
fi

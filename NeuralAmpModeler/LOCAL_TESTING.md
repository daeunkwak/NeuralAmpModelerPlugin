# BassNAM: macOS에서 직접 실행하기

이 fork의 제품명은 **BassNAM**, 제작자 표시는 **daeunkwak**입니다.
원본 NAM의 라이선스와 저작권 표시는 유지합니다.
이번 실행 경로는 macOS standalone(APP), VST3, AUv2를 대상으로 합니다.
기존 Windows/iOS/AUv3/AAX 배포 스크립트는 이 fork의 검증된 배포 경로가 아닙니다.

## 준비

저장소 루트에서 실행합니다.

```sh
git submodule update --init --recursive
bash NeuralAmpModeler/scripts/test-bass.sh
```

테스트는 Python 3, C++17 컴파일러와 ASan/UBSan이 필요하며, Mac에서는 Command Line Tools로
실행할 수 있습니다. 임시 테스트 바이너리 경로가 출력됩니다.

GUI 빌드는 **Xcode 전체 앱**이 필요합니다. Xcode를 설치하고 한 번 열어 라이선스와
필수 컴포넌트 설치를 마치세요. Command Line Tools만으로는 앱을 만들 수 없습니다.
시스템이 Command Line Tools를 가리키면 빌드 스크립트는 `/Applications/Xcode.app`,
`~/Downloads/Xcode.app` 순서로 전체 Xcode를 찾습니다. 전역 개발 도구 설정은 바꾸지 않습니다.
기본 개발 도구가 Command Line Tools를 가리키는 경우, 시스템 설정을 바꾸지 않고
현재 터미널에서 다음과 같이 지정할 수 있습니다.

```sh
export DEVELOPER_DIR=/Applications/Xcode.app/Contents/Developer
```

VST3 빌드에는 별도의 VST3 SDK도 필요합니다.
[iPlug2 SDK 안내](../iPlug2/Dependencies/IPlug/README.md)에 따라
`iPlug2/Dependencies/IPlug/VST3_SDK`에 설치하세요. APP부터 시작하면 이 SDK는 필요 없습니다.

## 빌드와 실행

```sh
bash NeuralAmpModeler/scripts/build-local-mac.sh APP
open build-local/products/BassNAM.app
```

빌드는 현재 Mac 아키텍처의 Debug 앱을 `build-local/products`에 생성하고 로컬용
ad-hoc 서명을 적용합니다. Apple 개발자 계정은 사용하지 않으며, 배포용 공증이나
universal 빌드가 아닙니다. 서명은 이 스크립트가 생성한 결과물에만 적용합니다.
앱과 플러그인을 자동 설치하거나 기존 NAM을 덮어쓰지 않습니다.

앱의 제목은 BASS NAM이며, 모델·IR 로더와 Blend, Split, Split Hz 컨트롤이 표시됩니다.
오디오 입력 권한을 요청하면 허용하세요. 거부했다면 macOS의 마이크 개인정보 설정에서
BassNAM의 접근을 허용한 뒤 재실행하세요.

## Scarlett 연결

1. 베이스를 Scarlett의 악기 입력에 연결하고 해당 입력을 Instrument 모드로 설정합니다.
2. 헤드폰이나 모니터는 Scarlett 출력에 연결하고, 낮은 출력 음량에서 시작합니다.
3. 앱 오디오 설정에서 입력·출력 장치를 Scarlett으로 선택하고 실제 연결한 입력 채널과
   출력 1/2를 지정합니다. 우선 48 kHz, 버퍼 128 또는 256 samples로 시작합니다.
4. 처리된 소리만 비교할 때는 Scarlett의 Direct Monitor를 끕니다.
5. `.nam` 모델과 선택적으로 IR `.wav`를 불러옵니다. macOS standalone에서는
   파일 대신 해당 파일이 들어 있는 폴더를 선택하는 브라우저가 표시될 수 있습니다.
6. Split OFF에서 Blend 0/50/100%를 비교하고, Split ON·150 Hz·Blend 50%로
   저역/고역 혼합을 확인합니다. 입력/출력 미터가 클리핑되지 않도록 조절합니다.

Split ON에서 Blend 0%는 Clean 저역만, 100%는 Wet 고역만 출력합니다.
50%는 각 대역에 절반 게인을 적용하며 자동 음량 보정은 없습니다.

## DAW 설치와 테스트

DAW가 지원하는 형식을 선택합니다. Logic은 AU, VST3 호스트는 VST3를 사용합니다.

```sh
bash NeuralAmpModeler/scripts/build-local-mac.sh VST3
# 또는
bash NeuralAmpModeler/scripts/build-local-mac.sh AU
```

Finder에서 생성된 `BassNAM.vst3`를 사용자 Library의 `Audio/Plug-Ins/VST3`에,
`BassNAM.component`를 `Audio/Plug-Ins/Components`에 복사합니다.
대상 폴더에 같은 이름의 이전 BassNAM이 있다면 먼저 별도로 보관하세요.
원본 `NeuralAmpModeler` 파일은 교체할 필요가 없습니다.
DAW를 재실행하거나 플러그인을 다시 스캔하고 **daeunkwak / BassNAM**을 선택합니다.
이 로컬 빌드 절차는 시스템 AU 캐시를 삭제하지 않습니다.

- 원본 NAM과 BassNAM을 별도 인스턴스로 열 수 있는지 확인합니다.
- Split/Blend를 조절하고 모델을 교체하며 클릭·드롭아웃·UI 겹침을 확인합니다.
- Split ON, cutoff 230 Hz, Blend 37%로 DAW 프로젝트를 저장한 뒤 다시 열어 복원을 확인합니다.
- 44.1/48/96 kHz에서 오디오와 UI가 정상 동작하는지 확인합니다.
- AU 설치 후 선택적으로 `auval -v aufx BsN1 Dkwk`를 실행합니다.

## 식별자와 저장 위치

| 항목 | BassNAM |
|---|---|
| C++ 클래스 / 번들 이름 | `BassNAM` |
| Plugin / Manufacturer ID | `BsN1` / `Dkwk` |
| App Bundle ID | `com.daeunkwak.app.BassNAM` |
| VST3 Bundle ID | `com.daeunkwak.vst3.BassNAM` |
| AU Bundle ID | `com.daeunkwak.audiounit.BassNAM` |
| 로컬 앱 설정 | `~/Library/Application Support/BassNAM/settings.ini` |
| iPlug VST3 사용자 프리셋 경로 | `~/Library/Audio/Presets/daeunkwak/BassNAM/` |

이 스크립트의 앱은 sandbox entitlements 없이 로컬 서명됩니다. 별도로 sandbox 빌드를
만들면 설정 경로는 해당 앱 컨테이너 아래에 위치할 수 있습니다. DAW의 자체 프리셋과
프로젝트 저장 위치는 호스트가 관리합니다.

제품 ID가 바뀌었으므로 기존 NAM 인스턴스를 자동으로 대체하지 않습니다. 이전 fork의
저장 데이터 형식과 파라미터 순서는 유지하지만, 이전 ID로 저장한 DAW 프로젝트를
열려면 이전 바이너리가 필요할 수 있습니다. 필요한 설정은 별도로 내보내고 옮기세요.

## 검증 상태

2026-09-10: macOS 15.7.2 / Apple Silicon / Xcode 26.3에서 APP Debug 빌드와
ad-hoc 서명 검증, 실제 앱 실행을 확인했습니다. 700px UI에 맞춰 배경을 수정했고,
Split ON/OFF에 따른 노브 활성화, 주파수 변경 및 기본값 복원을 확인했습니다.
DSP/상태 테스트와 제품 메타데이터 검사도 별도로 제공합니다.
VST3/AU 빌드·호스트 검증, Scarlett 청음, DAW 저장·재실행은 아직 완료하지 않았습니다.

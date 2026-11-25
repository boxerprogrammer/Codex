# 開発環境と依存ライブラリ

最終調査日 2026-11-25。M00で検証したツール構成と今後の依存導入方針を記録する。第三者ライブラリの取得・組み込みとGPU描画の検証は各導入段階で行う。

## 目次

- [1. 基本方針](#1-基本方針)
- [2. 環境の記録](#2-環境の記録)
- [3. 導入する順序](#3-導入する順序)
- [4. バージョンの固定と取得方式](#4-バージョンの固定と取得方式)
- [5. ライブラリ別の境界条件](#5-ライブラリ別の境界条件)
- [6. 依存関係の記録テンプレート](#6-依存関係の記録テンプレート)
- [7. 一次資料](#7-一次資料)

## 1. 基本方針

1. Windows x64 / MSVC / C++20 / DirectX 12を標準の実行環境とする。学校で使用するVisual Studio 2022以降の具体的な版を確認し、授業の基準環境を一つ決める。
2. バージョン番号は、インターネットで見つけた最新番号をそのまま採用しない。採用するヘッダ・ライブラリ・サンプル・ドキュメントの組み合わせを固定する。
3. M00は環境の記録とCMakeの土台まで。ImGui・Lua・Effekseer等を一括取得しない。各機能を学ぶマイルストーンで一つずつ導入する。
4. 第三者APIを推測して書かない。固定した版のヘッダと公式サンプルを確認してから、エンジン側の小さな接続クラスを実装する。
5. 導入の成功は「ダウンロードできた」「ヘッダが読めた」ではなく、該当する小さなWindowsサンプルがビルド・実行できた証跡で判定する。[検証方針](VALIDATION.md)を参照する。
6. ライセンス本文、配布に必要な表示、同梱するバイナリや素材の条件を取得元とともに記録する。SDK本体を学生や公開リポジトリへ再配布できると推測しない。

本書の選択は教育上の設計提案である。APIの仕様・対応状況については各節の一次資料を根拠とする。

## 2. 環境の記録

### 2.1 M00で採取する情報

| 項目 | 記録内容 | 用途 |
| --- | --- | --- |
| OS | Windowsの版、ビルド、x64 | 再現環境の特定 |
| Visual Studio | 製品名、版、選択したMSVC toolset | IDEの版とコンパイラの版を区別する |
| C++ | `/std:c++20`、例外・警告・ランタイム設定 | C++23以降の機能混入とABI不一致を防ぐ |
| Windows SDK | 実際に使用するSDKの版と探索結果 | D3D12、DXGI、XAudio2のヘッダとライブラリを揃える |
| CMake | 版、generator、preset schema | 教員PCと学生PCで同じ構成を生成する |
| ビルドツール | MSBuildまたはNinjaの版 | presetと実際のビルド経路を一致させる |
| GPU | アダプタ名、ドライバ版 | 機能確認と不具合の再現 |
| Graphics Tools | Debug Layerの利用可否 | 描画APIの使い方を検査する |
| Git | 版、既存リポジトリと変更の有無 | 既存作業を保ち、依存版を固定する |

MSVCのC++20モードは`/std:c++20`で指定できる。CMake実装時にはターゲットにC++20を要求し、生成されたコンパイル設定も確認する。`/std:c++latest`を教材の標準にしない。[MSVC言語標準オプション](https://learn.microsoft.com/en-us/cpp/build/reference/std-specify-language-standard-version?view=msvc-170)

Visual Studioの「C++によるデスクトップ開発」とCMake toolsの導入状況を調べる。IDEがインストール済みでも、C++ツールやWindows SDKが揃っているとは限らない。[Visual StudioのCMake対応](https://learn.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=msvc-180)

### 2.2 CMakeの扱い

共有の設定を`CMakePresets.json`、PC固有のパス等を`CMakeUserPresets.json`に分ける。後者はバージョン管理対象にしない。presetのschemaは採用するCMake版で読めるものを選ぶ。[CMake Presets公式仕様](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html)

M00の `CMakeLists.txt` と `CMakePresets.json` は次の構成を定義する。

- configure用の `windows-x64` と、ビルド・テスト用の `windows-debug` / `windows-release`。
- どのOSでも標準C++だけを使う `edu_environment_check` ターゲット。C++20の指定漏れはコンパイル時に検出し、実行すると言語モードの値を表示する。
- WindowsでD3D12とDXGIのヘッダ・ライブラリを確認するconfigure時のチェック。SDKの描画用部分に問題がある場合、`-DEDU_CHECK_WINDOWS_SDK=OFF` でCPUターゲットだけを調べられる。MSVC自体が必要とするWindows SDKは引き続き必要。

Linux等ではWindows用プリセットの代わりに `cmake -S . -B build/cpu` でCPUターゲットを構成できる。M00で実行した環境はWindows x64。

### 2.3 M00の実測記録

| 項目 | 2026-11-25の検証環境 |
| --- | --- |
| OS | Windows x64、26H2、OSビルド 10.0.26300.10075。 `cmd.exe /d /c ver`、レジストリ、OSアーキテクチャで確認 |
| Visual Studio | Community 2022 17.14.23 (17.14.36811.4)。 `vswhere` でC++デスクトップワークロードを確認し、CMakeのキャッシュで選択されたインストール先を確認 |
| MSVC | v143、ツールセットディレクトリ 14.44.35207、コンパイラ 19.44.35222 x64。ビルドの実引数に `/std:c++20`、`/EHsc`、`/W4`、`/Zc:__cplusplus`、`/utf-8` を確認。標準ライブラリのランタイム指定はDebugが `/MDd`、Releaseが `/MD` |
| SDK | Windows SDK 10.0.26100.0。CMakeの選択結果と、`Windows.h`、`d3d12.h`、`dxgi1_6.h`、x64の `d3d12.lib` と `dxgi.lib` の実体を確認 |
| CMake / CTest | Visual Studio同梱の3.31.6-msvc6を使用。共有プリセットのスキーマは3、最低CMake版は3.21。通常のPATHにあった4.4.0-rc3は今回のビルド経路では未使用。 [CMakeのプリセット仕様](https://cmake.org/cmake/help/v3.25/manual/cmake-presets.7.html) |
| ビルドツール | MSBuild 17.14.23.42201、ジェネレーター `Visual Studio 17 2022`、ターゲット x64。同梱Ninja 1.12.1も起動を確認 |
| GPU | NVIDIA GeForce RTX 2070 SUPER、ドライバ 32.0.16.1088。 `pnputil /enum-devices /connected /class Display`（終了0）と表示アダプタのレジストリで確認 |
| Graphics Tools | M00では未確認。M02で `D3D12GetDebugInterface` が成功し、Debug Layerを有効にした |
| Git | 2.56.0.windows.4。作業元・展開先ともリポジトリがなく、`git status --short --branch` は終了128。既存ファイルは展開の前後でSHA-256を照合 |

Visual Studio同梱のCMake / CTestが入ったディレクトリを現在のPowerShellセッションの `PATH` の先頭に置き、プロジェクトルートで実行した。下表のログ名はすべて `build/logs/` 以下。詳細なツール調査ログは `environment.log`、CMakeのコンパイル・リンク調査は `build/windows-x64/CMakeFiles/CMakeConfigureLog.yaml` にある。

| コマンド | 終了 | 結果とログ |
| --- | --- | --- |
| `cmake --list-presets=all` | 0 | configure 1件、ビルドとテスト各2件。 `presets.log` |
| `cmake --fresh --preset windows-x64` | 0 | D3D12 / DXGIの関数への参照を含むコードをコンパイル・リンクし、`build/windows-x64/` に生成。 `configure-windows-x64.log` |
| `cmake --preset windows-x64` | 0 | 共有プリセットの通常手順での再configure。 `configure-preset.log` |
| `cmake --build --preset windows-debug` | 0 | Debugの確認プログラムをコンパイル・リンク。 `build-windows-debug.log` |
| `ctest --preset windows-debug -V` | 0 | 1/1件合格。 `test-windows-debug.log` |
| `./build/windows-x64/Debug/edu_environment_check.exe` | 0 | `C++ language version: 202002`。 `run-windows-debug.log` |
| `cmake --build --preset windows-release` | 0 | Releaseの確認プログラムをコンパイル・リンク。 `build-windows-release.log` |
| `ctest --preset windows-release -V` | 0 | 1/1件合格。 `test-windows-release.log` |
| `./build/windows-x64/Release/edu_environment_check.exe` | 0 | `C++ language version: 202002`。 `run-windows-release.log` |
| `cmake --preset windows-x64 -B build/cpu-only -DEDU_CHECK_WINDOWS_SDK=OFF` | 0 | 描画用SDKチェックを省いて別ディレクトリに生成。 `configure-cpu-only.log` |
| `cmake --build build/cpu-only --config Debug --target edu_environment_check` | 0 | CPUターゲットをビルド。 `build-cpu-only.log` |
| `ctest --test-dir build/cpu-only -C Debug --output-on-failure -V` | 0 | 1/1件合格。 `test-cpu-only.log` |
| `dumpbin /dependents build/cpu-only/Debug/edu_environment_check.exe` | 0 | 実行ファイルの直接依存に `d3d12.dll` / `dxgi.dll` がないことを確認。 `dependencies-cpu-only.log` |
| `cmake --preset windows-x64 -B build/sdk-check-failure -DEDU_WINDOWS_SDK_CAN_LINK=FALSE` | 1（期待値） | 検査の失敗をキャッシュ値で模擬。SDKの確認とCPUのみで検証する指定を案内してconfigure停止。 `configure-sdk-check-failure.log` |

初回ビルドでは日本語コメントの文字コード警告 C4819 が出た。MSVCに `/utf-8` を指定してビルドをやり直し、上表の各構成で警告が消えたことを確認した。通常のPATHにあったMinGWの `g++ --version` は、システムDLLの再配置エラーで起動できなかった。CIMによるOS情報取得はアクセス制限で失敗したため、OSビルドは `ver` とレジストリで取得した。

## 3. 導入する順序

| 段階 | 依存・機能 | 導入する最小範囲 | この準備パックでの状態 |
| --- | --- | --- | --- |
| M00 | MSVC、Windows SDK、CMake、Git | 環境記録とビルド構成の土台 | Windows x64で検証済み。実測版は上表 |
| M01 | Win32 / User32 | ウィンドウ、メッセージ処理 | Windows SDK 10.0.26100.0でWindows x64のDebug・Releaseを検証済み |
| M02 | D3D12 / DXGI / WRL `ComPtr` | 背景色の描画と終了処理 | Windows SDK 10.0.26100.0とNVIDIA GeForce RTX 2070 SUPERでDebug・Releaseを検証済み |
| M03 | DXC | 頂点・ピクセルシェーダのビルド | 未導入・具体版未固定 |
| M04 | 追加の外部依存なし | フレーム別のGPU作業管理 | 未実装 |
| M05 | 追加の外部依存なし | Node / Scene / Resource | 未実装 |
| M06 | DirectXMath、画像読み込み手段 | 3D変換と小さなUnlit素材サンプル | 未導入・具体版未固定 |
| M07 | 追加の外部依存なし | 小さなRenderGraph | 未実装 |
| M08 | 追加の外部依存を原則増やさない | PBRの直接光とHDR描画・表示変換 | 未実装 |
| M09 | Dear ImGui | Win32 + DX12 backend、デバッグUI | 未導入・具体版未固定 |
| M10 | Lua + sol2候補 | 安全な関数呼び出しとNode操作 | 未導入・組み合わせ未固定 |
| M11 | XAudio2 | メモリ上のPCM WAVの再生 | 未実装 |
| M12 | Effekseer + 採用版が要求する依存 | DX12の最小エフェクト1個 | 未導入・具体版未固定 |
| M13 | 各段階で固定済みの構成 | 総合サンプルと配布用教材 | 未実装 |

画像・モデル用ライブラリはM06で用途を絞って選ぶ。最初はコード生成した形状と小さな画像だけで成立させ、glTF等のモデルインポータは必要が生じた時点で追加する。3Dデータ形式の導入をNode設計の前提にしない。

## 4. バージョンの固定と取得方式

### 4.1 比較

| 方式 | 長所 | 短所 | 授業で扱う難易度 |
| --- | --- | --- | --- |
| 固定した公式ソースを`third_party/`に配置 | ソースとbackendが見え、差分を追いやすい | 更新とライセンス同梱を自分で管理する | 低〜中 |
| CMake FetchContentでcommitまたはハッシュを固定 | CMakeから再現でき、取得場所が明確 | 初回ネット接続、学校のプロキシやキャッシュ対応が必要 | 中 |
| vcpkg等でmanifestとbaselineを固定 | 複数依存の構成を管理しやすい | パッケージ機構も学習対象になり、backendや機能設定の確認が必要 | 中〜高 |

**暫定方針:** 最初は依存の少ない構成で開始し、M03以降にライブラリごとに最も説明しやすい方式を選ぶ。プロジェクト全体で取得規則を揃え、同じ依存の複数版を混在させない。方式決定を[設計判断記録](DECISIONS.md)に残す。

### 4.2 固定の条件

- `main`、`master`、`latest`を再現ビルドの取得指定にしない。ブランチのURLは調査先としてのみ使う。
- Gitは採用tagと完全なcommit ID、配布アーカイブはURLとSHA-256を記録する。tag名だけで中身が固定されると扱わない。
- x64 / Debug / Release、MSVC toolset、C/C++ランタイムの設定を揃える。ヘッダだけ最新版、バイナリは旧版という組み合わせを避ける。
- 採用版に含まれる`LICENSE`等の所在と、配布時に必要な通知を記録する。ライセンス名だけで同梱可能と判定しない。
- バージョン更新は、小さな統合サンプルと該当機能の回帰確認を行ってから反映する。教材で引用するAPIも同時に更新する。
- セキュリティ修正や互換性上の必要がある場合は、固定を理由に更新を拒まない。更新理由と確認範囲を残す。

## 5. ライブラリ別の境界条件

### 5.1 DirectX 12 / DirectXMath / DXC

DirectXMathはMicrosoftの数学ライブラリ候補とする。Windows SDK同梱分を使うか、別取得した固定版を使うかを明記し、include探索でどちらが選ばれたか確認する。数学規約は[描画設計](RENDERING.md)と一致させる。[DirectXMath公式リポジトリ](https://github.com/microsoft/DirectXMath)

DXCはHLSLをDXILへコンパイルするためのツールである。M03で導入し、まずビルド時のシェーダコンパイルを採用する。授業中の実行時コンパイルやホットリロードは後の拡張とする。[DXC公式README](https://github.com/microsoft/DirectXShaderCompiler/blob/main/README.md)

固定時には`dxc.exe`、使用する場合の`dxcompiler.dll`と`dxil.dll`、選ぶShader Model、Windows SDK、GPUドライバの組み合わせを記録する。最初は必要最小限のShader Model 6系を候補にし、動作環境の対応を調べてから確定する。新しいShader ModelやAgility SDKをPBRの前提として自動追加しない。[DXC配布と構成](https://github.com/microsoft/DirectXShaderCompiler/wiki/Releases)

### 5.2 Dear ImGui

配置先は`engine/integrations/imgui/`。この層がWin32イベント入力とDX12描画の接続を担当し、SceneやResourceにImGui型を持ち込まない。

公式DX12 backendは、版によって初期化構造体やdescriptorの割り当て・解放callbackが変わる。採用版の`imgui_impl_dx12.h`、`imgui_impl_dx12.cpp`、`example_win32_directx12`を一組として読み、古い記事の関数引数を混ぜない。[公式導入ガイド](https://github.com/ocornut/imgui/wiki/Getting-Started)、[DX12 backendヘッダ](https://github.com/ocornut/imgui/blob/master/backends/imgui_impl_dx12.h)

統合時に確認する事項:

- エンジンと共有するdevice / direct queue、使用中のフレーム数、RTV / DSV format。
- shader-visible descriptor heapの所有者、割り当てcallback、GPU使用終了後の再利用。
- backendが利用するsamplerやテクスチャ更新処理の仕様。新旧版の想定を固定しない。
- フォントや画像のGPU寿命、resize後の参照、終了時の破棄順序。
- UI描画後に必要な描画状態を次のpass側で明示的に設定すること。
- DPI、マウス・キーボード入力の取り合い、`WantCaptureMouse`等を使う入力方針。

導入初期はdockingや複数OSウィンドウを必須にしない。シーン描画後のUI合成と色空間は[描画設計](RENDERING.md)に従う。

### 5.3 Lua / sol2

配置先は`engine/integrations/lua/`。Luaのヘッダ・リンクするruntime・sol2を一組として固定する。Luaの異なるversion間ではABI互換を前提にできないため、ライブラリだけ差し替えない。[Lua公式version規則](https://www.lua.org/versions.html)

| 案 | 長所 | 短所 | 授業上の位置付け |
| --- | --- | --- | --- |
| Lua C APIを直接使う | stackと型変換の仕組みを追える | 接続コードが多く、stack操作の説明が必要 | 発展教材 |
| sol2で接続する | C++関数を少ない記述で公開できる | 依存の互換性と内部テンプレートを切り分ける必要 | 初期採用候補 |

採用候補はsol2だが、このパックでは対応するLuaの具体版を確定しない。公式Luaの最新版と、sol2の文書・固定commitが対応する範囲を別々に確認する。

実装の条件:

- スクリプトのロードはエラーを受け取れる経路、呼び出しは`sol::protected_function`等の保護付き経路を使い、結果を検査する。エラー時に対象スクリプトを停止し、ファイル・関数・メッセージを表示する。[保護付き関数呼び出し](https://sol2.readthedocs.io/en/latest/api/protected_function.html)、[stateとsafe_script](https://sol2.readthedocs.io/en/latest/api/state.html)
- LuaにNodeの長寿命な生ポインタを渡さない。世代を検査できるNodeHandleを介し、削除後の参照を失敗として返す。これは本エンジンの寿命管理方針である。
- Luaに属する参照やcallbackを先に破棄し、その後にLua stateを破棄する。[sol2の所有権](https://sol2.readthedocs.io/en/latest/tutorial/ownership.html)
- Lua呼び出し中のNode追加・削除は、Sceneの遅延変更に渡す。更新中の配列を直接変更しない。
- Luaから呼べるC++ APIは小さく始める。ファイル操作・OS操作・動的ライブラリを初期教材に必要なく公開しない。エラー捕捉をセキュリティ上の完全な隔離と説明しない。

### 5.4 サウンド: XAudio2を最初に採用する

| 案 | 長所 | 短所 | 授業で扱う難易度 |
| --- | --- | --- | --- |
| XAudio2 | Windowsの仕組みに沿い、音声バッファの寿命を学べる | WAV読み込みとvoiceの管理は自作する | 中 |
| CRI ADX LE Native SDK | 制作ツールを使った音の調整へ発展できる | SDK・ツールの追加と利用条件の確認が必要 | 中〜高 |

初期標準はXAudio2とし、短いPCM WAVをメモリに読み込んで再生する。圧縮音源、ストリーミング、3Dサウンド、voice poolingはM11の必須範囲に含めない。

Microsoftの初期化手順に従い、COM初期化と`XAudio2Create`の結果を検査する。COM初期化は成功した呼び出しに対応して同一スレッドで終了処理する。`IXAudio2`は`ComPtr`を使えるが、voiceは`Release`を持つCOMオブジェクトではない。[XAudio2初期化](https://learn.microsoft.com/en-us/windows/win32/xaudio2/how-to--initialize-xaudio2)

voiceには`DestroyVoice`を呼ぶRAIIラッパを使う。再生バッファの実データは消費完了まで保持し、callback自身の寿命もvoiceより長く保つ。破棄はcallback内から実行せず、制御する側へ通知して行う。[SubmitSourceBuffer](https://learn.microsoft.com/en-us/windows/win32/api/xaudio2/nf-xaudio2-ixaudio2sourcevoice-submitsourcebuffer)、[DestroyVoice](https://learn.microsoft.com/en-us/windows/win32/api/xaudio2/nf-xaudio2-ixaudio2voice-destroyvoice)

CRI ADX LEの公式製品ページにはNative SDKが掲載されているため、自作C++から使えないとは判断しない。ただし本プロジェクトで使用する版のSDK取得条件、利用許諾、教材・サンプル・SDKの再配布条件は未検証である。採用時に確認し、XAudio2に依存しない小さな音声APIの内側で接続する。[CRI ADX LE公式ページ](https://game.criware.jp/products/adx-le/)

### 5.5 Effekseer

配置先は`engine/integrations/effekseer/`。M12でエフェクト1個の読み込み・再生・停止から導入する。公式構成にはDirectX12系の描画に関わるLLGI層があるため、DX11用の例を名前だけ置換して流用しない。[公式構成説明](https://github.com/effekseer/Effekseer/blob/master/docs/Development/CodeOverview.md)

採用するreleaseのC++ガイド・ヘッダ・DirectX12サンプルを照合する。公式ガイドが参照するdevice管理はサンプル側のものなので、エンジンのdevice / queue / command listとどう共有するかを決める。[公式C++ガイド](https://github.com/effekseer/Effekseer/blob/master/docs/Help_Cpp/18x/Guide_Cpp_En.md)

導入時に固定版のソースで確認する事項:

- 必要なcore / renderer / LLGI等のビルド対象、MSVC設定、データ形式の対応。
- 共有device、queue、command listの開始・終了と、誰がGPUへ送信するか。
- フレーム別のメモリプールとfence完了の対応。バッファ枚数が同じという理由だけで安全としない。
- descriptor heap切り替え、描画状態、RTV / DSV format、深度・MSAA、色空間。
- 左右の座標系、単位、camera / projection行列の受け渡し。
- texture / effect / rendererの保持期間、GPU完了後の終了処理。
- RenderGraphから見た読み取り・書き込み対象と開始・終了状態。内部の描画をgraphに自動推測させない。

この調査ではDirectX12サンプルへの直接アクセスに取得制限があった。ガイド内の案内は確認済みだが、具体的な関数署名・必要CMake targetは検証済みと扱わない。M12で採用版を取得できた時点に確認し、接続サンプルを実際にビルドする。

## 6. 依存関係の記録テンプレート

各依存を採用する際、下表を複製して記録する。`未確認`を任意の値で埋めない。

| フィールド | 記入内容 |
| --- | --- |
| 名前 / 導入マイルストーン | 未確認 |
| 目的 / 使用する機能 | 未確認 |
| 公式取得元 / API根拠URL | 未確認 |
| release / tag / 完全commit ID | 未固定 |
| アーカイブURL / SHA-256 | 未確認、または該当なし |
| 取得方式 / 保存先 / CMake target | 未実装 |
| 付随依存とその固定情報 | 未確認 |
| MSVC / SDK / x64 / CRT / Debug・Release | 未検証 |
| ライセンス原文の保存先 / 確認日 | 未確認 |
| 必要な通知 / 再配布の対象と条件 | 未確認 |
| 統合サンプル / 確認command | 未実装 |
| Windows実行結果 / ログの保存先 | 未実施 |
| 既知の制約 / 次回更新時の確認事項 | 未確認 |

実装と環境の進捗は[STATUS.md](STATUS.md)へ、採用理由は[DECISIONS.md](DECISIONS.md)へ記録する。この文書に未検証の依存を「対応済み」と追加しない。

## 7. 一次資料

本文中のリンクはいずれも開発元・公式ドキュメント。確認日は2026-11-25。最新版を指すページの内容は変わるため、実装時には固定した版のドキュメント・ヘッダも保存する。

- [Microsoft: MSVC `/std`](https://learn.microsoft.com/en-us/cpp/build/reference/std-specify-language-standard-version?view=msvc-170)
- [Microsoft: Visual StudioのCMakeプロジェクト](https://learn.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=msvc-180)
- [CMake: Presets](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html)
- [Microsoft: DirectXMath](https://github.com/microsoft/DirectXMath)
- [Microsoft: DirectXShaderCompiler](https://github.com/microsoft/DirectXShaderCompiler/blob/main/README.md)
- [Dear ImGui: Getting Started](https://github.com/ocornut/imgui/wiki/Getting-Started)
- [Lua: Version history](https://www.lua.org/versions.html)
- [sol2: protected_function](https://sol2.readthedocs.io/en/latest/api/protected_function.html)
- [sol2: ownership](https://sol2.readthedocs.io/en/latest/tutorial/ownership.html)
- [Microsoft: XAudio2初期化](https://learn.microsoft.com/en-us/windows/win32/xaudio2/how-to--initialize-xaudio2)
- [CRI: ADX LE](https://game.criware.jp/products/adx-le/)
- [Effekseer: C++ Guide](https://github.com/effekseer/Effekseer/blob/master/docs/Help_Cpp/18x/Guide_Cpp_En.md)


# 進捗と次の作業

最終更新 2026-11-25。Visual Studio 2026 / MSVC v145を対象に、M02の背景色描画までWindows x64と実GPUで再検証済み。

## 現在地

| 項目 | 状態 |
| --- | --- |
| ユーザー要件の整理 | 文書化済み |
| 共通指示・スキル・設計・ロードマップ | 準備文書あり |
| カリキュラム・教材/演習テンプレート | 準備文書あり。完成授業原稿は未作成 |
| CMake / 環境確認プログラム | 作成済み。Windows x64のDebug・Releaseでビルド・実行を確認 |
| エンジンソース / shader | Win32の `Window` とD3D12の `ClearRenderer` を `clear_window` から使用。シェーダはM03から |
| 依存版の固定 | ツールの実測版を記録済み。第三者ライブラリの採用版は各導入段階で決定 |
| Windows環境・GPU確認 | Visual Studio 2026 / MSVC v145 / SDK / CMake / MSBuildとGPUの機種・ドライバ版を確認 |
| DX12実行確認 | NVIDIA GeForce RTX 2070 SUPERで背景色を確認。Debug Layerを有効にした実行で警告・エラーなし |
| 次に実行する段階 | M03の三角形描画とUnlitの基礎 |

## マイルストーン

| ID | 内容 | 状態 | 証跡 |
| --- | --- | --- | --- |
| M00 | 環境・build方針 | 検証済み | [環境・終了コード・ログ](DEPENDENCIES.md#23-m00の実測記録) |
| M01 | Win32ウィンドウ | 検証済み | [起動・操作・終了の記録](#2026-11-25-m01の作業記録) |
| M02 | DX12 clear | 検証済み | [Visual Studio 2026での再検証](#2026-11-25-visual-studio-2026への移行) |
| M03 | 三角形・Unlit基礎 | 未着手 | なし |
| M04 | frame context・同期 | 未着手 | なし |
| M05 | Node / Scene / Resource | 未着手 | なし |
| M06 | mesh / texture / Unlit | 未着手 | なし |
| M07 | 簡易RenderGraph | 未着手 | なし |
| M08 | PBR・HDR・tonemap | 未着手 | なし |
| M09 | ImGui | 未着手 | なし |
| M10 | Lua | 未着手 | なし |
| M11 | XAudio2 | 未着手 | なし |
| M12 | Effekseer | 未着手 | なし |
| M13 | 総合サンプルと教材 | 未着手 | なし |

## 直近の次の一手

M03でDXCの利用可能な版とシェーダ設定を確認し、頂点色付きの三角形を表示する。 [M03の受入条件](ROADMAP.md#m03-最初の三角形)

## 2026-11-25 Visual Studio 2026への移行

- ユーザー指定に合わせ、共有プリセットのジェネレーターを `Visual Studio 18 2026`、MSVCツールセットを `v145` に変更した。出力先は `build/vs2026-x64/`。 [D015](DECISIONS.md#d015-visual-studio-2026への移行)
- Community 2026とBuild Tools 2026で、Windows SDK 10.0.26100.0と同梱CMake / CTest 4.3.1-msvc1を使用した。Debug・Releaseともビルドは終了0で、C++20の環境確認は各1/1件合格。CPUのみの構成でも1/1件合格。
- RTX 2070 SUPER上で両環境・両構成の計8回を実行し、青とオレンジの表示、リサイズ、最小化からの復帰、最小化したままの終了を確認した。Debug版の警告・エラー・破損診断は0件だった。ウィンドウ操作はWindows APIで実施した。 [環境・コマンド・終了コード・ログ](DEPENDENCIES.md#24-visual-studio-2026の実測記録)

## 2026-11-25 M02の作業記録

- `engine/render/d3d12/ClearRenderer` にdevice、direct queue、swap chain、RTV、命令リストとフェンス待機を実装した。バッファを `PRESENT → RENDER_TARGET → PRESENT` と遷移させて背景色を表示し、毎フレームGPUの完了を待つ。サイズ変更は旧バッファの解放後に行い、最小化中は描画を止める。閉じる要求では描画側を破棄してからウィンドウを破棄する。 [D014](DECISIONS.md#d014-m02の描画とウィンドウの寿命)
- 検証環境はWindows x64 26H2、MSVC 19.44.35222、Windows SDK 10.0.26100.0、CMake / CTest 3.31.6-msvc6。実機ではNVIDIA GeForce RTX 2070 SUPER（10DE:1E84、ドライバ32.0.16.1088）を選択した。Debug版では `D3D12GetDebugInterface` が成功し、device作成前にDebug Layerを有効にした。 [環境の詳細](DEPENDENCIES.md#23-m00の実測記録)

ログ・画像・検証スクリプトは `build/logs/` 以下にある。通常の起動コマンドはREADMEに記載した。 [起動手順](../README.md#1-最初にすること)

| コマンド | 終了 | 観察とログ |
| --- | --- | --- |
| `cmake --preset windows-x64` | 0 | `build/windows-x64/` に生成。 `m02-configure.log` |
| `cmake --build --preset windows-debug` と `cmake --build --preset windows-release` | 各0 | 両構成の `clear_window.exe` を警告なしで生成。 `m02-build-debug.log`、`m02-build-release.log` |
| `ctest --preset windows-debug` と `ctest --preset windows-release` | 各0 | M00の `environment_check` が各1/1件合格。 `m02-test-debug.log`、`m02-test-release.log` |
| `python build/logs/m02-d3d12-smoke.py Debug` と `python build/logs/m02-d3d12-smoke.py Release` | 各0 | 各構成で既定色と `--warm` の2回を実行。 `m02-smoke-debug.log`、`m02-smoke-release.log` |
| `./build/windows-x64/Debug/clear_window.exe --invalid` | 1（期待値） | 使い方を表示して終了。 `m02-invalid-argument.log` |
| `cmake --preset windows-x64 -B build/cpu-only -DEDU_CHECK_WINDOWS_SDK=OFF`、`cmake --build build/cpu-only --config Debug --target edu_environment_check`、`ctest --test-dir build/cpu-only -C Debug --output-on-failure` | 各0 | D3D12サンプルを生成しないCPUのみの構成でも1/1件合格。 `m02-cpu-configure.log`、`m02-cpu-build.log`、`m02-cpu-test.log` |

- 全4回でクライアント領域の拡大・縮小に追従した。ウィンドウをキャプチャし、各色がリサイズ後も維持されることを画素で確認した。Debugの画像は `m02-debug-cool.png`（RGB 20, 56, 115）と `m02-debug-warm.png`（RGB 166, 61, 25）。
- 各構成の既定色では、最小化時に `0 x 0` の通知を受け、復帰後に背景が再表示された。`--warm` では最小化したまま閉じた。全4回でゼロ寸法のswap chain再生成がなく、終了コード0とウィンドウの破棄を確認した。Debug版で収集したDebug Layerの警告・エラー・破損診断は0件。
- ウィンドウ操作はWindows APIで検証した。物理マウスやキーボードでの操作、WARP、GPUの強制切断とフェンス待機タイムアウトの再現、GPU-based validationは未実施。

## 2026-11-25 M01の作業記録

- `engine/platform/win32/Window.cpp` と `Window.hpp` に、ウィンドウの生成・破棄、クライアント領域の変化通知、メッセージループを実装した。`samples/clear_window/main.cpp` は実行とログ出力を担当する。OS通知を待つ方式を採用した。 [D013](DECISIONS.md#d013-m01のウィンドウとメッセージ処理)
- 検証環境はM00と同じWindows x64 26H2、MSVC 19.44.35222、SDK 10.0.26100.0、CMake / CTest 3.31.6-msvc6。以下のログは `build/logs/` 以下にある。 [環境の詳細](DEPENDENCIES.md#23-m00の実測記録)

| コマンド | 終了 | 観察とログ |
| --- | --- | --- |
| `cmake --preset windows-x64` | 0 | `build/windows-x64/` に生成。 `m01-configure.log` |
| `cmake --build --preset windows-debug` と `cmake --build --preset windows-release` | 各0 | 両構成の `clear_window.exe` を警告なしで生成。 `m01-build-debug.log`、`m01-build-release.log` |
| `ctest --preset windows-debug` と `ctest --preset windows-release` | 各0 | M00の `environment_check` が各1/1件合格。 `m01-test-debug.log`、`m01-test-release.log` |
| `python build/logs/m01-win32-smoke.py Debug` と `python build/logs/m01-win32-smoke.py Release` | 各0 | Windows APIを使って `build/windows-x64/<構成>/clear_window.exe` を各2回操作。 `m01-smoke-debug.log`、`m01-smoke-release.log` |
| `cmake --preset windows-x64 -B build/cpu-only -DEDU_CHECK_WINDOWS_SDK=OFF`、`cmake --build build/cpu-only --config Debug --target edu_environment_check`、`ctest --test-dir build/cpu-only -C Debug --output-on-failure` | 各0 | 描画用SDKチェックを省いたCPUのみの手順も1/1件合格。 `m01-cpu-configure.log`、`m01-cpu-build.log`、`m01-cpu-test.log` |

- 全4回で、タイトル付きウィンドウが表示され、拡大・縮小後の寸法通知が実際のクライアント領域と一致した。最小化では `0 x 0`、復帰では元の寸法を通知した。各操作後に `WM_NULL` への応答を確認した。システムメニューの閉じるコマンドと、Alt+F4相当のWin32メッセージで、それぞれ終了コード0とウィンドウの破棄を確認した。実行ログは `m01-run-*.log`、Debugの表示を採取した画像は `m01-window.png`。
- 初回の実行テストではタイトルが空だった。`WM_NCCREATE` でオブジェクトを関連付けた後、`DefWindowProcW` に標準処理を任せるよう修正し、両構成でタイトルを再確認した。 `m01-initial-smoke-debug.log`
- ウィンドウ操作はWindows APIで実施した。物理マウスのドラッグやキーボードでの手動操作、GPU描画、Debug Layerは未実施。

## 2026-11-25 M00の作業記録

- 新規の `edu-dx12-engine` に配布パックを展開した。作業開始前に展開先の相対パス・バイト数・SHA-256を30/30件照合した。親フォルダの既存17ファイルも展開前後でSHA-256が一致した。
- `CMakeLists.txt`、`CMakePresets.json`、`samples/environment_check/main.cpp`、ビルド出力を除外する `.gitignore` を追加した。D001にVisual Studio 2022 / MSBuildを選んだ理由を追記した。 [設計判断](DECISIONS.md)
- Windows x64、MSVC 19.44.35222、SDK 10.0.26100.0、CMake / CTest 3.31.6-msvc6で `cmake --preset windows-x64`、`cmake --build --preset windows-debug`、`cmake --build --preset windows-release` を実行し、いずれも終了0。SDKチェックはクリーンなconfigureでも通過した。
- `ctest --preset windows-debug -V` と `ctest --preset windows-release -V` はそれぞれ1/1件合格、終了0。両方の実行ファイルを直接起動し、`C++ language version: 202002` と終了0を確認した。ログは `build/logs/` にあり、コマンドとファイル名は実測記録に対応する。 [M00の実測記録](DEPENDENCIES.md#23-m00の実測記録)
- 描画用SDKチェックを省く構成もビルド・テストが終了0。SDKチェック失敗を模擬した構成は、手順を表示して終了1になった。初回ビルドの文字コード警告 C4819 は `/utf-8` を指定して解消した。
- GPUは機種とドライバを確認した。Win32ウィンドウの動作、GPUでの描画、Graphics ToolsとDebug Layerの利用可否、Windows以外でのCPUビルドは未実施。M00の検証を妨げる障害は残っていない。

## 未確定事項

| 項目 | 決める時期 | 今すぐ停止する理由になるか |
| --- | --- | --- |
| 正式なエンジン名 | 任意 | ならない。仮称で進む |
| 学校PCで使用するVS / MSVC / SDK / CMake | 授業環境を準備する時点 | M00の検証PCでの実測版を記録済み |
| DXC版とshader profile | M03 | M00〜M02には不要 |
| ImGui / Lua / sol2 / Effekseer版 | 各導入段階 | それ以前の段階には不要 |
| 公開ライセンス・教材配布方式 | 公開・配布前 | 私的な設計/実装を止めない |
| CRI利用の有無 | M11以降の選択 | 初期案のXAudio2で進める |

## タスク引継ぎ記録の書式

- 日付・段階・依頼された到達点：
- 実装/変更したファイルと動作：
- 実行したコマンド・環境・終了コード：
- 観察した結果とログのパス：
- 未実施の確認と理由：
- 残る問題と再現条件：
- 新規/変更した判断ID：
- 次に行う最小の一手：

状態は「未着手」「進行中」「実装済み・検証待ち」「検証済み」「ブロック中」を使う。コードだけで「検証済み」にしない。準備文書の確認結果は[PREPARATION_CHECKS.md](PREPARATION_CHECKS.md)へ分ける。

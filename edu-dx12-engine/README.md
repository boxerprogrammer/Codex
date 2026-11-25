# 教育用DX12エンジン — Codex開発準備パック

**C++20 / DirectX 12の教育用エンジンと、対応する専門学校向け教材を段階的に作るための指示・設計文書一式**です。作成日：2026-11-25。

**Visual Studio 2026でM02まで検証済み**です。Windows x64でC++20の環境確認プログラムと、DirectX 12で背景色を表示するウィンドウをビルドして実行できます。次はM03の三角形描画です。

## 1. 最初にすること

Windows x64でVisual Studio 2026の「C++によるデスクトップ開発」、MSVC v145、Windows SDKを用意します。共有プリセットには CMake / CTest 4.2以上が必要です。検証にはVisual Studio 2026同梱のCMake / CTest 4.3.1-msvc1とWindows SDK 10.0.26100.0を使用しました。 [検証環境の詳細](docs/DEPENDENCIES.md#24-visual-studio-2026の実測記録)

プロジェクトのルートでPowerShellを開きます。`cmake --version` と `ctest --version` で使うツールを確認してください。同梱版を使う場合は、Visual Studio 2026のインストール先にある `Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin` を現在のシェルの `PATH` の先頭に追加できます。

```powershell
cmake --list-presets=all
cmake --preset windows-x64
cmake --build --preset windows-debug
ctest --preset windows-debug
.\build\vs2026-x64\Debug\edu_environment_check.exe
.\build\vs2026-x64\Debug\clear_window.exe
```

プリセットは繰り返し使うビルド設定に名前を付けたものです。configureはビルドに必要な設定を調べ、Visual Studio 2026用のプロジェクトを `build/vs2026-x64/` に生成します。環境確認プログラムを実行すると `C++ language version: 202002` と表示されます。Releaseも同じ生成先を使います。ビルドとテストのプリセット名を `windows-release` に変え、実行ファイルは `Release` フォルダから起動します。

`clear_window.exe` を起動すると、`Edu DX12 Engine - M02` と表示されたウィンドウに青い背景が出ます。起動コマンドの末尾に `--warm` を付けると、オレンジの背景を表示します。ウィンドウの枠をドラッグすると、PowerShellにクライアント領域（枠やタイトルバーを除く内側）と描画先の寸法が出ます。最小化中は描画を休止し、復帰すると再開します。閉じるボタンまたは Alt+F4 で終了します。

実行にはDirectX 12対応のGPUが必要です。Debug版はDebug Layer（描画APIの使い方を点検する機能）を有効にします。`D3D12GetDebugInterface` のエラーが出た場合は、Windowsのオプション機能で「グラフィックス ツール（Graphics Tools）」を確認してください。

描画用SDKのチェックとDirectX 12サンプルの生成を省き、CPU側だけを調べるときは別の生成先を使います。WindowsのMSVCコンパイラ自体が必要とするWindows SDKは引き続き必要です。

```powershell
cmake --preset windows-x64 -B build/cpu-only-vs2026 -DEDU_CHECK_WINDOWS_SDK=OFF
cmake --build build/cpu-only-vs2026 --config Debug --target edu_environment_check
ctest --test-dir build/cpu-only-vs2026 -C Debug --output-on-failure
```

検証結果、実測した版、コマンド、ログは以下を参照してください。 [STATUS.md](docs/STATUS.md) · [DEPENDENCIES.md](docs/DEPENDENCIES.md#24-visual-studio-2026の実測記録)

既存リポジトリへ導入する場合は、既存の`AGENTS.md`等を先に読み、必要な方針を統合します。他のプロジェクトの指示や未コミット変更を無条件に上書きしないでください。

### 一括受け渡し用Markdownから始める場合

別添の`EDU_DX12_CODEX_HANDOFF.md`には、全ファイルの相対パスと本文を収録しています。この1ファイルをCodexへ渡し、「記載された相対パスに展開してから、初回プロンプトに従ってM00を進めてください」と依頼できます。手で各ファイルをコピーする必要はありません。

展開はまず空のサブフォルダへ行い、ファイル一覧・サイズ・SHA-256を確認します。既存プロジェクトへ統合する場合は差分を見て反映します。一括版は受け渡し用であり、展開後の正本は各ファイルです。開発中は一括版を編集せず、各ファイルを更新してください。

## 2. 指示ファイルの名前

| ファイル | 役割 |
| --- | --- |
| [AGENTS.md](AGENTS.md) | Codexがプロジェクトで継続して守る共通指示の正本 |
| [Agent.md](Agent.md) | ご指定の名前から`AGENTS.md`へ案内する入口 |
| [.agents/skills/build-educational-dx12-engine/SKILL.md](.agents/skills/build-educational-dx12-engine/SKILL.md) | 開発と教材執筆の再利用可能な作業手順 |
| [Skill.md](Skill.md) | ご指定の名前から正式なスキルへ案内する入口 |

Codexの標準では、プロジェクト指示に`AGENTS.md`、スキルに`.agents/skills/<name>/SKILL.md`を使います。`Agent.md`やルートの`Skill.md`だけで自動認識されるとは扱わないでください。スキルのYAMLには`name`と`description`を持たせています。[公式AGENTS.md説明](https://learn.chatgpt.com/docs/agent-configuration/agents-md)、[公式スキル説明](https://learn.chatgpt.com/docs/build-skills)

このスキルは明示的な利用を基本にする設定です。プロジェクトの`AGENTS.md`と開始プロンプトから読み込みます。ChatGPT側と別PCのCodexのスキル配置が自動で一致するとは仮定せず、プロジェクト内のファイルを使ってください。

## 3. 含まれる文書

| 文書 | 決めていること |
| --- | --- |
| [PROJECT.md](docs/PROJECT.md) | 目的、学生像、ユーザー指定要件、暫定案、対象外 |
| [ARCHITECTURE.md](docs/ARCHITECTURE.md) | モジュール、Node / Scene / Resource、所有権、更新順 |
| [RENDERING.md](docs/RENDERING.md) | DX12、同期、descriptor、RenderGraph、Unlit / PBR、色と行列 |
| [ROADMAP.md](docs/ROADMAP.md) | M00〜M13の前提・成果物・受入条件 |
| [DEPENDENCIES.md](docs/DEPENDENCIES.md) | 環境、依存の導入順、版固定、統合条件 |
| [CODING_STANDARDS.md](docs/CODING_STANDARDS.md) | C++20、命名、所有権、エラー処理、コメント |
| [VALIDATION.md](docs/VALIDATION.md) | Windows・GPU確認、CPUテスト、障害調査、完了判定 |
| [DECISIONS.md](docs/DECISIONS.md) | 確定要件と暫定採用案の根拠、再検討条件 |
| [STATUS.md](docs/STATUS.md) | 実装前の現在地、未確認、次の作業、引継ぎ形式 |
| [CURRICULUM.md](docs/CURRICULUM.md) | 学習の段階、授業例、コードと教材の対応、評価 |
| [GLOSSARY.md](docs/GLOSSARY.md) | 専門用語の一言の言い換え |
| [REFERENCES.md](docs/REFERENCES.md) | 公式資料の入口、調査・検証の限界 |
| [PREPARATION_CHECKS.md](docs/PREPARATION_CHECKS.md) | 今回の準備文書について実際に行った確認 |

### プロンプトとテンプレート

| 用途 | ファイル |
| --- | --- |
| 開発を始める | [START_DEVELOPMENT.md](prompts/START_DEVELOPMENT.md) |
| 指定段階まで進める | [CONTINUE_DEVELOPMENT.md](prompts/CONTINUE_DEVELOPMENT.md) |
| 実装を教材化する | [WRITE_LESSON.md](prompts/WRITE_LESSON.md) |
| 失敗を調査する | [DIAGNOSE_FAILURE.md](prompts/DIAGNOSE_FAILURE.md) |
| 学生向けの章 | [LESSON_TEMPLATE.md](docs/templates/LESSON_TEMPLATE.md) |
| 演習と別ファイルの解答 | [EXERCISE_TEMPLATE.md](docs/templates/EXERCISE_TEMPLATE.md) |
| 実装タスクの記録 | [TASK_TEMPLATE.md](docs/templates/TASK_TEMPLATE.md) |
| 設計判断の記録 | [DECISION_TEMPLATE.md](docs/templates/DECISION_TEMPLATE.md) |

## 4. 最初の設計方針

- **Node**：ゲーム世界を階層で組み立てる部品。親が子を単一所有します。
- **Scene**：実行中のNodeツリーと、その更新・変更の境界を管理します。
- **Resource**：Mesh・Texture・Material等の共有可能なCPU素材です。
- **Renderer**：Sceneから集めた描画データを使い、GPU資源・状態・同期を管理します。
- **RenderGraph**：各パスのread/writeと明示的な順序から状態遷移を計画します。初期は単一queueです。

これらの細部は準備時の推奨案です。何がユーザー指定で、何が暫定採用かを[DECISIONS.md](docs/DECISIONS.md)で分けています。

## 5. 学習順に沿った開発

| 段階 | 内容 |
| --- | --- |
| M00〜M02 | 環境、ウィンドウ、DX12による背景色描画 |
| M03〜M04 | 三角形、CPU / GPUの同期と寿命 |
| M05〜M06 | Node / Scene / Resource、3Dメッシュ、Unlit |
| M07〜M08 | 簡易RenderGraph、PBR、HDRから表示色への変換 |
| M09〜M12 | ImGui、Lua、XAudio2、Effekseer |
| M13 | 小さな総合ゲームと対応教材 |

授業では90分×2コマを目安に必要な部分を選べます。授業で扱わない機能があっても、最終エンジンに求める必須要件が自動的になくなるわけではありません。

## 6. 検証の読み方

M00ではC++20のビルド環境と描画用SDK、M01ではWin32のウィンドウ操作を検証しました。M02では実GPUで2色の表示を確認し、サイズ変更、最小化、復帰、終了を描画が有効な状態で再検証しました。 [検証記録](docs/STATUS.md)

## 7. 次回以降の引継ぎ

Codexには、毎回まず`docs/STATUS.md`で現在地を確認させます。変更理由は`docs/DECISIONS.md`、依存の採用版は`docs/DEPENDENCIES.md`、実行結果は検証記録に残します。教材を作るときは対象の実装版を指定し、架空のAPIや未実行の結果を教材に混ぜないようにします。

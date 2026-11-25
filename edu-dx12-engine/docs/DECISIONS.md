# 設計判断記録

作成日：2026-11-25。**要件として確定**と**準備時の暫定採用**を区別する。暫定採用は合理的な初期値であり、ユーザーが細部まで承認済みという意味ではない。

## 確定した要件

| ID | 内容 | 根拠 |
| --- | --- | --- |
| R001 | 可読性・理解しやすさを最優先 | ユーザーのCore Philosophy |
| R002 | C++20 / DX12 / HLSL | ユーザーのProject Specifications |
| R003 | PBRとUnlit、簡易RenderGraph | 同上 |
| R004 | ImGui / Lua / Effekseer、XAudio2またはCRI | 同上 |
| R005 | GodotのNode / Scene / Resourceを参考にした責務分離 | 同上 |
| R006 | 開発・教材の2モード、重要な設計の複数案比較 | Operation Modes / Responsibilities |
| R007 | 初出用語の説明、よくあるミス、具体例、段階的演習 | Constraints / Response Format |
| R008 | エンジン実装の対象IDEはVisual Studio 2026 | ユーザー指定 |

## 暫定採用一覧

| ID | 判断 | 比較した案 | 暫定採用理由・代償 | 見直す条件 |
| --- | --- | --- | --- | --- |
| D001 | 初回のWindows x64 / MSVC / CMake | VS専用solution / CMake / 独自build | CMakeはコマンド・IDE・検証を共通化しやすい。初回に設定の説明が必要 | 対象環境はD015で更新 |
| D002 | Node派生+小さなデータ合成 | Node派生 / 汎用Component / ECS | 基礎C++から追いやすい。大規模データ指向最適化は保留 | 継承が増え同一機能の再利用が難しくなった |
| D003 | Node単一所有・外部handle | shared_ptrツリー / unique_ptrツリー+handle / IDだけのregistry | 破棄の根を明確にし、外部参照の安全性も扱える。handle実装が必要 | Scene横断所有や複雑な編集機構を導入 |
| D004 | DX12専用Renderer | 専用実装 / 複数APIのRHI / 既存高水準renderer | DX12の順序と責務を見せやすい。移植性は下がる | 他APIの教育が明確な要件になった |
| D005 | 初期は毎フレーム待機、M04でframe context | 毎回待機 / 2〜3frame並行 / multi-queue | 最初の同期を説明してから再利用の安全性へ進める。初期版は遅い | M04へ進んだ時点 |
| D006 | 挿入順・単一queueのRenderGraph | 固定描画 / 明示順の簡易graph / 自動schedule+aliasing | read/writeとbarrierを集中管理でき、順序を追える。不正順を自動修正しない | 発展授業でトポロジカル整列等を扱う |
| D007 | 直接光metallic-roughness PBR | Unlitのみ / 基本PBR / IBL+多数light | Unlitと光応答を比較できる。IBL無しは環境反射を再現しない | 基本PBRが検証済みとなりIBLを学ぶ |
| D008 | XAudio2を初期採用 | XAudio2 / CRI無料版 | 基本再生を小さく学ぶ。編集用ツール等は用意しない | CRIの現行Native SDKと利用・教材配布条件が確認できた |
| D009 | DXCによる明示的shader build | build時DXC / 起動時compile / hot reload | ビルドログと成果物の追跡が容易。hot reloadは後回し | 授業の反復速度が課題になった |
| D010 | LH・行ベクトル・通常Z | LH/RH、row/column、通常Z/Reverse-Z | DX12基礎教材と対応させやすい。外部形式には変換が必要 | 他教材との整合に明確な理由がある |
| D011 | Lua+sol2を候補にし導入前に相互確認 | Lua C API / sol2 / 独自言語 | bindingコードを短くできる。依存とtemplateエラーの説明が必要 | 固定Lua版との互換性が成立しない場合はLua C APIへ |
| D012 | 素材は共有、ゲーム状態は複製 | すべて複製 / すべて共有 / 素材のみ共有 | 同じmeshを再利用しつつ別Nodeの状態を独立させる | 共有素材の実行時編集を追加 |
| D013 | M01では `GetMessageW` でOS通知を待つ | 待機型 / 非待機型 | 描画処理を持たない段階で待機・振り分け・終了を追いやすい | M02でフレーム進行を追加 |
| D014 | M02では描画の前にOS通知を処理し、描画領域が使えない間だけ待つ | 通知ごとの描画 / フレームごとの描画 | 次の段階でも毎フレームの更新を追加しやすい。最小化中の待機条件が増える | フレーム進行や待機条件が増えた時点 |
| D015 | Windows x64の共通プリセットをVisual Studio 2026 / v145へ変更 | R008に従い生成先を選択 | CMake 4.2以上でVisual Studio 2026用のプロジェクトを生成する | 対象IDEの要件変更時 |

## 重要な補足

### D001：M00で使用するCMakeの生成先

M00の初回検証では、Windows x64の共通プリセットに `Visual Studio 17 2022` を使用した。ジェネレーターは、CMakeがどのビルドツール向けに設定を出力するかの選択。現在の対象環境はD015に記録する。

| 案 | 可読性・拡張 | 授業難易度・導入費用 |
| --- | --- | --- |
| Visual Studio 2022 / MSBuild | 一つの生成先でDebug・Releaseを切り替えられ、生成したソリューションをIDEでも開ける | 低。既存のVSとSDKを利用し、通常のPowerShellから操作できる |
| Ninja Multi-Config / MSVC | 同じCMakeプロジェクトで構成を切り替えられ、IDEを使わない環境にも合わせやすい | 中。Ninjaの場所とMSVCの開発者環境を揃える必要がある |

開始時に学生が使うコマンドを少なくできるMSBuildを選んだ。Visual Studioのインストール先とSDKの具体版は、実測記録に残した。 [CMakeのVisual Studio 2022ジェネレーター](https://cmake.org/cmake/help/v3.25/generator/Visual%20Studio%2017%202022.html)

C++20の確認プログラムを独立したターゲットに置き、Windowsの描画用SDKのヘッダ・ライブラリはconfigure時のチェックで使う。これにより、SDK側に問題がある場合でもチェックを明示的に外してCPU側の構成を調べられる。

### D002：NodeとComponentを同時に巨大化させない

`Node3D`、`CameraNode`、`MeshInstanceNode`などの役割が説明できる最小の派生を考える。Transform等は小さなデータ型にまとめる。広い継承階層と汎用Component登録システムを同時に教えない。共通ロジックの再利用要求が現れたときに合成へ移す。

### D005：段階的な待機の削減

M02〜M03ではGPU完了を待つ単純なループを許容する。M04で`FrameContext`ごとのallocator・upload・fence値を導入し、再利用対象のframeだけを待つ。簡略版でもbarrierと寿命管理を省略しない。

### D006：RenderGraphの「Graph」の意味を限定する

初期版はread/writeによる依存とresource stateを記録し、明示したパス順を検証して実行する。書込み前読出し等をエラーにし、自動で順番を直さない。高度な一般DAGスケジューラを完成させたとは表現しない。

### D007：PBRの到達点

M08は直接光・metallic-roughness・GGX等の基本BRDFと色空間を扱う。IBL、影、複数散乱補償、透過、clearcoatは初期PBR完了条件に含めない。PBR対応の範囲を画面と教材で正しく説明する。

### D013 M01のウィンドウとメッセージ処理

| 案 | 可読性・拡張 | 授業難易度・費用 |
| --- | --- | --- |
| `GetMessageW` で通知を待つ | メッセージの取得、振り分け、終了が一つのループで追える | 低。描画のためのループ制御が不要 |
| `PeekMessageW` で通知の有無を調べる | 通知がない間もフレームを進められる | 中。フレームの処理順と、最小化中などに待つ条件の説明が必要 |

M01では待機型を採用した。Win32の通知を受ける関数 `WindowProc` は `Window` の状態を更新し、サンプルは寸法の通知をログに出した。M01の閉じる操作はWindowsの標準処理でウィンドウを破棄し、`WM_DESTROY` から処理ループの終了を伝えた。M02の処理順はD014に記録する。 [MicrosoftのGetMessageWの仕様](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getmessagew)・[ウィンドウ終了の処理順](https://learn.microsoft.com/en-us/windows/win32/learnwin32/closing-the-window)

### D014 M02の描画とウィンドウの寿命

| 案 | 可読性・拡張 | 授業難易度・費用 |
| --- | --- | --- |
| 描画通知を受けたときに更新 | 静止した背景は少ない処理で表示できる。連続更新を足すと描画タイミングの設計が必要 | 初回は低。次段階でフレーム進行を説明する |
| OS通知の処理後に毎フレーム更新 | 通知、リサイズ、描画の順をアプリ側で追え、更新処理も足せる | 中。最小化中の待機とGPUの完了待ちを説明する |

M02はフレームごとの更新を採用した。 `WindowProc` はサイズや終了要求を記録し、アプリのループが描画に反映する。閉じる要求を受けたら新規の描画を止めて、swap chainを持つ `ClearRenderer`、その後にウィンドウの順で破棄する。GPUへの送信はD005に従い毎フレーム完了を待つ。リサイズも完了後に旧描画先への参照を解放してから行う。 [MicrosoftのResizeBuffersの要件](https://learn.microsoft.com/en-us/windows/win32/api/dxgi/nf-dxgi-idxgiswapchain-resizebuffers)

### D015 Visual Studio 2026への移行

2026-11-25、ユーザーが指定したVisual Studio 2026に合わせ、共有プリセットのジェネレーターを `Visual Studio 18 2026`、MSVCツールセットを `v145` に設定した。Windows用プリセットの最低CMake版は、このジェネレーターが追加された4.2とする。 [CMakeのVisual Studio 2026ジェネレーター](https://cmake.org/cmake/help/latest/generator/Visual%20Studio%2018%202026.html)

プリセット名は既存手順と同じ名前を使い、出力先を `build/vs2026-x64/` にした。これにより、初回検証時に生成したVisual Studio 2022のCMakeキャッシュと共存できる。CMakeListsから直接構成するCPU側の最低CMake版は3.21。

## 追加判断の書式

[DECISION_TEMPLATE.md](templates/DECISION_TEMPLATE.md)を使い、日付、状態、問題、2〜3案、授業難易度、採用理由、不利な点、確認方法、再検討条件を残す。大きな変更時は旧判断を消さず「置換済み」として後継IDを記録する。

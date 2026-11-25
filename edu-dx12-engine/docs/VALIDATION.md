# 検証と完了判定

作成日: 2026-11-25。この文書は検証計画を扱う。実施済みの検証と結果は [STATUS.md](STATUS.md) に記録する。

## 目次

- [1. 検証できた範囲を明示する](#1-検証できた範囲を明示する)
- [2. 変更に応じて必要な検証を選ぶ](#2-変更に応じて必要な検証を選ぶ)
- [3. Windowsでの描画確認](#3-windowsでの描画確認)
- [4. マイルストーンごとの確認項目](#4-マイルストーンごとの確認項目)
- [5. 失敗を入力するテスト](#5-失敗を入力するテスト)
- [6. Debug Layer・PIX・DREDの使い分け](#6-debug-layerpixdredの使い分け)
- [7. 実行証跡テンプレート](#7-実行証跡テンプレート)
- [8. 完了の定義](#8-完了の定義)

## 1. 検証できた範囲を明示する

「コードがある」「ビルドできた」「期待した絵や音が出た」は別々の事実として記録する。テストを実行していない場合、成功と推測しない。

| 状態 | 意味 | 記録例 |
| --- | --- | --- |
| `verified` | 指定した環境で確認を実行し、期待結果に一致した | Windows x64 Debugビルド成功、終了コード0、ログあり |
| `unverified` | 確認をまだ実行していない | 初回実装前、または今回は未実行 |
| `blocked` | 必要な確認が環境・依存等によって実施できない | Windows runner / GPU / 必要SDKがない |
| `failed` | 実行し、期待結果に一致しなかった | resize後にdevice removed、再現手順あり |

`verified`は対象を省略しない。「CPU単体テストがverified」は「DX12描画もverified」を意味しない。マイルストーン全体の進捗表現は[STATUS.md](STATUS.md)と揃える。

### 環境ごとの役割

| 実行環境 | 確認できる主な範囲 | 確認できないと扱う範囲 |
| --- | --- | --- |
| Linux等のCPU実行環境 | Markdown・設定の整合、移植可能なC++20のCPUテスト | Windows APIのリンク・DX12の描画・XAudio2の音声 |
| Windows / MSVC / Windows SDK | ネイティブビルド、Win32動作、CPUテスト | 実行していないGPU機能・未接続の音声装置 |
| Windows / 対象GPU / Debug Layer | 描画、resource寿命、barrier、descriptor等の検査 | 他GPU・他ドライバでの保証 |
| Windows / WARPを明示して使用 | 対応範囲でのソフトウェア描画とAPI検査 | 学生PCの実GPUの性能や同一挙動の保証 |

Windowsが使えないCodex環境では、可能な実装・静的点検・CPUテストまで進める。Windows確認を`blocked`として具体的な実行手順を残し、未検証の描画段階を完成扱いにしない。GPU検証が前提となる後続統合を、検証済みと仮定して連鎖的に進めない。

## 2. 変更に応じて必要な検証を選ぶ

| 変更内容 | 最小限行う確認 |
| --- | --- |
| 文言、教材の説明、Markdownリンク | 対象文書と参照先の整合、未実装表記、コードとの一致 |
| CMakeや依存版 | 対象構成のconfigure・build、依存版記録、必要なら統合サンプル |
| Node・Resource・ハンドル | 所有権、無効化、ツリー変更のCPUテスト |
| GPU resource / fence / descriptor | Windows Debug実行、対象経路のDebug Layer検査、寿命関連の再現確認 |
| shader / PSO / root signature | 実際のshaderコンパイル、Windows描画、bindingと見た目の確認 |
| RenderGraph | CPUの計画・拒否テストと、実際のpass連携のGPU確認 |
| Lua / XAudio2 / Effekseer | その統合の小さな実行サンプル、終了処理、代表的な失敗入力 |
| UIの小さな色・配置変更 | 対象画面の確認。無関係な全機能テストは追加しない |

自動テストは、「問題があれば失敗する」振る舞いを検査する。常にtrueを返すテスト、実装コードを同じ形で書き写しただけのテスト、スクリーンショットを撮っただけで合格するテストを置かない。

性能はMVPの最優先判定にしない。ただし暴走、無限待ち、回収されず増え続けるメモリ、操作不能になる遅延は不具合として扱う。小規模な毎フレーム配列構築は許容し、測定なしにゼロ確保化を必須にしない。教育的な単純化と誤った寿命管理を混同しない。

## 3. Windowsでの描画確認

### 3.1 ビルドと起動

1. [環境記録](DEPENDENCIES.md)でMSVC / SDK / GPU / ドライバ / 依存版を特定する。
2. 実在するpresetを列挙してconfigure・buildする。コマンド、終了コード、エラー・警告を保存する。
3. Debug版を実行し、期待する背景色・形状・UIを確認する。表示できたこととDebug Layerに問題がないことを別々に確認する。
4. M13等の配布節目ではRelease版もビルド・実行する。Debug時だけ存在する初期化に依存していないか確認する。
5. 対象サンプルを正常終了し、GPU待機・resource解放・スレッドやcallback終了まで確認する。

クリーンなビルドディレクトリでの確認は、依存導入や配布節目など再現性が問題になるときに行う。小さな変更のたびに全構成を作り直す必要はない。

### 3.2 Debug Layer

Debug Layerは「DirectX 12の使い方を点検する監督役」である。`EnableDebugLayer`はdeviceを作成する前に呼ぶ。device作成後に有効化する処理を置かない。[Microsoft: EnableDebugLayer](https://learn.microsoft.com/en-us/windows/win32/api/d3d12sdklayers/nf-d3d12sdklayers-id3d12debug-enabledebuglayer)

Debug実行では次を満たす。

- 有効化の成否をログに残す。利用できない場合に黙って無効化して合格扱いにしない。
- error / corruption相当のメッセージを残さない。warningも原因を調べ、既知の例外を残すなら理由と対象IDを記録する。
- resource、command list、passに意味のある名前を付け、エラー箇所と授業で説明する対象を対応させる。
- barrierによる状態変更、root signatureとshaderのbinding、descriptorの範囲と寿命を検査する。
- 開発用機能がReleaseの実行条件にならないようにする。

GPU-Based Validation（GBV）は「GPUで実際に使われる瞬間も点検する追加検査」である。descriptorやresource状態の不具合を調べるとき、またGPU寿命管理の主要な節目に有効化する。device作成前の設定が必要で、実行負荷も増えるため、通常の全実行やReleaseへ強制しない。[Microsoft: GBV](https://learn.microsoft.com/en-us/windows/win32/direct3d12/using-d3d12-debug-layer-gpu-based-validation)、[設定API](https://learn.microsoft.com/en-us/windows/win32/api/d3d12sdklayers/nf-d3d12sdklayers-id3d12debug1-setenablegpubasedvalidation)

### 3.3 resize・最小化・終了

以下は本エンジンの受け入れテスト方針である。適用する段階以降、変更に関わる経路を確認する。

- ウィンドウを広げる・縮める。back bufferと必要なdepth / HDR targetの寸法が更新される。
- 最小化して戻す。描画領域が0の間は描画やサイズ依存resourceの再生成を保留し、0で除算しない。
- resize前に旧resourceを使用するGPU作業の完了を待つ。旧back bufferへの参照を解放し、再取得したbufferにdescriptorを作り直す。
- frame index、allocator、upload領域、descriptorの再利用条件が一致する。
- 起動→描画→終了を複数回試し、終了中にcallbackやGPUが解放済みメモリを参照しない。

allocatorの`Reset`は、そのallocatorに記録したGPU作業が終わった後に行う。swap chainの`ResizeBuffers`には旧back bufferの参照解放が必要である。DXGI文書のDX11用context操作をDX12へそのまま持ち込まない。[Allocator Reset](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12commandallocator-reset)、[ResizeBuffers](https://learn.microsoft.com/en-us/windows/win32/api/dxgi/nf-dxgi-idxgiswapchain-resizebuffers)

## 4. マイルストーンごとの確認項目

詳細な成果物と依存順は[ROADMAP.md](ROADMAP.md)に従う。以下は検証観点の補足で、すべて未実施から開始する。

| 段階 | 確認する振る舞い | 代表的な証拠 |
| --- | --- | --- |
| M00 環境 | 実環境と不足物を列挙し、CMake土台を確認できる | 環境表、configure結果 |
| M01 Win32 | 開く、拡大・縮小、最小化・復帰、閉じる。メッセージ処理が固まらない | 寸法の通知と起動・終了のログ、画面 |
| M02 clear | 指定色を継続表示し、PRESENTへ戻し、正常終了する | Debug Layerログ、画面 |
| M03 triangle | shaderをコンパイルし、三角形と頂点色を確認する | DXCログ、binding一覧、画面 |
| M04 frame contexts | 再利用するframe資源だけを適切に待ち、ちらつき・寿命エラーを出さない | fence値の記録、Debug Layer / 必要時GBV |
| M05 Scene | 親子所有、削除とハンドル無効化、遅延変更が成立する | CPUテスト結果 |
| M06 Unlit | 3D変換、depth、texture、共有resourceを確認する | 既知の配置の画面、resource確認 |
| M07 RenderGraph | 宣言した順番とresource利用でpassが動き、不正な宣言を拒否する | CPUテスト、passログ、GPU検査 |
| M08 PBR | 金属度・粗さ・光の変更に応答し、Unlit切り替えと色変換を確認する | 比較画面、選んだ条件、shaderログ |
| M09 ImGui | 入力・表示・画像参照・resizeが動き、終了時に参照が残らない | UI画面、入力手順、GPU検査 |
| M10 Lua | 正常な更新が動き、構文・実行エラーと消滅Nodeを扱える | scriptごとのログ、CPU確認 |
| M11 XAudio2 | PCM再生・停止・再再生・終了を確認する | 聴取結果、voice / buffer状態の記録 |
| M12 Effekseer | 同一の描画系に1個のeffectを統合し、停止・resize・終了を確認する | effect画面、固定版sample比較、GPU検査 |
| M13 総合 | 手順通りに再現し、教材と実際のAPI・画面が一致する | Debug / Release実行、教材対応表 |

### M08の観察条件を固定する

同じmesh、camera、露出、base colorを使って、UnlitとPBRを切り替える。PBRではmetallic / roughnessを変えた比較、光を動かした比較、NaNや極端な明滅の有無を確認する。IBL未実装であれば、そのために暗くなる領域をIBL実装済みの結果として評価しない。

色はlinearで計算し、HDRから表示用の範囲へ変換する責務を確認する。UIにはシーンの露出変更が不意にかからない構成を検査する。sRGB変換を二重に適用していないか、採用したUI合成方式と[RENDERING.md](RENDERING.md)を照合する。単に「トーンマップ後なら何でも正しい」と判定しない。

## 5. 失敗を入力するテスト

正常系だけでは気付けない設計上の境界を確認する。該当機能を実装する段階で追加し、まだないAPIのために空のテストを大量生成しない。

| 対象 | 入力・操作 | 期待する結果 |
| --- | --- | --- |
| NodeHandle | Node削除後に古いhandleで取得 | 失敗を返し、別Nodeに化けない |
| Scene | 走査中に子を追加・削除 | 変更が安全な時点に適用され、走査を壊さない |
| Scene | 他Sceneのhandle / 不正な親子循環 | 誤った参照や循環を拒否する |
| Resource | 同じ資産を2Nodeで共有し、一方を削除 | 他方の利用を維持する |
| RenderGraph | 作成しただけで未初期化のresourceを読む | pass名とresource名を示して拒否する |
| RenderGraph | 後続passの出力を先行passから読む | 挿入順実行の規約違反として拒否する |
| RenderGraph | 同じpassで許可していないread/write競合 | 無断で状態を推測せず拒否する |
| RenderGraph | 無効handle / 未宣言の利用 / import情報不足 | 検出できる層で明確な失敗にする |
| Shader | 入口名の誤り、構文エラー | 対象ファイル・入口・診断を表示する |
| GPU | 計画に矛盾するdescriptor / resource利用 | 計画検証またはDebug Layerで問題を特定する |
| Lua | 構文エラー、実行時error、削除済みNode操作 | 対象scriptのエラーにし、エンジンを不意に終了させない |
| Audio | 未対応WAV / 読み込み失敗 / 再生中の終了 | 診断し、保持中のbufferを早く解放しない |
| Effekseer | 読み込めないeffect / texture | 読み込み失敗を返し、他の描画を維持する |

GPUの不正操作を常用サンプルに混ぜない。Debug Layerの教育用実験で意図的な失敗を作る場合は、独立した実験として手順・期待診断・修正方法を添える。

## 6. Debug Layer・PIX・DREDの使い分け

| 手段 | 学生向けの言い換え | 主な用途 |
| --- | --- | --- |
| Debug Layer | APIの使い方を点検する監督役 | barrier、descriptor、binding、寿命の誤り |
| GBV | GPUで使う瞬間も確認する追加検査 | GPU時間軸で現れる不正参照 |
| PIX GPU capture | 1フレームの作業を記録して見直す道具 | draw / passの順番、resource、PSO、画像の変化 |
| DRED | GPU停止時に残る足跡 | device removed時の進行位置や関連resourceの手掛かり |

### 6.1 黒い画面を調べる順番

1. shaderのコンパイル失敗、HRESULT、Debug Layerのerrorを先に確認する。
2. clearだけの画面へ戻し、swap chainと表示が成立するかを分離する。
3. PIXで対象frameを取得し、期待するpassとdrawがあるか確認する。
4. render target / depth、viewport / scissor、PSO / root signature、vertex / index buffer、texture / constantの順に、実際に設定された値を追う。
5. 色の問題なら、表示変換前のlinear / HDR画像と最終画像を比較する。

PIX captureはDirect3D 12の呼び出しと引数を記録して調べる仕組みである。無効なAPI利用があるとcaptureできない場合もあるため、先にDebug Layerの診断を直す。またcaptureの再生可否はGPU・ドライバにも依存する。[PIX公式GPU Captures](https://devblogs.microsoft.com/pix/gpu-captures/)

### 6.2 device removedを調べる順番

device removedは「アプリからGPU deviceを使い続けられなくなった状態」と説明する。HRESULTとdevice removed reason、直前のpass名、最後に送信したfence値、Debug Layerの記録を保存する。DREDを利用できる環境では、必要な設定をdevice作成前に行い、停止時のbreadcrumbsやpage fault情報を追加の手掛かりにする。足跡の最終位置だけで原因を断定しない。[Microsoft: DRED](https://learn.microsoft.com/en-us/windows/win32/direct3d12/use-dred)

初期教材では複雑なdevice再生成を必須にせず、診断を出して安全に終了する。無限の再試行で問題を隠さない。

## 7. 実行証跡テンプレート

以下を検証ごとに記入する。ログや画面がない場合も、確認できたこととできていないことを区別する。

| 項目 | 値 |
| --- | --- |
| 検証ID / マイルストーン | 未記入 |
| 対象の変更 / commit | 未記入 |
| 実行日・担当 | 未記入 |
| OS / MSVC / SDK / CMake | 未確認 |
| GPU / ドライバ / WARPの有無 | 未確認 |
| 依存版 / shader profile | 未確認 |
| configure・build・実行command | 未実施 |
| 終了コード | 未実施 |
| 操作手順 / 入力データ | 未記入 |
| 期待結果 | 未記入 |
| 実際の結果 | 未実施 |
| Debug Layer / GBVの有効・無効 | 未実施 |
| error・warningと対処 | 未実施 |
| ログ / 画面 / captureの相対パス | 未取得 |
| 判定 | unverified |
| 未実行の項目と理由 | 未記入 |
| 次に必要な確認 | 未記入 |

証跡は`docs/validation/`等の決めた場所に保存し、巨大なPIX captureやビルド生成物を無条件にリポジトリへ追加しない。外部保存した場合は、チームが辿れる所在と対応commitを記録する。

## 8. 完了の定義

マイルストーンを完了にする条件は次のとおり。

- [ROADMAP.md](ROADMAP.md)の対象機能と受け入れ条件を満たす。
- 変更に関係するビルド・振る舞いを検証し、失敗を修正している。
- DX12や外部統合を含む場合、必要なWindows実行とGPU診断の証跡がある。
- 実装、設計図、ファイル配置、教材中のAPI名が一致している。
- 未実装・未検証・対象外の項目を明記し、[STATUS.md](STATUS.md)へ反映している。

描画機能の実装が完成してもWindows実行ができなければ、「実装済み・実機未検証」と報告する。文書・設定だけの段階には、存在しないGPUテストを要求しない。必要な確認で十分な証拠が得られたら、無関係なテストを増やさず次の作業へ進む。

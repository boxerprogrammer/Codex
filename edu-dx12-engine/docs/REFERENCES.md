# 一次資料と確認範囲

調査日：2026-11-25。これは実装時に照合するための参照集。**ページを確認したことは、依存の相互互換性やWindowsでの動作を検証したことを意味しない。** リンク先が更新される場合があるため、採用時に版・commit・取得日を記録する。

## Codexの指示とスキル

| 資料 | 確認すること |
| --- | --- |
| [OpenAI：Custom instructions with AGENTS.md](https://learn.chatgpt.com/docs/agent-configuration/agents-md) | `AGENTS.md`の探索、配置、指示の範囲 |
| [OpenAI：Build skills](https://learn.chatgpt.com/docs/build-skills) | `.agents/skills`、`SKILL.md`のname/description、呼び出し |
| [OpenAI：Using skills to accelerate OSS maintenance](https://developers.openai.com/blog/skills-agents-sdk) | 共通規則と再利用ワークフローを分離する実例 |

## DirectX 12・数学・shader

| 資料 | 主な用途 |
| --- | --- |
| [Microsoft：DirectX Graphics Samples](https://github.com/microsoft/DirectX-Graphics-Samples) | 導入時に対象版の最小sampleを参照 |
| [Microsoft：CPU/GPU synchronization等](https://learn.microsoft.com/en-us/windows/win32/direct3d12/porting-from-direct3d-11-to-direct3d-12) | CPU所有とGPU完了の区別 |
| [Microsoft：Resource barriers](https://learn.microsoft.com/en-us/windows/win32/direct3d12/using-resource-barriers-to-synchronize-resource-states-in-direct3d-12) | resource stateとtransition |
| [Microsoft：CommandAllocator::Reset](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12commandallocator-reset) | allocator再利用の前提 |
| [Microsoft：Descriptor tables](https://learn.microsoft.com/en-us/windows/win32/direct3d12/advanced-use-of-descriptor-tables) | descriptorの書換えと寿命 |
| [Microsoft：Creating descriptors](https://learn.microsoft.com/en-us/windows/win32/direct3d12/creating-descriptors) | RTV/CBV等の設定例 |
| [Microsoft：Root signature 1.1](https://learn.microsoft.com/en-us/windows/win32/direct3d12/root-signature-version-1-1) | static/volatile指定の約束 |
| [Microsoft：DirectXMath](https://learn.microsoft.com/en-us/windows/win32/dxmath/pg-xnamath-getting-started) | 行列・ベクトル規約 |
| [Microsoft：HLSL mul](https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-mul) | 行列積の意味 |
| [Microsoft：DirectXShaderCompiler](https://github.com/microsoft/DirectXShaderCompiler) | DXCの採用版とbuild手順 |
| [Microsoft：ResizeBuffers](https://learn.microsoft.com/en-us/windows/win32/api/dxgi/nf-dxgi-idxgiswapchain-resizebuffers) | リサイズ前の参照解放 |

## 構造・マテリアル

| 資料 | 主な用途 |
| --- | --- |
| [Godot：Key concepts](https://docs.godotengine.org/en/stable/getting_started/introduction/key_concepts_overview.html) | Node・Scene・Resource等の考え方 |
| [Godot：Nodes and scene instances](https://docs.godotengine.org/en/stable/tutorials/scripting/nodes_and_scene_instances.html) | 階層とインスタンスの参考 |
| [Godot：Resources](https://docs.godotengine.org/en/stable/tutorials/scripting/resources.html) | 共有する素材という役割 |
| [Khronos：glTF 2.0](https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html) | metallic-roughnessと材質データの参考。モデルloader導入を意味しない |

NodeのC++所有方式、RenderGraphの限定仕様、PBRの実装範囲は**本パックの設計案**。Godotの内部実装をそのまま再現する指示ではない。

## ライブラリ統合

| 資料 | 主な用途 |
| --- | --- |
| [Dear ImGui：Getting Started](https://github.com/ocornut/imgui/wiki/Getting-Started) | backendの導入経路 |
| [Dear ImGui：DX12 backend](https://github.com/ocornut/imgui/blob/master/backends/imgui_impl_dx12.h) | 採用commitの初期化・descriptor API |
| [Lua：Version history](https://www.lua.org/versions.html) | 版間互換性の前提 |
| [sol2：protected_function](https://sol2.readthedocs.io/en/latest/api/protected_function.html) | エラーを受け取る呼出し |
| [sol2：Ownership](https://sol2.readthedocs.io/en/latest/tutorial/ownership.html) | C++とLua間の寿命 |
| [Microsoft：XAudio2初期化](https://learn.microsoft.com/en-us/windows/win32/xaudio2/how-to--initialize-xaudio2) | deviceとvoiceの導入 |
| [Microsoft：SubmitSourceBuffer](https://learn.microsoft.com/en-us/windows/win32/api/xaudio2/nf-xaudio2-ixaudio2sourcevoice-submitsourcebuffer) | 再生bufferの保持期間 |
| [Microsoft：DestroyVoice](https://learn.microsoft.com/en-us/windows/win32/api/xaudio2/nf-xaudio2-ixaudio2voice-destroyvoice) | voiceの破棄規則 |
| [CRI：ADX LE](https://game.criware.jp/products/adx-le/) | Native SDK・利用条件の確認先 |
| [Effekseer：C++ガイド](https://github.com/effekseer/Effekseer/blob/master/docs/Help_Cpp/18x/Guide_Cpp_En.md) | C++連携・公式sampleへの案内 |
| [Effekseer：Code overview](https://github.com/effekseer/Effekseer/blob/master/docs/Development/CodeOverview.md) | renderer・LLGIの位置付け |

EffekseerのDX12 sampleは調査時に直接取得の制限があったため、具体的API署名とCMake targetの組み合わせは未確認。M12で採用releaseの実体を調べる。CRIの本プロジェクトへの利用許諾・再配布条件、各依存の相互動作は未検証。

## ビルドと診断

| 資料 | 主な用途 |
| --- | --- |
| [CMake：Presets](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html) | 再現可能なconfigure/build設定 |
| [Microsoft：C++標準オプション](https://learn.microsoft.com/en-us/cpp/build/reference/std-specify-language-standard-version?view=msvc-170) | `/std:c++20` |
| [Microsoft：EnableDebugLayer](https://learn.microsoft.com/en-us/windows/win32/api/d3d12sdklayers/nf-d3d12sdklayers-id3d12debug-enabledebuglayer) | device作成前の診断設定 |
| [Microsoft：GPU-Based Validation](https://learn.microsoft.com/en-us/windows/win32/direct3d12/using-d3d12-debug-layer-gpu-based-validation) | GPU時間軸の検査 |
| [Microsoft：PIX GPU Captures](https://devblogs.microsoft.com/pix/gpu-captures/) | frameの観察 |
| [Microsoft：DRED](https://learn.microsoft.com/en-us/windows/win32/direct3d12/use-dred) | device removed時の診断 |

## 採用前に確認すること

参照URL、採用版、実際のヘッダ・sample、使用したコンパイラ、Windows SDK、GPU、結果を[DEPENDENCIES.md](DEPENDENCIES.md)と[VALIDATION.md](VALIDATION.md)へ記録する。リンク集だけを根拠に「対応済み」とはしない。

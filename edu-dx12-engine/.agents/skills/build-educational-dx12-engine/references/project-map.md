# プロジェクト文書の対応

文書が存在する場合に参照する。これらのファイルがないプロジェクトでは、既存の構成を尊重し、不足する情報だけを確認する。

| 調べる内容 | 既定の相対パス |
| --- | --- |
| 常に守る規則 | `AGENTS.md` |
| 目的・確定要件・対象外 | `docs/PROJECT.md` |
| 今の段階・検証待ち・次の一手 | `docs/STATUS.md` |
| 段階の前提と受入条件 | `docs/ROADMAP.md` |
| 責務・所有・Node・Scene・Resource | `docs/ARCHITECTURE.md` |
| DX12・RenderGraph・PBR・色・行列 | `docs/RENDERING.md` |
| 環境・依存版・互換性・ライセンス | `docs/DEPENDENCIES.md` |
| C++20規約とエラー処理 | `docs/CODING_STANDARDS.md` |
| build・実行・リスクに対応する検証 | `docs/VALIDATION.md` |
| 暫定判断・採用理由 | `docs/DECISIONS.md` |
| 教材構成・前提・評価 | `docs/CURRICULUM.md` |
| 初出用語の説明 | `docs/GLOSSARY.md` |
| 教材・演習の形式 | `docs/templates/LESSON_TEMPLATE.md`, `docs/templates/EXERCISE_TEMPLATE.md` |
| 一次資料 | `docs/REFERENCES.md` |

準備パックの標準配置は`engine/core`, `engine/platform/win32`, `engine/render/d3d12`, `engine/render/graph`, `engine/scene`, `engine/resource`, `engine/integrations/{imgui,lua,audio,effekseer}`, `samples`, `shaders`, `tests`, `lessons/{students,teachers}`。これは計画であり、すべてが既に存在するとは扱わない。

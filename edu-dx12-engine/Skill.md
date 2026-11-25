# Skill.md — 作業手順の入口

プロジェクト用スキルの正本は、次のファイルです。

[build-educational-dx12-engine / SKILL.md](.agents/skills/build-educational-dx12-engine/SKILL.md)

`Skill.md`は案内です。スキルはYAMLの`name`・`description`を持つ**`SKILL.md`**として、スキル用ディレクトリに置きます。共通規則は[AGENTS.md](AGENTS.md)にまとめています。

## できること

- [開発モード]：段階を確認し、設計比較、C++20 / HLSL実装、依存統合、検証、記録を進める。
- [教材執筆モード]：検証済みの設計・コードに対応する学習目標、解説、図解、演習、講師用解答を作る。
- 障害調査：エラーメッセージと実際の環境から原因を絞り、小さな修正と再検証を行う。

## Codexへの呼び出し例

```text
$build-educational-dx12-engine
AGENTS.md と docs/STATUS.md を読み、prompts/START_DEVELOPMENT.md に従ってM00を進めてください。
```

スキル一覧で見つからない場合は、上記の`SKILL.md`をパスで指定して読むよう依頼できます。ChatGPT側で追加したスキルが、別のPCのCodexへ自動的に配布されるとは仮定せず、このプロジェクトのファイルを配置してください。

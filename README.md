# xylophone_arduino_test

楽器 Arduino 共通プログラム および 単体テスト（タスク I45）のリポジトリです。
Arduino UNO R4 WiFi を使った赤外線同期アンサンブル演奏システムにおける、各楽器 Arduino のスケッチをまとめています。

---

## ファイル一覧

| ファイル | 楽器 | roundOffset | 備考 |
|---|---|---|---|
| `xylophone.ino` | 木琴 | 12.0 拍 | 本番用スケッチ |
| `trumpet.ino` | トランペット | 0.0 拍 | 本番用スケッチ |
| `organ.ino` | オルガン | 8.0 拍 | 本番用スケッチ |
| `harp.ino` | ハープ | 16.0 拍 | 本番用スケッチ |
| `drum.ino` | ドラム | 4.0 拍 | 本番用スケッチ |
| `xylophone_test.ino` | 木琴（テスト用） | — | 単体テスト I45 |

---

## システム概要

赤外線（IR）通信を使って指揮 Arduino から各楽器 Arduino へ BPM とタイミングを送信し、  
各楽器が自律的に楽譜を再生する合奏システムです。

```
指揮 Arduino  ──[IR]──▶  楽器 Arduino（木琴 / トランペット / オルガン / ハープ / ドラム）
                              │
                              └─[USB Serial]──▶ PC（音源ソフト）
```

---

## 本番用スケッチの仕様

### 楽器ごとの変更箇所

各 `.ino` ファイルの冒頭 **INSTRUMENT SETTINGS** セクションのみが楽器ごとに異なります。

```cpp
static const char* INSTRUMENT_NAME = "xylophone"; // 楽器名（JSON出力に使用）
static const float roundOffset     = 12.0f;       // 受信タイミングへの加算オフセット（拍）
```

| パラメータ | 説明 |
|---|---|
| `INSTRUMENT_NAME` | USBシリアルへ出力するJSON の `"instrument"` フィールドに使われる楽器名 |
| `roundOffset` | 指揮Arduinoから受信したタイミングに加算するオフセット（拍単位）。楽器ごとの音の遅れを補正する |

### 動作状態（State）

| 状態 | 説明 |
|---|---|
| `IDLE` | 待機中。BPM=0 受信・タイムアウト・曲終端で遷移 |
| `PLAYING` | IR受信で再生中 |
| `DEMO` | DEMO_PIN（GPIO3）をLOWにすると起動するオフライン再生モード（BPM=120固定） |

### USBシリアル出力フォーマット

再生タイミングに達した音符を JSON 形式でシリアル出力します。

```json
{"instrument":"xylophone","length":1.00,"pitch":"C4","volume":80}
```

### LED マトリクス表示

Arduino UNO R4 WiFi 内蔵の 8×12 LED マトリクスに音符をスクロール表示します。

| ピッチ | 表示行（row） |
|---|---|
| オクターブ 7（例: C7） | row 0 |
| オクターブ 6（例: C6） | row 1 |
| … | … |
| オクターブ 1（例: C1） | row 6 |
| DR（ドラム） | row 7 |

---

## 単体テスト（xylophone_test.ino）

タスク **I45** に対応した木琴 Arduino の単体テストスケッチです。  
実機に書き込んで Arduino IDE のシリアルモニタ（115200 baud）で結果を確認します。

### テスト項目

| テストID | 内容 |
|---|---|
| Test4〜7 | BPM・タイミング計算の正確性 |
| Test12〜14, 16 | 停止制御（BPM=0 / タイムアウト / 曲終端 / 変数リセット） |
| Test17〜21 | JSON出力の正確性・重複送信防止 |
| Test22 | pitchToRow() マッピング |
| Test24〜26 | LED マトリクス表示（目視確認） |
| Test29 | 非ブロッキング処理（1000回ポーリング < 100ms） |

### 実行例

```
=== 楽器Arduino 単体テスト開始 (I45) ===

--- JSON出力テスト (Test17-21) ---
[PASS] Test17: phoneme[0]全フィールド
[PASS] Test18: timing=0.0 境界値
[PASS] Test20: JSON構文正確性
[PASS] Test21: 重複送信防止

--- タイミングテスト (Test4-11) ---
[PASS] Test4: BPM120で1秒≒2拍

--- 停止制御テスト (Test12-16) ---
[PASS] Test12: BPM=0でIDLE遷移
[PASS] Test13: タイムアウトでIDLE遷移
[PASS] Test14: 曲終端でIDLE遷移
[PASS] Test16: 停止時変数リセット

--- LED マトリクステスト (Test22-25) ---
[PASS] Test22a: C4 → row3
[PASS] Test22b: C6 → row1

--- 非ブロッキングテスト (Test29) ---
[PASS] Test29: 1000回ポーリングが100ms未満

=== 単体テスト完了 ===
```

---

## 依存ライブラリ

- `irtp.h` — 赤外線通信ライブラリ（プロジェクト独自）
- `Arduino_LED_Matrix.h` — Arduino UNO R4 内蔵 LED マトリクス制御（公式ライブラリ）

---

## ピン配置

| ピン | 用途 |
|---|---|
| GPIO 2 | IR受信（INPUT） |
| GPIO 3 | デモモード起動（INPUT_PULLUP、LOWで起動） |
| USB | シリアル出力（115200 baud） |

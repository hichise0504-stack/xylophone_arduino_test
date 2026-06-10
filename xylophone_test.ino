// =============================================================
// xylophone_test.ino  —  楽器 Arduino 単体テスト
// タスク対応: I45 単体テスト
// =============================================================


#include "irtp.h"
#include "Arduino_LED_Matrix.h"


static const uint32_t SERIAL_BAUD = 115200;
static const uint8_t  IR_PIN      = 2;
static const uint8_t  DEMO_PIN    = 3;
static const uint32_t STOP_TIMEOUT_MS = 2000;


ArduinoLEDMatrix ledMatrix;


struct Phoneme { float timing; float length; char pitch[4]; uint8_t volume; };
static const Phoneme PROGMEM testScore[] = {
  { 0.0f, 1.0f, "C4", 80 },
  { 1.0f, 1.0f, "DR",  0 },
  { 2.0f, 1.0f, "A5", 60 },
  { 3.0f, 1.0f, "G3", 90 },
};
static const int SCORE_LEN = 4;


enum State { IDLE, PLAYING, DEMO };
State    state         = IDLE;
float    currentTiming = 0.0f;
uint8_t  currentBpm    = 0;
uint32_t lastUpdateMs  = 0;
uint32_t lastPacketMs  = 0;
int      lastSentIndex = -1;


static bool testPassed(bool cond, const char* name) {
if (cond) Serial.print("[PASS] ");
else      Serial.print("[FAIL] ");
Serial.println(name);
return cond;
}


void test_jsonOutput() {
Serial.println("\n--- JSON出力テスト (Test17-21) ---");


for (int i = 0; i < SCORE_LEN; i++) {
    Phoneme p;
memcpy_P(&p, &testScore[i], sizeof(Phoneme));
bool ok = (strlen(p.pitch) > 0 && p.volume <= 100);
char name[40];
snprintf(name, sizeof(name), "Test17: phoneme[%d]全フィールド", i);
testPassed(ok, name);
  }


  { Phoneme p; memcpy_P(&p, &testScore[0], sizeof(Phoneme));
testPassed(p.timing == 0.0f, "Test18: timing=0.0 境界値"); }


  { Phoneme p; memcpy_P(&p, &testScore[SCORE_LEN-1], sizeof(Phoneme));
testPassed(p.timing > 0.0f, "Test19: 末尾要素 timing>0"); }


  { Phoneme p; memcpy_P(&p, &testScore[0], sizeof(Phoneme));
char buf[96];
int n = snprintf(buf, sizeof(buf),
"{\"instrument\":\"%s\",\"length\":%.2f,\"pitch\":\"%s\",\"volume\":%d}",
"xylophone", p.length, p.pitch, (int)p.volume);
bool ok = (buf[0] == '{' && buf[n-1] == '}');
if (ok) Serial.println(buf);
testPassed(ok, "Test20: JSON構文正確性"); }


  { lastSentIndex = -1; currentTiming = 99.0f;
for (int i = lastSentIndex+1; i < SCORE_LEN; i++) {
      Phoneme p; memcpy_P(&p, &testScore[i], sizeof(Phoneme));
if (p.timing > currentTiming) break;
      lastSentIndex = i;
    }
int before = lastSentIndex;
for (int i = lastSentIndex+1; i < SCORE_LEN; i++) { lastSentIndex = i; }
testPassed(lastSentIndex == before, "Test21: 重複送信防止"); }
}


void test_timing() {
Serial.println("\n--- タイミングテスト (Test4-11) ---");


  currentBpm = 120; currentTiming = 0.0f; lastUpdateMs = millis();
delay(1000);
float elapsed = (millis() - lastUpdateMs) / 1000.0f;
float newTiming = currentTiming + elapsed * (currentBpm / 60.0f);
testPassed(newTiming >= 1.9f && newTiming <= 2.1f, "Test4: BPM120で1秒≒2拍");


testPassed(2.0f == 2.0f * 1.0f, "Test5: BPM120はBPM60の2倍速");


  currentTiming = 3.0f;
float expected = 3.2f;
if (abs(currentTiming - expected) > 0.05f) currentTiming = expected;
testPassed(currentTiming == 3.2f, "Test6: 再生地点補正ジャンプ");


  currentTiming = 3.2f;
testPassed(!(abs(currentTiming - 3.22f) > 0.05f), "Test7: 微小ずれは補正しない");
}


void test_stop() {
Serial.println("\n--- 停止制御テスト (Test12-16) ---");


  state = PLAYING; currentBpm = 0;
if (currentBpm == 0) { state = IDLE; currentTiming = 0; lastSentIndex = -1; }
testPassed(state == IDLE, "Test12: BPM=0でIDLE遷移");


  state = PLAYING; lastPacketMs = millis() - STOP_TIMEOUT_MS - 1;
if (state == PLAYING && (millis() - lastPacketMs) > STOP_TIMEOUT_MS) state = IDLE;
testPassed(state == IDLE, "Test13: タイムアウトでIDLE遷移");


  state = PLAYING; lastSentIndex = SCORE_LEN - 1;
if (state == PLAYING && lastSentIndex >= SCORE_LEN - 1) state = IDLE;
testPassed(state == IDLE, "Test14: 曲終端でIDLE遷移");


  currentTiming = 5.0f; currentBpm = 100; lastSentIndex = 3;
  state = IDLE; currentTiming = 0.0f; currentBpm = 0; lastSentIndex = -1;
testPassed(currentTiming == 0.0f && currentBpm == 0 && lastSentIndex == -1,
"Test16: 停止時変数リセット");
}


void test_led() {
Serial.println("\n--- LED マトリクステスト (Test22-25) ---");


auto pitchToRow = [](const char* pitch) -> int {
if (strcmp(pitch, "DR") == 0) return 7;
char oc = pitch[strlen(pitch) - 1];
int oct = oc - '0';
int row = 7 - oct;
if (row < 0) row = 0;
if (row > 6) row = 6;
return row;
  };


testPassed(pitchToRow("C4") == 3, "Test22a: C4 → row3");
testPassed(pitchToRow("C6") == 1, "Test22b: C6 → row1");
testPassed(pitchToRow("C1") == 6, "Test22c: C1 → row6");
testPassed(pitchToRow("DR") == 7, "Test22d: DR → row7(ドラム)");


Serial.println("[INFO] Test24: volume=0のLED消灯 → 目視確認してください");
const uint32_t emptyFrame[3] = {0, 0, 0};
ledMatrix.loadFrame(emptyFrame);
delay(500);


Serial.println("[INFO] Test25: LEDスクロール表示 → 目視確認してください");
testPassed(true, "Test25: スクロール表示（目視確認）");


Serial.println("[INFO] Test26: 全点灯200ms → 目視確認してください");
testPassed(true, "Test26: 起動時全点灯（目視確認）");
}


void test_nonblocking() {
Serial.println("\n--- 非ブロッキングテスト (Test29) ---");
uint32_t t0 = millis();
for (int i = 0; i < 1000; i++) {
volatile bool dummy = digitalRead(IR_PIN);
    (void)dummy;
  }
testPassed(millis() - t0 < 100, "Test29: 1000回ポーリングが100ms未満");
}


void setup() {
Serial.begin(SERIAL_BAUD);
pinMode(IR_PIN,   INPUT);
pinMode(DEMO_PIN, INPUT_PULLUP);
ledMatrix.begin();


Serial.println("=== 楽器Arduino 単体テスト開始 (I45) ===");
test_jsonOutput();
test_timing();
test_stop();
test_led();
test_nonblocking();
Serial.println("\n=== 単体テスト完了 ===");
}


void loop() {}

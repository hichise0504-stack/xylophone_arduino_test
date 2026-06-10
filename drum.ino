// =============================================================
// drum.ino  —  楽器 Arduino 共通プログラム（ドラム用）
// =============================================================

#include "irtp.h"
#include "Arduino_LED_Matrix.h"

// =============================================================
// ★ INSTRUMENT SETTINGS★  楽器ごとにここだけ変更する
// =============================================================
static const char* INSTRUMENT_NAME = "drum";
static const float roundOffset     = 4.0f;

// =============================================================
// 譜面格納（phoneme 構造体 ＋ 楽譜データ配列）
// =============================================================
struct Phoneme {
float    timing;
float    length;
char     pitch[4];
uint8_t  volume;
};

static const Phoneme PROGMEM score[] = {
  { 0.0f,  1.0f, "DR", 100 },
  { 1.0f,  1.0f, "DR",  60 },
  { 2.0f,  1.0f, "DR", 100 },
  { 3.0f,  1.0f, "DR",  60 },
  { 4.0f,  1.0f, "DR", 100 },
  { 5.0f,  1.0f, "DR",  60 },
  { 6.0f,  1.0f, "DR", 100 },
  { 7.0f,  1.0f, "DR",  60 },
  { 8.0f,  1.0f, "DR", 100 },
  { 9.0f,  1.0f, "DR",  60 },
  {10.0f,  1.0f, "DR", 100 },
  {11.0f,  1.0f, "DR",  60 },
  {12.0f,  1.0f, "DR", 100 },
  {13.0f,  1.0f, "DR",  60 },
  {14.0f,  1.0f, "DR", 100 },
  {15.0f,  1.0f, "DR",  60 },
  {16.0f,  1.0f, "DR", 100 },
  {17.0f,  1.0f, "DR",  60 },
  {18.0f,  1.0f, "DR", 100 },
  {19.0f,  1.0f, "DR",  60 },
  {20.0f,  1.0f, "DR", 100 },
  {21.0f,  1.0f, "DR",  60 },
  {22.0f,  1.0f, "DR", 100 },
  {23.0f,  1.0f, "DR",  60 },
};
static const int SCORE_LEN = sizeof(score) / sizeof(score[0]);

// =============================================================
// ピン定数・通信定数
// =============================================================
static const uint8_t  IR_PIN          = 2;
static const uint8_t  DEMO_PIN        = 3;
static const uint32_t SERIAL_BAUD     = 115200;
static const uint32_t STOP_TIMEOUT_MS = 2000;

// =============================================================
// 状態管理・グローバル変数
// =============================================================
enum State { IDLE, PLAYING, DEMO };

State    state         = IDLE;
float    currentTiming = 0.0f;
uint8_t  currentBpm    = 0;
uint32_t lastUpdateMs  = 0;
uint32_t lastPacketMs  = 0;
int      lastSentIndex = -1;

// LED フラッシュタイマー（非ブロッキング）
static uint32_t flashEndMs = 0;
static int      flashRow   = -1;

// =============================================================
// LED マトリクス
// =============================================================
ArduinoLEDMatrix ledMatrix;
static uint8_t ledFrame[8][12] = {};

static int pitchToRow(const char* pitch) {
if (strcmp(pitch, "DR") == 0) return 7;
char octChar = pitch[strlen(pitch) - 1];
int oct = octChar - '0';
int row = 7 - oct;
if (row < 0) row = 0;
if (row > 6) row = 6;
return row;
}

static void ledScroll(int row, bool flash) {
for (int r = 0; r < 8; r++) {
for (int c = 0; c < 11; c++) {
ledFrame[r][c] = ledFrame[r][c + 1];
    }
ledFrame[r][11] = 0;
  }
if (flash) {
for (int r = 0; r < 8; r++) ledFrame[r][11] = 1;
  } else {
ledFrame[row][11] = 1;
  }
ledMatrix.loadPixels(&ledFrame[0][0], 8 * 12);
}

static void ledClear() {
memset(ledFrame, 0, sizeof(ledFrame));
ledMatrix.loadPixels(&ledFrame[0][0], 8 * 12);
}

static void ledAllOn() {
memset(ledFrame, 1, sizeof(ledFrame));
ledMatrix.loadPixels(&ledFrame[0][0], 8 * 12);
}

// =============================================================
// 停止処理
// =============================================================
static void stopPlayback() {
  state         = IDLE;
  currentTiming = 0.0f;
  currentBpm    = 0;
  lastSentIndex = -1;
ledClear();
}

// =============================================================
// 赤外線受信コールバック
// =============================================================
IrtpReceiver* receiver;

void onCapsuleReceived(int result, uint8_t* data, uint16_t length) {
if (result != 0) return;

  lastPacketMs = millis();

uint8_t bpm = data[0];
float   recvTiming;
memcpy(&recvTiming, data + 1, sizeof(float));

if (bpm == 0) {
stopPlayback();
return;
  }

if (state == IDLE || state == DEMO) {
    currentBpm    = bpm;
    currentTiming = recvTiming + roundOffset;
    lastUpdateMs  = millis();
    lastSentIndex = -1;
    state         = PLAYING;
return;
  }

if (currentBpm != bpm) currentBpm = bpm;
float expectedTiming = recvTiming + roundOffset;
if (abs(currentTiming - expectedTiming) > 0.05f) {
    currentTiming = expectedTiming;
    lastUpdateMs  = millis();
  }
}

// =============================================================
// 内部再生地点の自律更新
// =============================================================
static void updateTiming() {
if ((state != PLAYING && state != DEMO) || currentBpm == 0) return;
uint32_t now = millis();
float elapsed = (now - lastUpdateMs) / 1000.0f;
  currentTiming += elapsed * (currentBpm / 60.0f);
  lastUpdateMs = now;
}

// =============================================================
// 音要素検索 + USB シリアル出力
// =============================================================
static void sendPendingPhonemes() {
for (int i = lastSentIndex + 1; i < SCORE_LEN; i++) {
    Phoneme p;
memcpy_P(&p, &score[i], sizeof(Phoneme));
if (p.timing > currentTiming) break;

char buf[96];
snprintf(buf, sizeof(buf),
"{\"instrument\":\"%s\",\"length\":%.2f,\"pitch\":\"%s\",\"volume\":%d}",
             INSTRUMENT_NAME, p.length, p.pitch, (int)p.volume);
Serial.println(buf);

int row = pitchToRow(p.pitch);
if (p.volume == 0) {
ledClear();
    } else {
ledScroll(row, true);
      flashRow   = row;
      flashEndMs = millis() + 30;
    }

    lastSentIndex = i;
  }
}

// =============================================================
// タイムアウト・曲終端チェック
// =============================================================
static void checkTimeout() {
if (state != PLAYING) return;
if (millis() - lastPacketMs > STOP_TIMEOUT_MS) stopPlayback();
}

static void checkEndOfScore() {
if (state != PLAYING) return;
if (lastSentIndex >= SCORE_LEN - 1) stopPlayback();
}

// =============================================================
// デモ再生モード
// =============================================================
static void handleDemoPin() {
if (state != IDLE) return;
if (digitalRead(DEMO_PIN) == LOW) {
    currentTiming = 0.0f;
    currentBpm    = 120;
    lastUpdateMs  = millis();
    lastSentIndex = -1;
    state         = DEMO;
  }
}

// =============================================================
// setup / loop
// =============================================================
void setup() {
Serial.begin(SERIAL_BAUD);
pinMode(IR_PIN,   INPUT);
pinMode(DEMO_PIN, INPUT_PULLUP);

ledMatrix.begin();
ledAllOn();
delay(200);
ledClear();

  receiver = new IrtpReceiver(IR_PIN);
receiver->setReceiveCallback(onCapsuleReceived);
}

void loop() {
receiver->update();
handleDemoPin();
updateTiming();

if (state == PLAYING || state == DEMO) {
sendPendingPhonemes();
  }

  // LED フラッシュ終了処理（非ブロッキング）
if (flashRow >= 0 && millis() >= flashEndMs) {
ledScroll(flashRow, false);
    flashRow = -1;
  }

checkTimeout();
checkEndOfScore();
}

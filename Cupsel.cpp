#include "Cupsel.h"


Cupsel::Cupsel(BitBuffer& data) {
    // 非const参照から内部にバッファを安全にコピー
uint16_t sz = data.size();
for (uint16_t i = 0; i < sz; i++) {
payload.pushBit(data.getBit(i));
    }
}


BitBuffer Cupsel::getBits() {
    BitBuffer raw;
raw.pushUint8(START_SIGNAL, 4);
for (uint16_t i = 0; i < payload.size(); i++) {
raw.pushBit(payload.getBit(i));
    }
raw.pushUint8(END_SIGNAL, 4);
return raw;
}


bool Cupsel::parse(BitBuffer& raw, uint16_t& outConsumed, BitBuffer& outPayload) {
outPayload.clear();
if (raw.size() < 8) return false;


uint8_t startSign = 0;
raw.getUint8(0, 4, startSign);
if (startSign != START_SIGNAL) return false;


for (uint16_t i = 4; i <= raw.size() - 4; i++) {
uint8_t checkEnd = 0;
raw.getUint8(i, 4, checkEnd);
if (checkEnd == END_SIGNAL && i + 4 == raw.size()) {
for (uint16_t j = 4; j < i; j++) {
outPayload.pushBit(raw.getBit(j));
            }
            outConsumed = i + 4;
return true;
        }
    }
return false;
}

#ifndef CUPSEL_H
#define CUPSEL_H


#include "BitBuffer.h"


class Cupsel {
private:
    BitBuffer payload;
static const uint8_t START_SIGNAL = 0b1001; // 仕様書: 開始信号 1001
static const uint8_t END_SIGNAL   = 0b0110; // 仕様書: 終了信号 0110


public:
Cupsel(BitBuffer& data); // 仕様書表3.9準拠: 非const参照
    BitBuffer getBits();     // 仕様書表3.9準拠
static bool parse(BitBuffer& raw, uint16_t& outConsumed, BitBuffer& outPayload);
};


#endif

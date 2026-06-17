#ifndef BIT_BUFFER_H
#define BIT_BUFFER_H


#include <stdint.h>


class BitBuffer {
private:
uint8_t data[17]; // 仕様書表3.8: 上限136ビット固定
uint16_t bitSize;


public:
BitBuffer();
uint16_t size() const;
bool pushBit(bool bit);
bool getBit(uint16_t index) const;
bool pushUint8(uint8_t value, uint8_t bitLength);
bool getUint8(uint16_t startIdx, uint8_t bitLength, uint8_t& out) const;
void clear();
};


#endif

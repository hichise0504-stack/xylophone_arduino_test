#include "BitBuffer.h"
#include <cstring>


BitBuffer::BitBuffer() { clear(); }
uint16_t BitBuffer::size() const { return bitSize; }
void BitBuffer::clear() { memset(data, 0, sizeof(data)); bitSize = 0; }


bool BitBuffer::pushBit(bool bit) {
if (bitSize >= 136) return false;
if (bit) { data[bitSize / 8] |= (1 << (7 - (bitSize % 8))); }
    bitSize++;
return true;
}


bool BitBuffer::getBit(uint16_t index) const {
if (index >= bitSize) return false;
return (data[index / 8] & (1 << (7 - (index % 8)))) != 0;
}


bool BitBuffer::pushUint8(uint8_t value, uint8_t bitLength) {
for (int i = bitLength - 1; i >= 0; i--) {
if (!pushBit((value >> i) & 0x01)) return false;
    }
return true;
}


bool BitBuffer::getUint8(uint16_t startIdx, uint8_t bitLength, uint8_t& out) const {
if (startIdx + bitLength > bitSize) return false;
    out = 0;
for (uint8_t i = 0; i < bitLength; i++) {
        out <<= 1;
if (getBit(startIdx + i)) out |= 1;
    }
return true;
}

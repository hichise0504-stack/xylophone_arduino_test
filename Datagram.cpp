#include "Datagram.h"
#include <cstring>


Datagram::Datagram(BitBuffer& data) {
uint16_t sz = data.size();
for (uint16_t i = 0; i < sz; i++) {
dataBuffer.pushBit(data.getBit(i));
    }
}


uint8_t Datagram::getChecksum() {
uint32_t totalSum = 0;
for (uint16_t i = 0; i < dataBuffer.size(); i += 8) {
uint8_t byteVal = 0;
uint8_t rem = (dataBuffer.size() - i >= 8) ? 8 : (dataBuffer.size() - i);
if (!dataBuffer.getUint8(i, rem, byteVal)) break;
uint32_t shifted = byteVal;
if (rem < 8) shifted <<= (8 - rem);
        totalSum += shifted;
    }
return (uint8_t)(totalSum % 256);
}


Cupsel Datagram::getCupsel() {
    BitBuffer dgmBuffer;
dgmBuffer.pushUint8(getChecksum(), 8);
for (uint16_t i = 0; i < dataBuffer.size(); i++) {
dgmBuffer.pushBit(dataBuffer.getBit(i));
    }
return Cupsel(dgmBuffer);
}


int8_t Datagram::parse(BitBuffer& payload, uint8_t* outData, uint16_t& outLength) {
if (payload.size() < 8) return -1;


uint8_t receivedChecksum = 0;
payload.getUint8(0, 8, receivedChecksum);


uint32_t calculatedSum = 0;
    outLength = payload.size() - 8;


for (uint16_t i = 8; i < payload.size(); i += 8) {
uint8_t byteVal = 0;
uint8_t rem = (payload.size() - i >= 8) ? 8 : (payload.size() - i);
uint32_t shifted = 0;
payload.getUint8(i, rem, byteVal);
        shifted = byteVal;
if (rem < 8) shifted <<= (8 - rem);
        calculatedSum += shifted;
    }
uint8_t finalChecksum = (uint8_t)(calculatedSum % 256);


memset(outData, 0, (outLength + 7) / 8);
for (uint16_t i = 8; i < payload.size(); i++) {
if (payload.getBit(i)) {
outData[(i - 8) / 8] |= (1 << (7 - ((i - 8) % 8)));
        }
    }


if (finalChecksum == receivedChecksum) return 0;
return 17;
}

#ifndef DATAGRAM_H
#define DATAGRAM_H


#include "BitBuffer.h"
#include "Cupsel.h"


class Datagram {
private:
    BitBuffer dataBuffer;


public:
Datagram(BitBuffer& data);
    Cupsel getCupsel();
uint8_t getChecksum();


static int8_t parse(BitBuffer& payload, uint8_t* outData, uint16_t& outLength);
};


#endif

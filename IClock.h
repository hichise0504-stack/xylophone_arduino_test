#ifndef ICLOCK_H
#define ICLOCK_H


#include <stdint.h>


class IClock {
public:
virtual ~IClock() {}
virtual uint32_t millis() = 0;
virtual uint32_t micros() = 0;
virtual void delayMicros(uint32_t us) = 0;
};


#endif

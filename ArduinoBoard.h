#ifndef ARDUINOBOARD_H
#define ARDUINOBOARD_H


#include <Arduino.h>
#include "ISenderBoard.h"
#include "IReceiverBoard.h"
#include "IClock.h"


class ArduinoBoard : public ISenderBoard, public IReceiverBoard, public IClock {
private:
uint8_t _pin;
bool _outputMode;
public:
ArduinoBoard(uint8_t pin) : _pin(pin), _outputMode(false) {}
void begin() override {
pinMode(_pin, OUTPUT);
digitalWrite(_pin, LOW);
        _outputMode = true;
    }
void beginInput() {
pinMode(_pin, INPUT);
        _outputMode = false;
    }
uint8_t pin() const { return _pin; }
void ledWrite(IrtpPinStatus value) override {
if (!_outputMode) begin();
digitalWrite(_pin, value == IRTP_HIGH ? HIGH : LOW);
    }
void carrierOn() override {
if (!_outputMode) begin();
tone(_pin, 38000);
    }
void carrierOff() override {
noTone(_pin);
digitalWrite(_pin, LOW);
    }
bool read() override {
if (_outputMode) beginInput();
return digitalRead(_pin);
    }
uint32_t millis() override { return ::millis(); }
uint32_t micros() override { return ::micros(); }
void delayMicros(uint32_t us) override { delayMicroseconds(us); }
};


#endif

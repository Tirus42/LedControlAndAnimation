#pragma once

#include <ILedStripWithStorage.h>

#include <vector>
#include <Arduino.h>

/**
* LedStrip implementation for leds using the APA102 controller.
* They use two signals, clock + data.
*
* This implementation uses bit banging to transmit the data to the leds.
*/
class LedStrip_APA102 : public ILedStripWithStorage {
    private:
        std::vector<uint8_t> sendBuffer;
        uint16_t countLeds;
        uint16_t pinClock;
        uint16_t pinData;

        static void WriteGPIO(const uint8_t* ptr, size_t length, uint16_t pinClock, uint16_t pinData) {
            for (size_t i = 0; i < length; ++i) {
                uint8_t byte = ptr[i];

                for (int j = 7; j >= 0; j--) {
                    bool state = byte & (1 << j);

                    digitalWrite(pinData, state);
                    digitalWrite(pinClock, 1);
                    digitalWrite(pinClock, 0);
                }
            }
        }

        void writeGPIO() {
            WriteGPIO(sendBuffer.data(), sendBuffer.size(), pinClock, pinData);
        }

    public:
        LedStrip_APA102(uint16_t countLeds, uint16_t pinClock, uint16_t pinData) :
            ILedStripWithStorage(),
            sendBuffer(4 + countLeds * 4),
            countLeds(countLeds),
            pinClock(pinClock),
            pinData(pinData) {

            // Set initial value (including header byte)
            clear();

            pinMode(pinClock, OUTPUT);
            pinMode(pinData, OUTPUT);
        }

        virtual void updateLeds() override {
            writeGPIO();
        }

        void setLed(ledoffset_t index, RGBW color, uint8_t brightness, bool flush = false) {
            size_t offset = 4 + index * 4;

            sendBuffer[offset + 0] = (0x07 << 5) | (brightness & 0b11111);
            sendBuffer[offset + 1] = color.b;
            sendBuffer[offset + 2] = color.g;
            sendBuffer[offset + 3] = color.r;

            if (flush)
                updateLeds();
        }

        virtual void setLed(ledoffset_t index, RGBW color, bool flush = false) override {
            uint8_t brightness = ((color.r > 0) | (color.g > 0) | (color.b > 0)) * 31;

            setLed(index, color, brightness, flush);
        }

        virtual RGBW getLed(ledoffset_t index) const override {
            size_t offset = 4 + index * 4;

            uint8_t r = sendBuffer[offset + 3];
            uint8_t g = sendBuffer[offset + 2];
            uint8_t b = sendBuffer[offset + 1];

            return RGBW(r, g, b, 0);
        }

        virtual ledoffset_t getLedCount() const override {
            return countLeds;
        }
};

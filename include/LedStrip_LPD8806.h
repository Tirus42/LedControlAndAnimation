#pragma once

#include "LedBufferStorage.h"

#include <array>
#include <Arduino.h>

/**
* LedStrip implementation for leds using the LPD8806 controller.
* They use two signals, clock + data.
*
* This implementation uses bit banging to transmit the data to the leds.
* Also it directly applies gamma correction before sending the data to the leds.
*/
class LedStrip_LPD8806 : public LedBufferStorage {
    private:
        std::vector<uint8_t> sendBuffer;
        uint16_t pinClock;
        uint16_t pinData;

        static std::array<uint8_t, 256> CreateGammaTable() {
            std::array<uint8_t, 256> table;

            for (uint32_t i = 0; i < 256; ++i) {
                table[i] = 0x80 | (uint32_t)(pow((float)i / 255.0f, 2.5f) * 127.0 + 0.5f);
            }

            return table;
        }

        // TODO: Extract to own header
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

    public:
        LedStrip_LPD8806(ledoffset_t countLeds, uint16_t pinClock, uint16_t pinData) :
            LedBufferStorage(countLeds),
            sendBuffer(countLeds * 3 + 3),
            pinClock(pinClock),
            pinData(pinData) {

            pinMode(pinClock, OUTPUT);
            pinMode(pinData, OUTPUT);
        }

        virtual void updateLeds() override {
            WriteGPIO(sendBuffer.data(), sendBuffer.size(), pinClock, pinData);
        }

        virtual void setLed(ledoffset_t index, RGBW color, bool flush = false) override {
            color.w = 0;    // This led strip does not support the W component

            LedBufferStorage::setLed(index, color, flush);

            sendBuffer[index * 3 + 0] = getGammaTable()[color.r];
            sendBuffer[index * 3 + 1] = getGammaTable()[color.g];
            sendBuffer[index * 3 + 2] = getGammaTable()[color.b];

            if (flush) {
                updateLeds();
            }
        }

        const std::array<uint8_t, 256>& getGammaTable() const {
            static std::array<uint8_t, 256> GammaTable = CreateGammaTable();
            return GammaTable;
        }
};

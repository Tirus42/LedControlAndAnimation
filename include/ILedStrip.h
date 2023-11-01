#pragma once

#include <stdint.h>

#include "RGBW.h"

typedef uint8_t ledoffset_t;

class ILedStrip {
    public:
        virtual ledoffset_t getLedCount() const = 0;

        virtual void setLed(ledoffset_t index, RGBW color, bool flush = false) = 0;

        virtual void setRange(ledoffset_t index, ledoffset_t count, RGBW color, bool flush = false) {
            for (ledoffset_t i = 0; i < count; ++i) {
                setLed(index + i, color, false);
            }

            if (flush) {
                updateLeds();
            }
        }

        virtual void clear(bool flush = false) {
            ledoffset_t count = getLedCount();

            for (ledoffset_t i = 0; i < count; ++i) {
                setLed(i, RGBW(), false);
            }

            if (flush) {
                updateLeds();
            }
        }

        virtual void updateLeds() = 0;
};

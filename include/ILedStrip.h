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

        /**
        * Sets all LEDs to the specified color.
        * \param flush performs a update to the physics leds
        */
        virtual void setAll(RGBW color, bool flush = false) {
			setRange(0, getLedCount(), color, flush);
        }

        virtual void clear(bool flush = false) {
            setRange(0, getLedCount(), RGBW(), flush);
        }

        virtual void updateLeds() = 0;
};

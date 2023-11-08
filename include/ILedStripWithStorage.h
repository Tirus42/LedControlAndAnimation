#pragma once

#include "ILedStrip.h"

/**
* Sub class of ILedStrip, adds getter for the current state of the LEDs.
*/
class ILedStripWithStorage : public ILedStrip {
    public:
        virtual RGBW getLed(ledoffset_t index) const = 0;

        /// \returns true if any LED is not off, false otherwise.
        virtual bool isAnyActive() const {
            for (ledoffset_t i = 0; i < getLedCount(); ++i) {
                if (getLed(i) != COLOR_OFF) {
                    return true;
                }
            }

            return false;
        }
};

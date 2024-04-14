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

        /**
         * Copies the current content of this led strip to the given led strip
         * starting at offset 0. Checks for the size of the target.
         */
        virtual void copyTo(ILedStrip& target, bool flush = false) {
            ledoffset_t maxOffset = std::min(getLedCount(), target.getLedCount());

            for (ledoffset_t i = 0; i < maxOffset; ++i) {
                target.setLed(i, getLed(i), false);
            }

            if (flush) {
                target.updateLeds();
            }
        }
};

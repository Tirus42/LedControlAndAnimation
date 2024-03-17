#pragma once

#include "ILedStripWithStorage.h"

#include <vector>

/**
 * Simple storage class.
 * Implements the ILedStripWithStorage interface but only stores the values.
 */
class LedBufferStorage : public ILedStripWithStorage {
    private:
        std::vector<RGBW> pixels;

    public:
        LedBufferStorage(ledoffset_t ledCount) :
            pixels(ledCount) {}

        virtual ledoffset_t getLedCount() const override {
            return pixels.size();
        }

        virtual void setLed(ledoffset_t index, RGBW color, bool flush = false) override {
            pixels[index] = color;

            if (flush) {
                updateLeds();
            }
        }

        virtual RGBW getLed(ledoffset_t index) const override {
            return pixels[index];
        }

        virtual void updateLeds() override {
            // This is only a storage, nothing to do here
        }
};

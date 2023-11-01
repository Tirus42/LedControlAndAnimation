#pragma once

#include "ILedStrip.h"

/**
* Sub class of ILedStrip, adds getter for the current state of the LEDs.
*/
class ILedStripWithStorage : public ILedStrip {
    public:
        virtual RGBW getLed(ledoffset_t index) const = 0;
};

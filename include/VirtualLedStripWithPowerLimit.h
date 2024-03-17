#pragma once

#include <ILedStripWithStorage.h>
#include <LedBufferStorage.h>

struct LedPowerConsumptionInfo {
    const float ledBasePowerConsumtion_mA;
    const float colorChannelMaxPowerConsumtion_mA;
    const float whiteChannelMaxPowerConsumtion_mA;

    constexpr LedPowerConsumptionInfo(float ledBasePowerConsumtion_mA, float colorChannelMaxPowerConsumtion_mA, float whiteChannelMaxPowerConsumtion_mA) :
        ledBasePowerConsumtion_mA(ledBasePowerConsumtion_mA),
        colorChannelMaxPowerConsumtion_mA(colorChannelMaxPowerConsumtion_mA),
        whiteChannelMaxPowerConsumtion_mA(whiteChannelMaxPowerConsumtion_mA) {}

    float calculateColorChannelPowerConsumption(uint8_t channelValue) const {
        return float(channelValue) * (1.0 / 255.f) * colorChannelMaxPowerConsumtion_mA;
    }

    float calculateWhiteChannelPowerConsumption(uint8_t channelValue) const {
        return float(channelValue) * (1.0 / 255.f) * whiteChannelMaxPowerConsumtion_mA;
    }

    float calculatePowerConsumption(RGBW color) const {
        return ledBasePowerConsumtion_mA
               + calculateColorChannelPowerConsumption(color.r)
               + calculateColorChannelPowerConsumption(color.g)
               + calculateColorChannelPowerConsumption(color.b)
               + calculateWhiteChannelPowerConsumption(color.w);
    }
};

/**
* Led strip pass-through implementation with a power consumption limit.
* Uses the per-led specified LedPowerConsumptionInfo to compute the actual consumption
* and reduce the brightness when necessary before updating the underlying led strip.
*/
class VirtualLedStripWithPowerLimit : public ILedStripWithStorage {
    private:
        LedBufferStorage ledBuffer;
        ILedStripWithStorage& baseStrip;
        const LedPowerConsumptionInfo consumptionInfo;

        float powerLimit_mA;

        float getCurrentPowerConsumption_mA(ILedStripWithStorage& leds) const {
            float summedPowerConsumption = 0.f;

            for (ledoffset_t i = 0; i < getLedCount(); ++i) {
                summedPowerConsumption += consumptionInfo.calculatePowerConsumption(leds.getLed(i));
            }

            return summedPowerConsumption;
        }

    public:
        VirtualLedStripWithPowerLimit(ILedStripWithStorage& baseStrip, LedPowerConsumptionInfo consumptionInfo, float powerLimit_mA) :
            ledBuffer(baseStrip.getLedCount()),
            baseStrip(baseStrip),
            consumptionInfo(consumptionInfo),
            powerLimit_mA(powerLimit_mA) {}

        void setPowerLimit(float newPowerLimit_mA) {
            powerLimit_mA = newPowerLimit_mA;
            updateLeds();
        }

        virtual ledoffset_t getLedCount() const override {
            return baseStrip.getLedCount();
        }

        virtual void setLed(ledoffset_t index, RGBW color, bool flush = false) override {
            ledBuffer.setLed(index, color, flush);

            if (flush) {
                updateLeds();
            }
        }

        virtual RGBW getLed(ledoffset_t index) const override {
            return ledBuffer.getLed(index);
        }

        virtual void updateLeds() override {
            // Step 1: Apply current values (but don't send them yet!)
            for (ledoffset_t i = 0; i < getLedCount(); ++i) {
                baseStrip.setLed(i, ledBuffer.getLed(i), false);
            }

            //#define DEBUG_REDUCE_STEPS

#ifdef DEBUG_REDUCE_STEPS
            float settedConsumption = getCurrentPowerConsumption_mA(baseStrip);
            uint32_t reduceStep = 0;
            uint32_t t0 = millis();
#endif

            // Step 2: Reduce color values until power limit is meet
            // Note: This is not the most efficient way to do this ...
            // This loop may take several milliseconds to complete
            while (getCurrentPowerConsumption_mA(baseStrip) > powerLimit_mA) {
                for (ledoffset_t i = 0; i < getLedCount(); ++i) {
                    RGBW currentColor = baseStrip.getLed(i);
                    uint32_t currentBrightness = currentColor.getTotalBrightness();

                    if (currentBrightness >= 10) {
                        baseStrip.setLed(i, currentColor.getWithTotalBrightness(currentBrightness - 10));
                    }
                }

#ifdef DEBUG_REDUCE_STEPS
                reduceStep++;
#endif
            }

#ifdef DEBUG_REDUCE_STEPS
            uint32_t t1 = millis();
            Serial.printf("Reduce steps: %u. Set from %f to %f (took %u ms)\n", reduceStep, settedConsumption, getCurrentPowerConsumption_mA(baseStrip), t1 - t0);
#endif

            // Step 3: Update the actual leds
            baseStrip.updateLeds();
        }

        float getCurrentPowerConsumption_mA() const {
            return getCurrentPowerConsumption_mA(baseStrip);
        }
};

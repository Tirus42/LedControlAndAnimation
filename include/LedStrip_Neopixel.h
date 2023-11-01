#pragma once

/**
* Note: This uses the Adafruid Neopixel library to use the leds.
* Add "adafruit/Adafruit NeoPixel@^1.11.0" as your dependencies when using this file.
*/
#include <Adafruit_NeoPixel.h>

#include "ILedStripWithStorage.h"
#include "IGPIOMappedDevice.h"

/**
* Leds strip with storage implementation of the NeoPixel protocol.
* Uses the Adafruid NeoPixel library.
*/
class LedStrip_Neopixel : public ILedStripWithStorage, IGPIOMappedDevice {
	private:
		Adafruit_NeoPixel leds;

	public:
		LedStrip_Neopixel(uint16_t countLeds, uint16_t pin, neoPixelType type = NEO_GRBW + NEO_KHZ800) :
			leds(countLeds, pin, type) {

			leds.begin();
		}

		virtual void setLed(ledoffset_t index, RGBW color, bool flush = false) override {
			leds.setPixelColor(index, leds.Color(color.r, color.g, color.b, color.w));

			if (flush) {
				updateLeds();
			}
		}

		virtual void updateLeds() {
			leds.show();
		}

		virtual ledoffset_t getLedCount() const {
			return leds.numPixels();
		}

		virtual RGBW getLed(ledoffset_t index) const override {
			return RGBW(leds.getPixelColor(index));
		}

		virtual std::vector<int16_t> getGPIOPins() const override {
			return {leds.getPin()};
		}

		virtual std::string getDeviceName() const override {
			return "LED strip (Neopixel)";
		}
};

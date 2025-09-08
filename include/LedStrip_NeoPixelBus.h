#pragma once

/**
 * Note: This uses the makuna/NeoPixelBus library to control the leds.
 * Add "makuna/NeoPixelBus@^2.8.4" as your dependencies when using this file.
 */
#include <NeoPixelBus.h>

#include "ILedStripWithStorage.h"

#include <type_traits>

/**
 * Leds strip with storage implementation.
 * Uses the NeoPixelBus library.
 *
 * Example:
 * LedStrip_NeoPixelBus<NeoGrbwFeature, NeoEsp32Rmt0Ws2812xMethod>
 * for RGBW leds with RMT control.
 * Note to use different RMT channels when controlling multiple strips!
 */
template<typename T_COLOR_FEATURE, typename T_METHOD>
class LedStrip_NeoPixelBus : public ILedStripWithStorage {
	private:
		NeoPixelBus<T_COLOR_FEATURE, T_METHOD> leds;

	public:
		LedStrip_NeoPixelBus(ledoffset_t countLeds, uint16_t pin) :
			leds(countLeds, pin) {

			leds.Begin();
		}

		virtual void setLed(ledoffset_t index, RGBW color, bool flush = false) override {
			if constexpr (std::is_same<T_COLOR_FEATURE, NeoGrbwFeature>::value) {
				leds.SetPixelColor(index, RgbwColor(color.r, color.g, color.b, color.w));
			} else if constexpr (std::is_same<T_COLOR_FEATURE, NeoGrbFeature>::value) {
				leds.SetPixelColor(index, RgbColor(color.r, color.g, color.b));
			} else {
				static_assert(sizeof(T_COLOR_FEATURE) == 0, "Unsupported pixel color channel");
			}

			if (flush) {
				updateLeds();
			}
		}

		virtual void updateLeds() override {
			// Explicit set dirty to force a update of the physical leds
			leds.Dirty();
			leds.Show();
		}

		virtual ledoffset_t getLedCount() const override {
			return leds.PixelCount();
		}

		virtual RGBW getLed(ledoffset_t index) const override {
			// Convert NeoPixelBus::RgbwColor -> RGBW
			RgbwColor r(leds.GetPixelColor(index));
			return RGBW(r.R, r.G, r.B, r.W);
		}
};

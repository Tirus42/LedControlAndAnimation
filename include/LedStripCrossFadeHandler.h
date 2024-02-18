#pragma once

#include <LedBufferStorageWithCallback.h>
#include <LedBufferStorage.h>

/**
* Handler class to fade between two led strip states.
* Defines one target led strip and stores two internal LedBufferStorage-instances.
* Via the fade-factor the cross fade can be controlled.
*/
class LedStripCrossFadeHandler : public ILedBufferStorageCallback {
	private:
		ILedStripWithStorage& target;
		float factor;

		LedBufferStorageWithCallback strip0;
		LedBufferStorageWithCallback strip1;

	public:
		LedStripCrossFadeHandler(ILedStripWithStorage& target, float initialFactor = 0.f) :
			target(target),
			factor(initialFactor),
			strip0(target.getLedCount(), this),
			strip1(target.getLedCount(), this) {}

		LedBufferStorage& getBaseLeds0() {
			return strip0;
		}

		LedBufferStorage& getBaseLeds1() {
			return strip1;
		}

		virtual void onUpdate(LedBufferStorageWithCallback& source) override {
			if (factor == 0.f && &source != &strip0) {
				return;
			}

			if (factor == 1.f && &source != &strip1) {
				return;
			}

			// full update required
			updateLeds();
		}

		/**
		* Sets the new factor for the cross fade, value must be in [0.0, 1.0].
		*/
		void setFactor(float newFactor, bool updateTarget = true) {
			this->factor = newFactor;

			if (updateTarget) {
				updateLeds();
			}
		}

		float getFactor() const {
			return factor;
		}

		/**
		* Updates the leds.
		* Computes the cross-faded values and updates the target led strip.
		*/
		void updateLeds() {
			for (ledoffset_t i = 0; i < target.getLedCount(); ++i) {
				RGBW mixedColor = strip0.getLed(i).interpolateTo(strip1.getLed(i), factor);

				target.setLed(i, mixedColor);
			}

			target.updateLeds();
		}
};

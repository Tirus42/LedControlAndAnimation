#include <LedStrip_APA102.h>
#include <AnimationManager.h>

/**
* Example to control a APA102 led strip and perform a simple color fade animation.
*/

// Set your configuration here
static const uint16_t LED_COUNT = 32;
static const uint16_t APA102_CLOCK_PIN = 25;
static const uint16_t APA102_DATA_PIN = 26;

static const uint32_t FADE_TIME_MS = 2000; // Animation fade time in milliseconds

// Create instance of the AnimationManager
AnimationManager animationManager;

// Create instance of the LedStrip
LedStrip_APA102 ledStrip(LED_COUNT, APA102_CLOCK_PIN, APA102_DATA_PIN);

static void FadeToColorRangeAnimation(uint32_t startTime, uint32_t fadeDuration, RGBW color0, RGBW color1) {
	uint16_t ledCount = ledStrip.getLedCount();

	for (uint16_t i = 0; i < ledCount; ++i) {
		// Interpolate color for current led
		float factor = float(i) / float(ledCount);

		RGBW ledColor = color0.interpolateTo(color1, factor);

		// Add fade from current animation
		animationManager.addAnimation(new FadeFromExistingAnimation(startTime, fadeDuration, ledStrip, i, ledColor));
	}
}

static void StartAnimation() {
	uint32_t currentTime = millis();

	// Add red -> green animation
	FadeToColorRangeAnimation(currentTime + 0, FADE_TIME_MS, COLOR_RED, COLOR_GREEN);

	// Add green -> blue animation
	FadeToColorRangeAnimation(currentTime + FADE_TIME_MS, FADE_TIME_MS, COLOR_GREEN, COLOR_BLUE);

	// Add blue -> red animation
	FadeToColorRangeAnimation(currentTime + FADE_TIME_MS, FADE_TIME_MS, COLOR_BLUE, COLOR_RED);
}

void setup() {
	StartAnimation();
}

void loop() {
	if (animationManager.empty()) {
		StartAnimation();
	}

	animationManager.update();
}

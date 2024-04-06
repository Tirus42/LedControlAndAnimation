#pragma once

#include "ILedStripWithStorage.h"

#include "Arduino.h"

#include <set>
#include <vector>
#include <memory>

#include "RGBW.h"

class AAnimation {
    protected:
        uint32_t startTime;
        uint32_t duration;

    public:
        AAnimation(uint32_t startTime, uint32_t duration) :
            startTime(startTime),
            duration(duration) {}

        uint32_t getStartTime() const {
            return startTime;
        }

        uint32_t getEndTime() const {
            return startTime + duration;
        }

        uint32_t getDuration() const {
            return duration;
        }

        bool operator<(const AAnimation& other) const {
            return startTime < other.startTime;
        }

        float getFactor(uint32_t currentTime) const {
            if (currentTime < getStartTime())
                return 0.f;
            if (currentTime > getEndTime())
                return 1.f;

            uint32_t animationTime = getEndTime() - currentTime;
            return 1.f - (float(animationTime) / float(getDuration()));
        }

        virtual void update(uint32_t currentTime) = 0;
};

class ALedAnimation : public AAnimation {
    protected:
        ILedStripWithStorage& ledControl;

    public:
        ALedAnimation(uint32_t startTime, uint32_t duration, ILedStripWithStorage& ledControl) :
            AAnimation(startTime, duration),
            ledControl(ledControl) {}

        ILedStripWithStorage& getLedControl() const {
            return ledControl;
        }
};

class FadeAnimation : public ALedAnimation {
    protected:
        RGBW startColor;
        RGBW endColor;
        uint16_t ledIndex;

    public:
        FadeAnimation(uint32_t startTime, uint32_t duration, ILedStripWithStorage& ledControl, uint16_t ledIndex, RGBW startColor, RGBW endColor) :
            ALedAnimation(startTime, duration, ledControl),
            startColor(startColor),
            endColor(endColor),
            ledIndex(ledIndex) {}

        virtual void update(uint32_t currentTime) override {
            RGBW color = startColor.interpolateTo(endColor, getFactor(currentTime));
            ledControl.setLed(ledIndex, color);
        }
};

class FadeFromExistingAnimation : public FadeAnimation {
    private:
        bool started;

    public:
        FadeFromExistingAnimation(uint32_t startTime, uint32_t duration, ILedStripWithStorage& ledControl, uint16_t ledIndex, RGBW endColor) :
            FadeAnimation(startTime, duration, ledControl, ledIndex, RGBW(), endColor),
            started(false) {}

        virtual void update(uint32_t currentTime) override {
            if (!started) {
                startColor = ledControl.getLed(ledIndex);
                started = true;
            }

            FadeAnimation::update(currentTime);
        }
};

/**
 * Lets the specified led blink countBlinks times.
 * Each flash is 100 ms long and 300 ms off.
 */
class BlinkAnimation : public ALedAnimation {
    private:
        RGBW color;
        RGBW previousColor;
        uint16_t ledIndex;
        uint16_t countBlicks;
        bool started:1;
        bool active:1;

    public:
        BlinkAnimation(uint32_t startTime, uint16_t countBlicks, ILedStripWithStorage& ledControl, uint16_t ledIndex, RGBW color) :
            ALedAnimation(startTime, countBlicks * 400, ledControl),
            color(color),
            previousColor(),
            ledIndex(ledIndex),
            countBlicks(countBlicks),
            started(false),
            active(false) {}

        virtual void update(uint32_t currentTime) override {
            if (!started) {
                previousColor = ledControl.getLed(ledIndex);
                started = true;
            }

            uint32_t deltaTime = startTime - currentTime;
            deltaTime = deltaTime % 400;

            if (deltaTime <= 100) {
                if (!active && countBlicks > 0) {
                    ledControl.setLed(ledIndex, color);
                    countBlicks--;
                    active = true;
                }
            } else {
                if (active) {
                    ledControl.setLed(ledIndex, previousColor);
                    active = false;
                }
            }
        }
};

class AnimationManager {
    private:
        typedef std::unique_ptr<ALedAnimation> AnimationPtr;

        std::vector<AnimationPtr> queue;

    public:
        AnimationManager() :
            queue() {}

        void update() {
            uint32_t currentTime = millis();

            std::vector<size_t> dropIndex;
            std::set<ILedStripWithStorage*> affectedLeds;

            for (size_t i = 0; i < queue.size(); ++i) {
                AnimationPtr& ptr = queue[i];

                if (currentTime < ptr->getStartTime())
                    continue;

                if (currentTime > ptr->getEndTime()) {
                    dropIndex.push_back(i);
                }

                ptr->update(currentTime);
                affectedLeds.insert(&(ptr->getLedControl()));
            }

            for (ILedStripWithStorage* ledControl : affectedLeds) {
                ledControl->updateLeds();
            }

            while (!dropIndex.empty()) {
                size_t lastEntry = *dropIndex.rbegin();

                queue.erase(queue.begin() + lastEntry);
                dropIndex.erase(dropIndex.begin() + dropIndex.size() - 1);
            }
        }

        void addAnimation(ALedAnimation* ptr) {
            queue.emplace_back(std::move(ptr));
        }

        bool empty() const {
            return queue.empty();
        }

        /**
         * Deletes all active animations.
         * Note that the end color of animations will not be applied.
         */
        void clear() {
            queue.clear();
        }
};

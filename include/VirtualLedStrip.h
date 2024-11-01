#pragma once

#include "ILedStripWithStorage.h"

#include <vector>
#include <iterator>

template <typename ... Base>
class VirtualMultiLedStrip : public ILedStripWithStorage {
    public:
        virtual ledoffset_t getLedCount() const override {
            return 0;
        }

        virtual void setLed(ledoffset_t, RGBW, bool) override {}
        virtual RGBW getLed(ledoffset_t index) const override {
            return RGBW();
        }
        virtual void updateLeds() override {}
};

template <typename Base, typename ... Rest>
class VirtualMultiLedStrip<Base, Rest ...> : public ILedStripWithStorage {
    private:
        Base& first;
        VirtualMultiLedStrip<Rest ...> rest;

        const Base& getAffectedLedStrip_(ledoffset_t& inout_offset) const {
            if (inout_offset < first.getLedCount()) {
                return first;
            } else {
                inout_offset -= first.getLedCount();

                if constexpr (sizeof...(Rest) > 0) {
                    return rest.getAffectedLedStrip(inout_offset);
                } else {
                    return first;	// Should never happen
                }
            }
        }

    public:
        VirtualMultiLedStrip(Base& first, Rest& ... rest) :
            first(first),
            rest(rest ...) {}

        virtual ledoffset_t getLedCount() const override {
            return first.getLedCount() + rest.getLedCount();
        }

        virtual void setLed(ledoffset_t index, RGBW color, bool flush = false) override {
            if (index < first.getLedCount()) {
                first.setLed(index, color, flush);
            } else {
                rest.setLed(index - first.getLedCount(), color, flush);
            }
        }

        virtual RGBW getLed(ledoffset_t index) const override {
            if (index < first.getLedCount()) {
                return first.getLed(index);
            } else {
                return rest.getLed(index - first.getLedCount());
            }
        }

        virtual void updateLeds() override {
            first.updateLeds();
            rest.updateLeds();
        }

        /**
         * \param inout_offset returns the actual offset in the affected led strip.
         * \returns the underlaying led strip which is affected by the specified offset.
         */
        const Base& getAffectedLedStrip(ledoffset_t& inout_offset) const {
            return getAffectedLedStrip_(inout_offset);
        }

        /**
         * \param inout_offset returns the actual offset in the affected led strip.
         * \returns the underlaying led strip which is affected by the specified offset.
         */
        Base& getAffectedLedStrip(ledoffset_t& inout_offset) {
            const Base& ledStrip = getAffectedLedStrip_(inout_offset);
            return const_cast<Base&>(ledStrip);
        }
};

typedef VirtualMultiLedStrip<ILedStripWithStorage, ILedStripWithStorage> VirtualMultiLedStrip2;
typedef VirtualMultiLedStrip<ILedStripWithStorage, ILedStripWithStorage, ILedStripWithStorage> VirtualMultiLedStrip3;
typedef VirtualMultiLedStrip<ILedStripWithStorage, ILedStripWithStorage, ILedStripWithStorage, ILedStripWithStorage> VirtualMultiLedStrip4;
typedef VirtualMultiLedStrip<ILedStripWithStorage, ILedStripWithStorage, ILedStripWithStorage, ILedStripWithStorage, ILedStripWithStorage> VirtualMultiLedStrip5;

/**
* Simple pass-through virtual led strip.
* Passes all calls to the base strip reference.
*/
class VirtualPassthroughLedStrip : public ILedStripWithStorage {
    protected:
        ILedStripWithStorage& baseStrip;

    public:
        VirtualPassthroughLedStrip(ILedStripWithStorage& baseStrip) :
            baseStrip(baseStrip) {}

        virtual ledoffset_t getLedCount() const {
            return baseStrip.getLedCount();
        }

        virtual void setLed(ledoffset_t index, RGBW color, bool flush = false) override {
            baseStrip.setLed(index, color, flush);
        }

        virtual RGBW getLed(ledoffset_t index) const override {
            return baseStrip.getLed(index);
        }

        virtual void updateLeds() override {
            baseStrip.updateLeds();
        }
};

/**
* Maps a subset of leds of the given base strip.
* Allows to specify the individual indices.
*/
class VirtualMappedLedStrip : public VirtualPassthroughLedStrip {
    private:
        std::vector<ledoffset_t> indices;

    public:
        typedef std::iterator<std::forward_iterator_tag, ledoffset_t> iterator;

        VirtualMappedLedStrip(ILedStripWithStorage& baseStrip, const std::vector<ledoffset_t>& indices) :
            VirtualPassthroughLedStrip(baseStrip),
            indices(indices) {}

        template<typename Iterator>
        VirtualMappedLedStrip(ILedStripWithStorage& baseStrip, Iterator iteratorBegin, Iterator iteratorEnd) :
            VirtualPassthroughLedStrip(baseStrip),
            indices(iteratorBegin, iteratorEnd) {}

        virtual ledoffset_t getLedCount() const override {
            return indices.size();
        }

        virtual void setLed(ledoffset_t index, RGBW color, bool flush = false) override {
            baseStrip.setLed(indices[index], color, flush);
        }

        virtual RGBW getLed(ledoffset_t index) const override {
            return baseStrip.getLed(indices[index]);
        }
};

/**
* Virtual led strip which inverts the offsets of all leds.
* Makes the first to the last and vice versa.
*/
class VirtualInversedLedStrip : public ILedStripWithStorage {
    private:
        ILedStripWithStorage& leds;

        ledoffset_t calcOffset(ledoffset_t index) const {
            return leds.getLedCount() - 1 - index;
        }

    public:
        VirtualInversedLedStrip(ILedStripWithStorage& leds) :
            leds(leds) {}

        virtual ledoffset_t getLedCount() const override {
            return leds.getLedCount();
        }

        virtual void setLed(ledoffset_t index, RGBW color, bool flush = false) override {
            leds.setLed(calcOffset(index), color, flush);
        }

        virtual RGBW getLed(ledoffset_t index) const override {
            return leds.getLed(calcOffset(index));
        }

        virtual void updateLeds() override {
            leds.updateLeds();
        }
};

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
};

typedef VirtualMultiLedStrip<ILedStripWithStorage, ILedStripWithStorage> VirtualMultiLedStrip2;
typedef VirtualMultiLedStrip<ILedStripWithStorage, ILedStripWithStorage, ILedStripWithStorage> VirtualMultiLedStrip3;
typedef VirtualMultiLedStrip<ILedStripWithStorage, ILedStripWithStorage, ILedStripWithStorage, ILedStripWithStorage> VirtualMultiLedStrip4;
typedef VirtualMultiLedStrip<ILedStripWithStorage, ILedStripWithStorage, ILedStripWithStorage, ILedStripWithStorage, ILedStripWithStorage> VirtualMultiLedStrip5;

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

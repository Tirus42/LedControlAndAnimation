#pragma once

// Include CiString for case insensitive string comparison
#include <CiString.h>

#include <map>

static uint8_t AddWOOverflow(uint8_t a, uint8_t b) {
    if (uint16_t(a) + uint16_t(b) > 0xFF)
        return 0xFF;

    return a + b;
}

struct RGBW {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t w;

    RGBW() :
        r(0), g(0), b(0), w(0) {}

    RGBW(uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0) :
        r(r), g(g), b(b), w(w) {}

    RGBW(uint32_t packedColor) :
        r((packedColor & 0xFF0000) >> 16),
        g((packedColor & 0xFF00) >> 8),
        b((packedColor & 0xFF)),
        w((packedColor & 0xFF000000) >> 24) {}

    /// Returns the RGBW value as packed uint32_t (byte order: WRGB)
    uint32_t getAsPackedColor() const {
        return uint32_t(w) << 24 | uint32_t(r) << 16 | uint32_t(g) << 8 | uint32_t(b);
    }

    /// Returns the summed value over all 4 components
    uint16_t getTotalBrightness() const {
        return uint16_t(r) + uint16_t(g) + uint16_t(b) + uint16_t(w);
    }

    /**
    * Returns a new RGBW value with all components adapted so that
    * the getTotalBrightness() is <= the \param targetBrightness.
    */
    RGBW getWithTotalBrightness(uint16_t targetBrightness) const {
        targetBrightness = std::min<uint16_t>(targetBrightness, 0xFF * 4);

        if (r == 0 && g == 0 && b == 0 && w == 0) {
            uint16_t componentValue = std::min(targetBrightness / 4, 0xFF);
            return RGBW(componentValue, componentValue, componentValue, componentValue);
        }

        if (targetBrightness == 0) {
            return RGBW();
        }

        uint16_t currentBrightness = getTotalBrightness();

        float factor = float(targetBrightness) / float(currentBrightness);

        RGBW newValue(
            std::min<uint16_t>(0xFFu, uint16_t(r * factor)),
            std::min<uint16_t>(0xFFu, uint16_t(g * factor)),
            std::min<uint16_t>(0xFFu, uint16_t(b * factor)),
            std::min<uint16_t>(0xFFu, uint16_t(w * factor))
        );

        while (newValue.getTotalBrightness() > targetBrightness) {
            newValue.r = newValue.r > 0 ? newValue.r - 1 : 0;
            newValue.g = newValue.g > 0 ? newValue.g - 1 : 0;
            newValue.b = newValue.b > 0 ? newValue.b - 1 : 0;
            newValue.w = newValue.w > 0 ? newValue.w - 1 : 0;
        }

        return newValue;
    }

    RGBW operator*(float factor) const {
        return RGBW(
                   r * factor,
                   g * factor,
                   b * factor,
                   w * factor
               );
    }

    RGBW operator+(const RGBW& other) const {
        return RGBW(
                   AddWOOverflow(r, other.r),
                   AddWOOverflow(g, other.g),
                   AddWOOverflow(b, other.b),
                   AddWOOverflow(w, other.w)
               );
    }

    bool operator==(const RGBW& other) const {
        return r == other.r && g == other.g && b == other.b && w == other.w;
    }

    bool operator!=(const RGBW& other) const {
        return r != other.r || g != other.g || b != other.b || w != other.w;
    }

    RGBW interpolateTo(const RGBW& other, float factor) const {
        return *this * (1.f - factor) + other * factor;
    }

    static RGBW Max(const RGBW& a, const RGBW& b) {
        return RGBW(
                   std::max(a.r, b.r),
                   std::max(a.g, b.g),
                   std::max(a.b, b.b),
                   std::max(a.w, b.w)
               );
    }

    static RGBW Min(const RGBW& a, const RGBW& b) {
        return RGBW(
                   std::min(a.r, b.r),
                   std::min(a.g, b.g),
                   std::min(a.b, b.b),
                   std::min(a.w, b.w)
               );
    }
};

static const RGBW COLOR_OFF = RGBW();
static const RGBW COLOR_RED = RGBW(255, 0, 0, 0);
static const RGBW COLOR_GREEN = RGBW(0, 255, 0, 0);
static const RGBW COLOR_BLUE = RGBW(0, 0, 255, 0);
static const RGBW COLOR_YELLOW = RGBW(0xFF, 0xFF, 0, 0);
static const RGBW COLOR_WWHITE = RGBW(0, 0, 0, 255);
static const RGBW COLOR_TURQUOISE = RGBW(0, 0xFF, 0xFF, 0);
static const RGBW COLOR_MAGENTA = RGBW(0xFF, 0, 0xFF, 0);
static const RGBW COLOR_KWHITE = RGBW(255, 255, 255, 0);
static const RGBW COLOR_NWHITE = RGBW(255, 255, 255, 255);
static const RGBW COLOR_ALL = COLOR_NWHITE;

static const std::map<CiString, RGBW> NamedColors = {
    {"Off", COLOR_OFF},
    {"Full", COLOR_ALL},
    {"Red", COLOR_RED},
    {"Green", COLOR_GREEN},
    {"Blue", COLOR_BLUE},
    {"White", COLOR_NWHITE},
    {"CWhite", COLOR_KWHITE},
    {"WWhite", COLOR_WWHITE},
    {"Yellow", COLOR_YELLOW},
    {"Turquoise", COLOR_TURQUOISE},
    {"Magenta", COLOR_MAGENTA},
};

/**
* Finds the named color by name.
* \param name The input name, comparison will be case insensitive.
* \param factor factor to directly multiply on the found color.
* \param output Output variable for the found color, only defined when the return value is true.
* \returns true when the color by name was found, false otherwise.
*/
inline bool GetNamedColor(const std::string& name, float factor, RGBW& output) {
    auto colorIter = NamedColors.find(name);

    if (colorIter == NamedColors.end()) {
        return false;
    }

    factor = std::min(1.f, factor);
    factor = std::max(0.f, factor);

    output = colorIter->second * factor;
    return true;
}

/**
* Parses the named color by name and allows a additional factor to multiply it directly.
* E.g. "red*0.5".
* \param name
* \param output Output variable for the found color, only defined when the return value is true.
* \returns true when the color by name was found, false otherwise.
*/
inline bool ParseNamedColor(const std::string& str, RGBW& output) {
    size_t pos = str.find('*');

    if (pos == std::string::npos) {
        return GetNamedColor(str, 1.f, output);
    }

    std::string name = str.substr(0, pos);
    std::string factorString = str.substr(pos + 1);

    float factor = std::atof(factorString.c_str());

    return GetNamedColor(name, factor, output);
}

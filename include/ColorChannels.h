#pragma once

/**
* Helper struct to define, which color channels are active / useable.
*/
struct ColorChannels {
	bool r:1;
	bool g:1;
	bool b:1;
	bool w:1;

	ColorChannels(bool r, bool g, bool b, bool w) :
		r(r), g(g), b(b), w(w) {}

	/**
	* Expects a string like "RGB", "RGBW" or any combination.
	* Extracts the channels based on the letters inside the string.
	*/
	ColorChannels(const char* channels) :
		r(false), g(false), b(false), w(false) {

		size_t offset = 0;
		while (channels[offset] != 0) {
			char c = channels[offset++];

			switch (c) {
				case 'r':
				case 'R':
					r = true;
					break;
				case 'g':
				case 'G':
					g = true;
					break;
				case 'b':
				case 'B':
					b = true;
					break;
				case 'w':
				case 'W':
					w = true;
					break;
			}
		}
	}
};

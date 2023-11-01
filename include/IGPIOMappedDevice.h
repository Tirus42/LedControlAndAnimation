#pragma once

#include <vector>
#include <cstdint>
#include <string>

/**
* Interface class for devices mapped via one or multiple GPIO pins.
*/
class IGPIOMappedDevice {
	public:
		virtual ~IGPIOMappedDevice() = default;

		virtual std::vector<int16_t> getGPIOPins() const = 0;
		virtual std::string getDeviceName() const = 0;
};

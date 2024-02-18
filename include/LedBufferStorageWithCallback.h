#pragma once

#include <LedBufferStorage.h>

class LedBufferStorageWithCallback;

class ILedBufferStorageCallback {
	public:
		virtual void onUpdate(LedBufferStorageWithCallback& source) = 0;
};

/**
* Subclass of the LedBufferStorage.
* Calls the given callback instance on specified events.
*/
class LedBufferStorageWithCallback : public LedBufferStorage {
	private:
		ILedBufferStorageCallback* callback;

	public:
		LedBufferStorageWithCallback(ledoffset_t ledCount, ILedBufferStorageCallback* callback) :
			LedBufferStorage(ledCount),
			callback(callback) {}

		void setCallbackHandler(ILedBufferStorageCallback* newCallback) {
			this->callback = newCallback;
		}

		virtual void updateLeds() override {
			if (callback) {
				callback->onUpdate(*this);
			}

			LedBufferStorage::updateLeds();
		}
};

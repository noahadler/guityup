#ifndef GUITYUP_MIDIBIND
#define GUITYUP_MIDIBIND

#include "ConfigFile.h"
#include <string>
#include <iostream>
#include <vector>

extern ConfigFile config;

class MidiBind;

enum BindState
{
	Activating = 0,
	Deactivating = 1
};

typedef void (*MidiBindCallback)(MidiBind*, BindState);

class MidiBind
{
	std::vector<unsigned char> message;
	bool longPress;
	unsigned char device;

	MidiBindCallback callback;
public:
	MidiBind(const std::string& configName, MidiBindCallback callback);

	void processMessage(double timestamp, unsigned char device, std::vector<unsigned char> *message, void *userData);
};

#endif


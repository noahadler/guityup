#include "MidiBind.h"
#include <sstream>
#include <cstdlib>

MidiBind::MidiBind(const std::string& configName)
{
	std::string setting = config.read<std::string>(configName);
	std::cout<<"[" << configName << "] is [" << setting << "]\n";

	longPress = false;

	std::stringstream stream(setting);
	std::string token;

	bool last_token_was_hex = false;

	while (stream && !stream.eof())
	{
		stream >> token;
		std::cout << "\ttoken: " << token << std::endl;

		last_token_was_hex = false;

		if (token == "long")
		{
			longPress = true;
		}
		else if (token.length() == 2)
		{
			// assume hex char for now
			unsigned char byte = std::strtoul(token.c_str(),0,16);
			//std::cout << "hex: " << (unsigned int)byte << std::endl;
			message.push_back(byte);
		}
	}
	
	// the first digit was device number
	device = message[0];
	message.erase(message.begin());
	
}

// if the message matches the saved 
void MidiBind::processMessage(double timestamp, unsigned char device, std::vector<unsigned char> *message, void *userData)
{
	// make sure this is the correct device
	if (device != this->device)
		return;

	std::cout << "device match!" << std::endl;

	unsigned char messageType = message->at(0) >> 4;
	unsigned char channel = message->at(0) & 0x0f;

	// make sure this is the correct channel
	if (channel != (this->message.at(0) & 0x0f))
		return;

	if (messageType == 0x09)
	{
		// note on
		if (message->at(1) == this->message.at(1))
		{
			// TODO: trigger!
			std::cout << "note trigger!" << std::endl;
		}
	}
	else if (messageType == 0x08)
	{
		// note off
		if (message->at(1) == this->message.at(1))
		{
			// TODO: trigger or untrigger!
			std::cout << "note off trigger!" << std::endl;
		}
	}
	else if (messageType == 0x0b)
	{
		// CC
		if (message->at(1) == this->message.at(1))
		{	
			// TODO: trigger/adjust parameter!
			std::cout << "cc trigger!" << std::endl;
		}
	}
}



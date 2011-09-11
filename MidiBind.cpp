#include "MidiBind.h"

MidiBind::MidiBind(const std::string& configName)
{
	std::string setting = config.read<std::string>(configName);
	std::cout<<"[" << configName << "] is [" << setting << "]\n";
}

// if the message matches the saved 
void MidiBind::processMessage(double timestamp, std::vector<unsigned char> *message, void *userData)
{
}


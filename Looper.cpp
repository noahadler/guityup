#include "Looper.h"

MidiLooper::MidiLooper()
{
}

void MidiLooper::consumeMidiMessage(double timestamp, std::vector<unsigned char> *message, void *userData)
{
	if (recording)
	{
		sequence.push_back(MidiEvent(
			timestamp-recordingStartedTimestamp,
			*message));
	}
}

int MidiLooper::getLengthInSamples() const
{
}

double MidiLooper::getLengthInSeconds() const
{
	return recordingLength;
}

int MidiLooper::getScrubPositionInSamples() const
{
}

double MidiLooper::getScrubPositionInSeconds() const
{
}


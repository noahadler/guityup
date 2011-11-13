#include "Looper.h"
#include <iomanip>

MidiLooper::MidiLooper()
{
}

void MidiLooper::advancePlayback(double deltatime)
{
	// advance scrubPosition
	double newScrubPosition = scrubPosition + deltatime;
	if (newScrubPosition >= recordingLength)
	{
		newScrubPosition -= recordingLength;
	}

	// find first timestamped event that falls into this range
	for (int i=0; i<sequence.size(); ++i)
	{
		if (sequence[i].deltatime > newScrubPosition)
			break;
		if (sequence[i].deltatime >= scrubPosition)
		{
			midiOut[0]->sendMessage(&(sequence[i].message));

			// output the MIDI message
			std::cout << std::setprecision(10) << std::setw(8)
				<< "SCRUB" << '\t'
				<< std::setw(8) << deltatime << std::hex;
			for (unsigned int i=0; i<sequence[i].message.size(); ++i)
			{
				std::cout << '\t' << (int)(sequence[i].message.at(i));
			}
			std::cout << std::dec << '\n';
		}
	}

	scrubPosition = newScrubPosition;
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

void MidiLooper::setRecording(bool recording)
{
	if (recording)
	{
		sequence.empty();
		recordingLength = 0;
		this->recording = recording;
		recordingStartedTimestamp = audio.getStreamTime();
		scrubPosition = 0.0;
	}
	else
	{
		this->recording = recording;
		recordingLength = audio.getStreamTime() - recordingStartedTimestamp;
		scrubPosition = 0.0;
		std::cout << "loop set: " << recordingLength << "s with " << sequence.size() << " MIDI events" << std::endl;
	}
}

bool MidiLooper::getRecording()
{
	return recording;
}

// return the state of recording _after_ toggling
bool MidiLooper::toggleRecording()
{
	setRecording(!recording);
	return recording;
}


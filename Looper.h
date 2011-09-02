#ifndef GUITYUP_LOOPER
#define GUITYUP_LOOPER

#include <vector>
#include <deque>

enum LoopState
{
        Paused = 0,
        Playing,
        Recording,
        Overdubbing
};

struct MidiEvent
{
	double deltatime;
	std::vector<unsigned char> message;
	inline MidiEvent(double deltatime, std::vector<unsigned char> &message)
		: deltatime(deltatime), message(message)
	{
	}
};

typedef std::deque<MidiEvent> MidiBuffer;

class MidiLooper
{
	// Contains a series of MIDI events
	// corresponding to the beginning and end
	// of a voice containing a start and end pitch,
	// which may or may not be the same, and the
	// envelope between them
	struct VoiceFragment
	{
		int StartPitch;
		int EndPitch;
		
	};

	bool recordingCued;
	bool recording;
	//int sampleLength;
	//int sampleScrub;
	
	double recordingStartedTimestamp;
	double recordingLength;
	double scrubPosition;

	MidiBuffer sequence;

	bool activePitchClass[12];

	// sequence converted to a certain subset of notes
	MidiBuffer alteredSequence;
	void regenerateAlteredSequence();

	//Key estimatedKey;

public:
	MidiLooper();

	//void prepareToPlay(double sampleRate, int estimatedSamplesPerBlock);
	//void releaseResources();
	//void processBlock(AudioSampleBuffer &, MidiBuffer &);

	void consumeMidiMessage(double deltatime, std::vector<unsigned char> *message, void *userData);

	int getLengthInSamples() const;
	double getLengthInSeconds() const;

	int getScrubPositionInSamples() const;
	double getScrubPositionInSeconds() const;

	//inline const Key& getEstimatedKey() const { return estimatedKey; }

	inline void setRecording(bool recording) { this->recording = recording; }
	inline bool getRecording() { return recording; }

	// return the state of recording _after_ toggling
	inline bool toggleRecording()
	{
		return recording = !recording;
	}
};

#endif


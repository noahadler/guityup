#include "RtAudio.h"
#include "lo/lo.h"
#include "RtMidi.h"
#include "ConfigFile.h"
#include "Looper.h"
#include "MidiBind.h"
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <vector>

// This program is a multi-channel looper with sophisticated MIDI
// capabilities.  It's designed particularly for guitar, with thought
// towards other non-traditional MIDI devices, where bending, volume
// swells, or other capabilities not typically needed for keyboard
// based performances may arise.

ConfigFile config("settings.txt");

RtAudio audio;

std::vector<RtMidiIn*> midiIn;
std::vector<RtMidiOut*> midiOut;

MidiLooper looper;
std::vector<MidiBind> midiBinds;

double lastPlayTimestamp;

// liblo functions
void osc_error(int num, const char *m, const char *path);

int osc_generic_handler(const char *path, const char *types, lo_arg **argv,
		    int argc, void *data, void *user_data);

int foo_handler(const char *path, const char *types, lo_arg **argv, int argc,
		 void *data, void *user_data);

int quit_handler(const char *path, const char *types, lo_arg **argv, int argc,
		 void *data, void *user_data);

void read_stdin(void);


void listDevices(RtAudio& audio, RtMidiIn& midiIn, RtMidiOut& midiOut)
{


	unsigned int devices = audio.getDeviceCount();

	std::cout << "Device count: " << devices << std::endl;

	// Scan through devices for various capabilities
	RtAudio::DeviceInfo info;
	for ( unsigned int i=0; i<devices; i++ ) {

		info = audio.getDeviceInfo( i );

		if ( info.probed == true ) {
			std::cout << "Audio device #" << i << ": " << info.name << " (maximum output channels = " << info.outputChannels << ")\n";
		}
	}

	for ( unsigned int i=0; i<midiIn.getPortCount(); ++i )
		std::cout << "Midi In #" << i << ": " << midiIn.getPortName(i) << std::endl;

	for ( unsigned int i=0; i<midiOut.getPortCount(); ++i )
		std::cout << "Midi Out #" << i << ": " << midiOut.getPortName(i) << std::endl;
}

int audioCallback( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
           double streamTime, RtAudioStreamStatus status, void *data )
{
	class SineTable
	{
	public:
		std::vector<float> table;

		SineTable(float freq, int sampleRate) {
			// calculate wavelength in samples
			float wavelength = sampleRate/freq;
			table.resize(static_cast<int>(wavelength));
			for (int i=0; i<table.size(); ++i)
			{
				table[i] = std::sin(i*2*M_PI/wavelength);
			}
		}
	};

	static SineTable sineTable(440, 48000);
	static int sineTableIndex(0);

	// Since the number of input and output channels is equal, we can do
	// a simple buffer copy operation here.
	if ( status )
		std::cerr << "Stream over/underflow detected." << std::endl;

	unsigned long *bytes = (unsigned long *) data;
	//memcpy( outputBuffer, inputBuffer, *bytes );

	for (int i=0; i<nBufferFrames; ++i)
	{
		((float*)outputBuffer)[i] = 0.0*sineTable.table[sineTableIndex];
		sineTableIndex = (1+sineTableIndex)%sineTable.table.size();
	}

	// give looper a chance to output it's stored events
	looper.advancePlayback(streamTime - lastPlayTimestamp);
	lastPlayTimestamp = streamTime;

	return 0;
}

void midiInputCallback(double deltatime, std::vector<unsigned char> *message, void *userData)
{

	int device = *((int*)(userData));

	double timestamp = audio.getStreamTime();
	unsigned int nBytes = message->size();

	// output the MIDI message
	std::cout << std::setprecision(10) << std::setw(8)
		<< timestamp << '\t'
		<< std::setw(8) << deltatime << device << std::hex;
	for (unsigned int i=0; i<nBytes; ++i)
	{
		std::cout << '\t' << (int)(message->at(i));
	}
	std::cout << std::dec << '\n';

	// check bindings
	for (int i=0; i<midiBinds.size(); ++i)
	{
		// TODO: pass proper device ID
		midiBinds[i].processMessage(timestamp, device, message, userData);
	}

	// pass to phrase tracks
	looper.consumeMidiMessage(timestamp, message, userData);

	// pass to output
	midiOut[0]->sendMessage(message);
}

/* catch any incoming messages and display them. returning 1 means that the
 * message has not been fully handled and the server should try other methods */
int osc_generic_handler(const char *path, const char *types, lo_arg **argv,
		    int argc, void *data, void *user_data)
{
    int i;

    std::cout << "path: " << path << '\n';
    for (i=0; i<argc; i++) {
	std::cout << "arg " << i << ' ' << types[i];
	lo_arg_pp((lo_type)types[i], argv[i]);
	std::cout << '\n';
    }
    std::cout << std::endl;

    return 1;
}

void osc_error(int num, const char *msg, const char *path)
{
	std::cout << "OSC error #" << num << " at path " << path << ": "
		<< msg << std::endl;
}

void bind_toggleRecording(MidiBind* bind, BindState state)
{
	std::cout << "TOGGLE RECORDING LOOP" << std::endl;
	looper.toggleRecording();
}

void loadBindSettings()
{
	midiBinds.push_back(MidiBind("func_toggle_recording", &bind_toggleRecording));
}

int main(int argc, char** argv)
{
	midiIn.push_back(new RtMidiIn("Guityup remote"));
	midiIn.push_back(new RtMidiIn("Guityup instrument"));
	midiOut.push_back(new RtMidiOut("Guityup"));

	listDevices(audio,*midiIn[0],*midiOut[0]);

	RtAudio::StreamParameters inputParams, outputParams;
	inputParams.deviceId = config.read<int>("audio_device"); //std::atoi(argv[1]);
  	inputParams.nChannels = 2;
	outputParams = inputParams;

	RtAudio::StreamOptions options;
	options.flags = RTAUDIO_NONINTERLEAVED | RTAUDIO_MINIMIZE_LATENCY;
	options.streamName = "Guityup";

	unsigned int bufferBytes, bufferFrames = 128;

	try
	{
		audio.openStream( &outputParams, &inputParams, RTAUDIO_FLOAT32, 48000, &bufferFrames, &audioCallback, (void *)&bufferBytes, &options);
	}
	catch ( RtError& e )
	{
		e.printMessage();
		exit( 0 );
	}

	bufferBytes = bufferFrames * 2 * 4;

	// MIDI initialization
	int midi_input_device = config.read<int>("midi_in", -1); 
	if (midi_input_device == -1)
		midiIn[0]->openVirtualPort("Guityup");
	else
		midiIn[0]->openPort(midi_input_device, "Guityup");
	midiIn[0]->setCallback(&midiInputCallback, &midi_input_device);
	midiIn[0]->ignoreTypes(false,false,false);

	int midi_input_instrument_device = config.read<int>("midi_instrument_in",-1);
	if (midi_input_instrument_device == -1)
		midiIn[1]->openVirtualPort("Guityup");
	else
		midiIn[1]->openPort(midi_input_instrument_device, "Guityup");
	midiIn[1]->setCallback(&midiInputCallback, &midi_input_instrument_device);
	midiIn[1]->ignoreTypes(false,false,false);


	int midi_output_device = config.read<int>("midi_out", -1);
	if (midi_output_device == -1)
		midiOut[0]->openVirtualPort("Guityup");
	else
		midiOut[0]->openPort(midi_output_device, "Guityup");

	// OSC initialization
	int lo_fd;
	fd_set rfds;
	struct timeval tv;
	int retval;
	lo_server osc_server = lo_server_new("7770", osc_error);
	lo_server_add_method(osc_server, NULL, NULL, osc_generic_handler, NULL);

	// restore bonds
	loadBindSettings();

	try
	{
		lastPlayTimestamp = 0;
		audio.startStream();

		// TODO: replace this with select or put it in its
		// own thread
		while (true)
		{
			lo_server_recv_noblock(osc_server, 0);
		}

		char input;
		std::cout << "\nRunning ... press <enter> to quit.\n";
		std::cin.get(input);
	}
	catch ( RtError& e ) {
		e.printMessage();
  	}

	if ( audio.isStreamOpen() )
		audio.closeStream();
}


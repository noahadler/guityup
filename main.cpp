#include "RtAudio.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <vector>


void listDevices(RtAudio& audio)
{


	unsigned int devices = audio.getDeviceCount();

	std::cout << "Device count: " << devices << std::endl;

	// Scan through devices for various capabilities
	RtAudio::DeviceInfo info;
	for ( unsigned int i=0; i<devices; i++ ) {

		info = audio.getDeviceInfo( i );

		if ( info.probed == true ) {
			std::cout << i << ") " << info.name << " (maximum output channels = " << info.outputChannels << ")\n";
		}
	}

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
		((float*)outputBuffer)[i] = sineTable.table[sineTableIndex];
		sineTableIndex = (1+sineTableIndex)%sineTable.table.size();
	}

	std::cout << "test(" << nBufferFrames << "," << sineTableIndex << ")" << std::flush;

	return 0;
}

int main(int argc, char** argv)
{
	RtAudio audio;

	// list devices
	if (argc == 1)
	{
		listDevices(audio);
		return 0;
	}

	RtAudio::StreamParameters inputParams, outputParams;
	inputParams.deviceId = std::atoi(argv[1]);
  	inputParams.nChannels = 2;
	outputParams = inputParams;

	RtAudio::StreamOptions options;
	options.flags = RTAUDIO_NONINTERLEAVED;

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

	try
	{
		audio.startStream();

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


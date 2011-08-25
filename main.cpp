#include "RtAudio.h"
#include "lo/lo.h"
#include "RtMidi.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <vector>


// liblo functions
void osc_error(int num, const char *m, const char *path);

int osc_generic_handler(const char *path, const char *types, lo_arg **argv,
		    int argc, void *data, void *user_data);

int foo_handler(const char *path, const char *types, lo_arg **argv, int argc,
		 void *data, void *user_data);

int quit_handler(const char *path, const char *types, lo_arg **argv, int argc,
		 void *data, void *user_data);

void read_stdin(void);


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
		((float*)outputBuffer)[i] = 0.1*sineTable.table[sineTableIndex];
		sineTableIndex = (1+sineTableIndex)%sineTable.table.size();
	}

	return 0;
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

void midiCallback( double deltatime, std::vector< unsigned char > *message, void *userData )
{
  unsigned int nBytes = message->size();
  for ( unsigned int i=0; i<nBytes; i++ )
    std::cout << "Byte " << i << " = " << (int)message->at(i) << ", ";
  if ( nBytes > 0 )
    std::cout << "stamp = " << deltatime << std::endl;
}

int main(int argc, char** argv)
{
	RtAudio audio;
	RtMidiOut midiOut;

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


	// OSC initialization
	int lo_fd;
	fd_set rfds;
	struct timeval tv;
	int retval;
	lo_server osc_server = lo_server_new("7770", osc_error);
	lo_server_add_method(osc_server, NULL, NULL, osc_generic_handler, NULL);

	try
	{
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


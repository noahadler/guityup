ifdef OSX
	LIBRARY  = -framework CoreMIDI -framework CoreFoundation -framework CoreAudio
else
	LIBRARY	= -ljack
endif

guityup: main.cpp
	g++ main.cpp -o guityup rtmidi-1.0.15/tests/Release/RtMidi.o -I./rtaudio-4.0.8 -I./rtmidi-1.0.15 -Lrtaudio-4.0.8 -lrtaudio -llo $(LIBRARY)

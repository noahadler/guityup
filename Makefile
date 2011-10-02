UNAME := $(shell uname)

ifeq ($(UNAME), Darwin)
	LIBRARY  = -framework CoreMIDI -framework CoreFoundation -framework CoreAudio
endif
ifeq ($(UNAME), Linux)
	LIBRARY	= -lasound -ljack
endif

SOURCES=main.cpp Looper.cpp ConfigFile.cpp MidiBind.cpp

guityup: $(SOURCES)
	g++ $(SOURCES) -o guityup rtmidi-1.0.15/tests/Release/RtMidi.o -I./rtaudio-4.0.8 -I./rtmidi-1.0.15 -Lrtaudio-4.0.8 -lrtaudio -llo $(LIBRARY)


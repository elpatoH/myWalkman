PORTAUDIOINCLUDEFOLDER = /opt/homebrew/Cellar/portaudio/19.7.0/include
PORTAUDIOLIBFOLDER = /opt/homebrew/Cellar/portaudio/19.7.0/lib

CXXFLAGS = -Wall -std=c++11 -I$(PORTAUDIOINCLUDEFOLDER)
LDFLAGS = -L$(PORTAUDIOLIBFOLDER) -lportaudio

SRC = main.cpp

digitalWalkman: $(SRC)
	g++ $(CXXFLAGS) -o digitalWalkman $(SRC) $(LDFLAGS)

clean:
	rm -f digitalWalkman

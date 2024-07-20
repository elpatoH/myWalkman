#include <cstdlib>
#include <iostream>

#include <portaudio.h>
#include <ncurses.h>

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 512

/* 
    This routine will be called by the PortAudio engine when audio is needed.
    It may be called at interrupt level on some machines so don't do anything
    that could mess up the system like calling malloc() or free().
*/ 
static int patestCallback( 
    const void *inputBuffer, void *outputBuffer,
    unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags, void *userData 
    ) {
        return 0;
}

/* 
    Checks the return value of a PortAudio function. Logs the message and exits 
    if there was an error
*/
static void checkErr(PaError err) {
    if (err != paNoError) {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
        exit(EXIT_FAILURE);
    }
}

void processCommand(char* command){
    if (strcmp(command, "rec") == 0) {
        printf("Recording...\n");
        //TODO
    } else if (strcmp(command, "play") == 0) {
        printf("Playing...\n");
        //TODO
    } else if (strcmp(command, "add") == 0) {
        printf("Adding...\n");
        //TODO
    } else if (strcmp(command, "remove") == 0) {
        printf("Removing...\n");
        //TODO
    } else {
        printf("Unknown command.\n");
    }
}

int main() {
    printf("Welcome!\n");

    PaError err;
    err = Pa_Initialize();
    checkErr(err);

    int numDevices = Pa_GetDeviceCount();
    if (numDevices < 0) {
        printf("There was an error getting device count\n");
        exit(EXIT_FAILURE);
    } else if (numDevices == 0) {
        printf("No available devices\n");
        exit(EXIT_FAILURE);
    }

    printf("Number of devices: %d\nChoose an input device:\n", numDevices);

    // Display audio device information for each device accessible to PortAudio
    const PaDeviceInfo* deviceInfo;
    for (int i = 0; i < numDevices; i++) {
        deviceInfo = Pa_GetDeviceInfo(i);
        printf("  Device %d:\n", i);
        printf("  \tname: %s\n", deviceInfo->name);
        printf("  \tmaxInputChannels: %d\n", deviceInfo->maxInputChannels);
        printf("  \tmaxOutputChannels: %d\n", deviceInfo->maxOutputChannels);
        printf("  \tdefaultSampleRate: %f\n", deviceInfo->defaultSampleRate);
    }

    int inputDevice = 0;
    int outputDevice = 0;

    printf("Please choose input device (0-%d): ", numDevices - 1);
    scanf("%d", &inputDevice);

    printf("Please choose output device (0-%d): ", numDevices - 1);
    scanf("%d", &outputDevice);

    printf("Selected Devices:\n");
    printf("Input Device name: %s\n", Pa_GetDeviceInfo(inputDevice)->name);
    printf("Output Device name: %s\n", Pa_GetDeviceInfo(outputDevice)->name);

    char command[10];
    while (true){
        printf("Enter a command (rec, play, add, remove): ");
        memset(command, 0, sizeof(command));
        scanf("%9s", command);

        if (strcmp(command, "exit") == 0) {
            printf("Exiting...\n");
            break;
        }
        processCommand(command);
    }

    err = Pa_Terminate();
    checkErr(err);

    return EXIT_SUCCESS;
}
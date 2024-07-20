#include <cstdlib>
#include <iostream>

#include <portaudio.h>

#define FRAMES_PER_BUFFER 256

typedef struct {
    int frameIndex;
    int maxFrameIndex;
    int numChannels;
    bool armed;
    float *recordedSamples;
} paTestData;

static paTestData data;

static int recordCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
    paTestData *data = (paTestData*)userData;
    const float *rptr = (const float*)inputBuffer;
    float *wptr = &data->recordedSamples[data->frameIndex * data->numChannels];
    long framesToCalc;
    long i;

    if (data->frameIndex + framesPerBuffer > data->maxFrameIndex) {
        framesToCalc = data->maxFrameIndex - data->frameIndex;
    } else {
        framesToCalc = framesPerBuffer;
    }

    if (inputBuffer == NULL) {
        for (i = 0; i < framesToCalc; i++) {
            for (int j = 0; j < data->numChannels; j++) {
                *wptr++ = 0.0f;
            }
        }
    } else {
        for (i = 0; i < framesToCalc; i++) {
            for (int j = 0; j < data->numChannels; j++) {
                *wptr++ = *rptr++;
            }
        }
    }

    data->frameIndex += framesToCalc;
    return data->frameIndex >= data->maxFrameIndex ? paComplete : paContinue;
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

/*
    sets up audio recording
*/
void recordAudio(int inputDevice, int seconds){
    PaStreamParameters inputParameters;
    PaStream *stream;
    PaError err;
    int numSamples;
    int numBytes;

    //set number of channels
    data.numChannels = Pa_GetDeviceInfo(inputDevice)->maxInputChannels;

    //calculate number of frames
    data.maxFrameIndex = seconds * Pa_GetDeviceInfo(inputDevice)->defaultSampleRate;
    data.frameIndex = 0;

    //calculate number of bytes that will be used in the recording based on seconds given
    numSamples = data.maxFrameIndex * Pa_GetDeviceInfo(inputDevice)->maxInputChannels;
    numBytes = numSamples * sizeof(float);
    data.recordedSamples = (float *) malloc(numBytes);

    if (data.recordedSamples == NULL) {
        printf("Could not allocate memory for recording.\n");
        exit(EXIT_FAILURE);
    }
    memset(data.recordedSamples, 0, numBytes);

    inputParameters.device = inputDevice;
    inputParameters.channelCount = Pa_GetDeviceInfo(inputDevice)->maxInputChannels;
    inputParameters.sampleFormat = paFloat32;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(
            &stream,
            &inputParameters,
            NULL, // no output
            Pa_GetDeviceInfo(inputDevice)->defaultSampleRate,
            FRAMES_PER_BUFFER,
            paClipOff,
            recordCallback,
            &data);
    checkErr(err);

    printf("Recording for %d seconds...\n", seconds);

    //start recording
    err = Pa_StartStream(stream);
    checkErr(err);

    //wait for recording to end
    while ((err = Pa_IsStreamActive(stream)) == 1) {
        Pa_Sleep(1000);
    }
    checkErr(err);

    //close recording
    err = Pa_CloseStream(stream);
    checkErr(err);

    printf("Recording complete. You recorded %d seconds.\n", seconds);
}

static int playCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
    paTestData *data = (paTestData*)userData;
    float *rptr = &data->recordedSamples[data->frameIndex * data->numChannels];
    float *wptr = (float*)outputBuffer;
    unsigned int i;

    for (i = 0; i < framesPerBuffer; i++) {
        if (data->numChannels == 1) { // Mono recording
            float sample = *rptr++;
            *wptr++ = sample; // Left channel
            *wptr++ = sample; // Right channel
        } else {
            for (int j = 0; j < data->numChannels; j++) {
                *wptr++ = *rptr++;
            }
        }
    }
    data->frameIndex += framesPerBuffer;
    if (data->frameIndex >= data->maxFrameIndex) {
        return paComplete;
    }
    return paContinue;
}

void playAudio(int outputDevice){
    PaStreamParameters outputParameters;
    PaStream *stream;
    PaError err;

    //reset frameindex
    data.frameIndex = 0;

    outputParameters.device = outputDevice;
    outputParameters.channelCount = 2;
    outputParameters.sampleFormat = paFloat32;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputDevice)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(
            &stream,
            NULL, // no input
            &outputParameters,
            Pa_GetDeviceInfo(outputDevice)->defaultSampleRate,
            FRAMES_PER_BUFFER,
            paClipOff,
            playCallback,
            &data);
    checkErr(err);

    printf("Playing back recorded audio...\n");
    err = Pa_StartStream(stream);
    checkErr(err);

    while ((err = Pa_IsStreamActive(stream)) == 1) {
        Pa_Sleep(1000);
    }
    checkErr(err);

    err = Pa_CloseStream(stream);
    checkErr(err);

    printf("Playback complete.\n");
}

void processCommand(char* command, int inputDevice, int outputDevice){
    if (strcmp(command, "rec") == 0) {
        printf("How many seconds would you like to record: ");
        int seconds = 0;
        scanf("%d", &seconds);
        recordAudio(inputDevice, seconds);        
    } else if (strcmp(command, "play") == 0) {
        if (data.recordedSamples != NULL) {
            printf("Playing...\n");
            playAudio(outputDevice);
        } else {
            printf("No recording available to play.\n");
        }
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
        processCommand(command, inputDevice, outputDevice);
    }

    err = Pa_Terminate();
    checkErr(err);

    return EXIT_SUCCESS;
}
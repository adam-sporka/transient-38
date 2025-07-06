#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <windows.h>
#include <iostream>
#include <conio.h> 

#include "arduino_bypass.h"
#include "../transient-38/main.h"

#define SAMPLE_RATE 32000
#define BUFLEN 4096
#define CHANNELS 1
#define BYTES_PER_SAMPLE 1
#define BITS_PER_SAMPLE 8

HWAVEOUT hWaveOut;
WAVEHDR whdr[8];
unsigned char* audio_data[8];
int round_robin = 0;

////////////////////////////////////////////////////////////////
void InitSong()
{
    song.demo();
    song.restart();
}

////////////////////////////////////////////////////////////////
byte GetSample()
{
    bool should_update_song;
    byte sample = synth.getSample(should_update_song);
    sample *= 6;
    if (should_update_song)
    {
        song.onTick_();
    }
    return sample;
}

////////////////////////////////////////////////////////////////
DWORD dw;
void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    int skip;

    switch (uMsg) {
    case WOM_OPEN:
        break;
    case WOM_CLOSE:
        break;
    case WOM_DONE:
        for (int a = 0; a < BUFLEN; a++)
        {
            audio_data[round_robin % 4][a] = GetSample();
        }
        waveOutWrite(hWaveOut, &whdr[round_robin % 4], sizeof(whdr[round_robin % 4]));
        round_robin++;
        break;
    default:
        break;
    }
}

////////////////////////////////////////////////////////////////
void WaveOut_Initialize()
{
    // Create the output buffers
    WAVEFORMATEX waveformatex;
    waveformatex.wFormatTag = WAVE_FORMAT_PCM;
    waveformatex.nChannels = 1;
    waveformatex.nSamplesPerSec = static_cast<int>(SAMPLE_RATE);
    waveformatex.nAvgBytesPerSec = static_cast<int>(SAMPLE_RATE * CHANNELS * BYTES_PER_SAMPLE);
    waveformatex.nBlockAlign = CHANNELS * BYTES_PER_SAMPLE;
    waveformatex.wBitsPerSample = BITS_PER_SAMPLE;
    waveformatex.cbSize = 0;
    for (int i = 0; i < 4; i++)
    {
        audio_data[i] = new unsigned char[BUFLEN * CHANNELS * BYTES_PER_SAMPLE];
        memset(audio_data[i], 0, BUFLEN * CHANNELS * BYTES_PER_SAMPLE);
        whdr[i].lpData = (char*)audio_data[i];
        whdr[i].dwBufferLength = BUFLEN * CHANNELS * BYTES_PER_SAMPLE;
        whdr[i].dwBytesRecorded = 0;
        whdr[i].dwUser = 0;
        whdr[i].dwFlags = 0;
        whdr[i].dwLoops = 0;
    }

    // Open the sound card
    waveOutOpen(&hWaveOut, WAVE_MAPPER, &waveformatex, (DWORD_PTR)waveOutProc, (DWORD_PTR)&dw, CALLBACK_FUNCTION | WAVE_FORMAT_DIRECT);

    InitSong();

    // Pre-buffer
    for (int i = 0; i < 4; i++)
    {
        for (int a = 0; a < BUFLEN; a++)
        {
            audio_data[i][a] = GetSample();
        }
        waveOutPrepareHeader(hWaveOut, &whdr[i], sizeof(whdr[i]));
        waveOutWrite(hWaveOut, &whdr[i], sizeof(whdr[i]));
    }
}

////////////////////////////////////////////////////////////////
void main()
{
    printf("Transient 38 Emulator\n");
    WaveOut_Initialize();
    while (true)
    {
        char key = _getch();
        if (key == 27)
        {
            exit(0);
        }
    }
}

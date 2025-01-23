/*
 * Copyright (c) 2025, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <string.h>

#include "sensorytypes.h"
#include "SensoryDemoHelper.h"

// Read NET or GRAMMAR input file (*.bin)
void* readSensoryDataFile(const char* fileName, const char* description) {
    void* fileMemory;
    int i, fileSize;
    // open net, read into memory
    FILE* file = fopen(fileName, "rb");
    if (file == NULL)
    {
        printf("Cannot open %s input file '%s'", description, fileName);
        return NULL;
    }
    fseek(file, 0L, SEEK_END);
    fileSize = ftell(file);
    fileMemory = malloc(fileSize);
    if (fileMemory == 0)
    {
        printf("Out of memory reading %s file '%s'\n", description, fileName);
        return NULL;
    }
    rewind(file);
    i = fread(fileMemory, sizeof(char), fileSize, file);
    printf("%s read %d / %d bytes\n", description, i, fileSize);
    fclose(file);
    return fileMemory;
}

BOOL openAudioFile(const char* audioFile, audioData* audio) {
    memset((void *) audio, 0, sizeof(audioData));
    int size, header;
    // open .RAW/.WAV utterance, read into memory
    FILE* file = fopen(audioFile, "rb");
    if (file == NULL) {
        printf("\nCannot open audio input file '%s'\n", audioFile);
        return FALSE;
    }
    
    // Get file length, skip header
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    if (strstr(audioFile, ".wav") || strstr(audioFile, ".WAV")) {
        header = 44;    // skip .WAV header (usually 44 bytes)
    }
    else {
        header = 0;
    }
    fseek(file, header, SEEK_SET);
    audio->length = size - header;

    audio->file = file;
    audio->fileName = audioFile;
    audio->position = 0;
    printf("\nAudio file '%s' opened, size is %d bytes\n", audioFile, audio->length);
    return TRUE;
}

BOOL getAudio(audioData *audio, s16* samples, int sampleCount) {
    int sampleSize = sampleCount * sizeof(s16);
    // - - - Audio from the file - - - 
    // NOTE: if the phrase ends too close to file end (under 1 second)
    // then there may not be time to finish recognizing.
    //   Padding could be added here.
    if (audio->position < audio->length) {
        if (audio->position + sampleSize > audio->length) {
            // zero out any missing samples
            memset(samples, 0, sampleSize);
        }
        // Read from file
        fread(samples, 1, sampleSize, audio->file);
        audio->position += sampleSize;
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL initProcess(t2siStruct* t, void *netMemory, void *grammarMemory) {
    errors_t error;
    unsigned int sppSize;

    t->net = (intptr_t) netMemory;
    t->gram = (intptr_t) grammarMemory;

    error = SensoryAlloc(t, &sppSize);  // Find size needed
    if (error) {
        printf("SensoryAlloc failed with error 0x%x\n", error);
        return FALSE;
    }
    printf("SPP size = %d\n", sppSize);

    // Allocate a single block of memory for all dynamic persistent data
    t->spp = (void *)malloc(sppSize);
    if (t->spp == NULL)
    {
        printf("No memory left for SPP\n");
        return FALSE;
    }

    // initialize
    error = SensoryProcessInit(t);
    if (error) {
        printf("SensoryProcessInit failed with error 0x%x\n", error);
        return FALSE;
    }
    return TRUE;
}



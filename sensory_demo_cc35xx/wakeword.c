/*
 * Copyright (c) 2024, Texas Instruments Incorporated
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
/* This example demonstrates the McSPI RX and TX operation configured
 * in blocking, interrupt mode of operation.
 *
 * This example sends a known data in the TX mode of length APP_MCSPI_MSGSIZE
 * and then receives the same in RX mode. Internal pad level loopback mode
 * is enabled to receive data.
 * To enable internal pad level loopback mode, D0 pin is configured to both
 * TX Enable as well as RX input pin in the SYSCFG.
 *
 * When transfer is completed, TX and RX buffer data are compared.
 * If data is matched, test result is passed otherwise failed.
 */

#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"

// Driver Files
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART2.h>
#include "led_if.h"
#include <i2s_mic.h>

// DPL header files 
#include <ti/drivers/dpl/ClockP.h>
#include <ti/drivers/Power.h>

// Driver configuration 
#include "sensorytypes.h"
#include "ti_drivers_config.h"
#include "common.h"
#include "sensorylib.h"
#include "SensoryDemoHelper.h"

// Board Header files
#include "ti_drivers_config.h"

// Errors
#include "errors.h"

// Adaptation
#include "osi_kernel.h"
#include "uart_term.h"

// Sensory model from Voicehub
#include <wakeword-pc60-6.1.0-op08-prod-search-genie.h>
#include <command-pc62-6.1.0-op10-prod-search-new-genie.h>

#define MIC_DC_OFFSET           1650
#define MIC_DC_ATTENUATION      14

t2siStruct  appStruct;

typedef enum { RECOMODE_NONE, RECOMODE_WAKE, RECOMODE_COMMAND } RecoMode;

RecoMode recoMode = RECOMODE_WAKE;  // At first, wait for wakeword

uint16_t nnpqThresholdNew = 0; // If set, try 25000 - 32768
uint16_t maxTokens = 0;        // > 0 : override MAX_TOKENS
uint16_t sdet_type = SDET_NONE; // use SDET_LPSD for Low Power Sound Detect

int commandCountdown = 0;
#define COMMAND_SEC_WAIT 3  // Wait N seconds after wakeword for command
#define COMMAND_COUNTDOWN_FRAMES_DURATION   (COMMAND_SEC_WAIT * 1000 / 15)
//#define NUM_AUDIO_SAMPLES AUDIO_BUFFER_LEN

char * cmdPhrases[20] = { COMMAND_PHRASE_0, COMMAND_PHRASE_1, COMMAND_PHRASE_2, COMMAND_PHRASE_3, COMMAND_PHRASE_4, COMMAND_PHRASE_5 };
int16_t paramAOffsetWake = 0;     // Go negative if too many FA on wakeword, go positive if too many FR
int16_t paramAOffsetCommand = 0;  // Go negative (by 100, 200, etc) if too many FA on command, go positive if too many FR
int32_t glitchCount = 0;

int16_t audioBuffer[AUDIO_BUFFER_LEN];
int16_t raw_audio_samples_copy[NUM_AUDIO_SAMPLES];

// Enables ease of calibration for the microphone
uint32_t   microphoneAtten = MIC_DC_ATTENUATION;
int32_t    microphoneOffset = MIC_DC_OFFSET;

uint32_t getTick() {
    return xTaskGetTickCount();
}

BOOL reInitProcess(t2siStruct* t, void *netMemory, void *grammarMemory) {
    errors_t error;

    t->net = (intptr_t) netMemory;
    t->gram = (intptr_t) grammarMemory;

    // initialize
    error = SensoryProcessInit(t);
    if (error) {
        return FALSE;
    }
    return TRUE;
}

int32_t sqAmplitude(int16_t *audio, int len) {
    uint64_t sum = 0;
    for (int i=0; i<len; ++i) {
        int32_t sample = audio[i];
        sum += (uint64_t) (sample * sample);
    }
    return (uint32_t) (sum / len);
}

int32_t audioMean(int16_t *audio, int len) {
    int32_t sum = 0;
    for (int i=0; i<len; ++i) {
        sum += audio[i];
    }
    return (sum / len);
}

float timeInfo(uint32_t startTick, uint32_t endTick)
{
    float elapsedTime;
    uint32_t tickPeriod = 0; // ClockP_getSystemTickPeriod();
    uint32_t t1;

    if (endTick > startTick)
    {
        elapsedTime = (endTick - startTick) * tickPeriod;
    }
    else
    {
        /* Timer rolled over */
        t1 = UINT32_MAX - startTick;

        elapsedTime = (t1 + endTick) * tickPeriod;
    }
    return elapsedTime;
}

BOOL setupAppStruct(t2siStruct *t) {

    // zero appStruct including t2siStruct member
    memset(t, 0, sizeof(t2siStruct));
    // NOTE: the only option you should normally set in t2siStruct is audioBufferLen
    // But audioBufferLen can be 0 if you don't want any audio buffering
    // You must set net and search and eventually spp of course
    t->maxTokens = maxTokens ? maxTokens : MAX_TOKENS;
    if (sdet_type) t->sdet_type = sdet_type;
    t->paramAOffset = paramAOffsetWake;
    // initialize audio buffer items
    t->audioBufferLen = AUDIO_BUFFER_LEN;
    // Statically allocate the audio buffer instead of using malloc
    t->audioBuffer = audioBuffer;
    // You could set t->audioBuffer if you wanted to manage audio memory yourself
    // SensoryLPSDInit is called automatically now

    return TRUE;
}

//OSPREY_MX-38
#define HWREG(x)                                                              \
        (*((volatile unsigned long *)(x))) //TODO temporary need to be removed
#define ICACHE_BASE 0x41902000  //TODO temporary need to be removed, only for M3, M$ has different address


void *wakeword_demo(void *args)
{
    int32_t             RetVal = -1;
    HWREG(ICACHE_BASE + 0x84) |= 0x00000001  ;//OSPREY_MX-38
    HWREG(ICACHE_BASE + 0x4) |= 0xc0000000  ;//OSPREY_MX-38
    //HWREG(ICACHE_BASE + 0x4) |= 0x80000000  ;//OSPREY_MX-38, this is for 64M cache, instead CRAM
    
    uint32_t i = 0;
    RecoResult * sensoryStatus;
    uint32_t elapsedAccum = 0;
    infoStruct_T isp;
    unsigned short * dnn_command_netLabel;
    unsigned short * gs_command_grammarLabel;

    t2siStruct *t = &appStruct; // Where we look for return values

    Board_init();
    
    // init the terminal
    InitTerm();

    LED_IF_init();

    UART_PRINT("\rStarting Project\n");

    if (i2s_mic_init() != 0)
    {
        exit(-1);
    }

    UART_PRINT("\rMicrophone Initialized\n");

    // optionally get the version (or other info)
    SensoryInfo(&isp);
    UART_PRINT("\rVersion = %d.%d.%d\r\n", (isp.version>>20)&0x00000fff, (isp.version>>12)&0x000000ff, isp.version&0x00000fff);

    if (!setupAppStruct(t)) {
        exit(-1);
    }

    UART_PRINT("\r\nRecognizer setup.\r\n");

    if (!initProcess(t, (void *) dnn_wakeword_netLabel, (void *) gs_wakeword_grammarLabel)) {
        exit(-1);
    }

    UART_PRINT("\rRecognizer init.\r\n");

    commandCountdown = COMMAND_COUNTDOWN_FRAMES_DURATION;

    while (1)
    {
        /* Wait for I2S data to be available */
        if (sem_wait(&semDataReadyForTreatment) == 0)
        {
            /* This transaction should trigger every FRAME_LEN samples (240) to feed into Sensory */

            /* Get the latest audio pointer triggered by the semaphore */
            int32_t *buf          = latestAudioBufPtr.audioBufPtr;
            /* bufSize is expressed in bytes but samples to consider are 16 bits long */
            uint16_t numOfSamples = latestAudioBufPtr.numOfSamples;

            uint32_t n = 0;

            /* We only modify Left channel's samples () */
            for (i = 0; i < numOfSamples; i = i + 3)
            {
                /*
                    * Shift the data down to 16-bits from 32-bits and then
                    * remove the DC offset of the I2S microphone
                    */
                raw_audio_samples_copy[n++] = (int16_t) ((buf[i] >> microphoneAtten) + microphoneOffset);
            }

            uint32_t counter1, counter2, elapsed;

            if (commandCountdown > 0) 
            {
                // count down command duration
                -- commandCountdown;
                if (commandCountdown == 0) {
                    if (recoMode != RECOMODE_WAKE) {
                        t->paramAOffset = paramAOffsetWake;
                        reInitProcess(t, (void *) dnn_wakeword_netLabel, (void *) gs_wakeword_grammarLabel);
                        recoMode = RECOMODE_WAKE;
                        UART_PRINT("\nNo command found.\n");
                    }
                    // Reset the counter
                    commandCountdown = COMMAND_COUNTDOWN_FRAMES_DURATION;
                }
            }

            counter1 = getTick();
            sensoryStatus = SensoryProcessData(t, (SAMPLE *) raw_audio_samples_copy);
            counter2 = getTick();
            if (counter2 >= counter1) 
            {
                elapsed = counter2 - counter1;
                elapsedAccum += elapsed;
            }

            if (sensoryStatus->wordID && sensoryStatus->nnpqScore > 0) 
            {
                if (nnpqThresholdNew) 
                {
                    sensoryStatus->nnpqThreshold = nnpqThresholdNew;
                    if (!sensoryStatus->nnpqPass && sensoryStatus->nnpqScore >= nnpqThresholdNew) 
                    {
                        sensoryStatus->error = ERR_OK;
                        sensoryStatus->nnpqPass = TRUE;
                    }
                }

                UART_PRINT("\rRecognition .. ? #%lu, wordID = %d  score: %d  elapsed_time: %dms\r\n", (uint32_t) t->brickCount, sensoryStatus->wordID, sensoryStatus->finalScore, elapsed);
                UART_PRINT("\rNNPQ score= %d, NNPQ threshold= %d, NNPQ check pass= %d\r\n", sensoryStatus->nnpqScore, sensoryStatus->nnpqThreshold, sensoryStatus->nnpqPass);
            }

            if (sensoryStatus->error == ERR_OK) 
            {
                if (recoMode == RECOMODE_WAKE) 
                {
                    dnn_command_netLabel = (unsigned short *) dnn_en_command_netLabel;
                    gs_command_grammarLabel = (unsigned short *) gs_en_command_grammarLabel;
                    // Enter command mode
                    t->paramAOffset = paramAOffsetCommand;
                    reInitProcess(t, (void *) dnn_command_netLabel, (void *) gs_command_grammarLabel);
                    UART_PRINT("Waking up!\n");
                    recoMode = RECOMODE_COMMAND;
                }
                else if (recoMode == RECOMODE_COMMAND) 
                {
                    switch (sensoryStatus->wordID)
                    {
                        case 1:
                        {
                            // Toggle greed LED
                            LED_IF_toggle(GREEN_LED, 100);
                            break;
                        }
                        case 2:
                        {
                            // Toggle red LED
                            LED_IF_toggle(RED_LED, 100);
                            break;
                        }
                        case 3:
                        {
                            // Toggle blue LED
                            LED_IF_toggle(BLUE_LED, 100);
                            break;
                        }
                        case 4:
                        {
                            // Toggle all LEDs
                            LED_IF_toggle(GREEN_LED, 100);
                            LED_IF_toggle(RED_LED, 100);
                            LED_IF_toggle(BLUE_LED, 100);
                            break;
                        }
                    }

                    UART_PRINT("\r\n= = = COMMAND %d: %s = = =\r\n", sensoryStatus->wordID, cmdPhrases[sensoryStatus->wordID]);
                    UART_PRINT("\r*** Recognizer found wordID = %d, score = %d, at time %d, elapsed_time: %dms\r\n", sensoryStatus->wordID, sensoryStatus->finalScore, (uint32_t) t->brickCount, elapsed);

                    // Got an actual command
                    recoMode = RECOMODE_WAKE;
                    t->paramAOffset = paramAOffsetWake;
                    reInitProcess(t, (void *) dnn_wakeword_netLabel, (void *) gs_wakeword_grammarLabel);
                }

                commandCountdown = COMMAND_COUNTDOWN_FRAMES_DURATION;  // Wait approx N seconds for a command

            } else if (sensoryStatus->error == ERR_LICENSE) 
            {
                UART_PRINT("Sensory Lib license error!\n");
                break;
            } else if (sensoryStatus->error == ERR_DATACOL_TIMEOUT) 
            {
                UART_PRINT("Sensory automatic command timeout\n");
                // Go back to wakeword on command timeout (if there was an automatic command timeout)
                t->paramAOffset = paramAOffsetWake;
                reInitProcess(t, (void *) dnn_wakeword_netLabel, (void *) gs_wakeword_grammarLabel);
                UART_PRINT("ERR_DATACOL_TIMEOUT\n");
            } else if (sensoryStatus->error != ERR_NOT_FINISHED) 
            {
                UART_PRINT("SensoryProcessData returned error code 0x%x, wordID= %d\n", sensoryStatus->error, sensoryStatus->wordID);
                break;
            }
        }
    }
    
	return NULL;
}


#ifdef CC35XX
void *mainThread(void *args)
{
    wakeword_demo(NULL);
    return NULL;
}
#endif // CC35XX

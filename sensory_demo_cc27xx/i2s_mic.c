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

/* Includes */

#include <stdint.h>
#include <stddef.h>
#include <stdint.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2S.h>

/* RTOS header files */
#include <FreeRTOS.h>
#include <task.h>

/* Driver configuration */
#include "ti_drivers_config.h"
#include <ti/display/Display.h>
#include <ti/display/DisplayUart2.h>
#include <ti/drivers/I2S.h>
#include <ti/drivers/i2s/I2SLPF3.h>

#include "sensorylib.h"
#include <i2s_mic.h>
#include "common.h"

/* Total number of buffers to loop through */
#define NUMBUFS 3

/* This is the I2S data output offset
 * Set to 3 if running in Mono mode
 * Set to 4 if running in Stereo mode
 */
#define AUDIO_BUFFER_OFFSET     3

/* I2S buffer size. Each buffer size is sized up to 15ms of voice data */
#define BUFSIZE  ((BRICK_SIZE_MS * AUDIO_BUFFER_OFFSET * SAMPLE_RATE * sizeof(uint32_t)) / 1000)
volatile uint32_t adc_overflow = 0;

/*
 *  =============================== I2S ===============================
 */
I2S_Handle i2sHandle;
I2S_Params i2sParams;

/* pointer and size to the latest I2S audio buffer */
i2sAudioPtr_t latestAudioBufPtr;

/* Semaphore used to indicate that data must be processed */
sem_t semDataReadyForTreatment;
static sem_t semErrorCallback;

/* Lists containing transactions. Each transaction is in turn in these three lists */
List_List i2sReadList;

/* Buffers containing the data: written by read-interface, modified by treatment, and read by write-interface */
static uint8_t buf1[BUFSIZE];
static uint8_t buf2[BUFSIZE];
static uint8_t buf3[BUFSIZE];

static uint8_t *i2sBufList[NUMBUFS] = {buf1, buf2, buf3};

/* Transactions will successively be part of the i2sReadList, the treatmentList and the i2sWriteList */
I2S_Transaction i2sTransaction1;
I2S_Transaction i2sTransaction2;
I2S_Transaction i2sTransaction3;

extern Display_Handle hSerial;

static I2S_Transaction *i2sTransactionList[NUMBUFS] = {&i2sTransaction1,
                                                       &i2sTransaction2,
                                                       &i2sTransaction3};

static void errCallbackFxn(I2S_Handle handle, int_fast16_t status, I2S_Transaction *transactionPtr)
{
    /* The content of this callback is executed if an I2S error occurs */
    sem_post(&semErrorCallback);
}

static void readCallbackFxn(I2S_Handle handle, int_fast16_t status, I2S_Transaction *transactionPtr)
{
    /*
     * The content of this callback is executed every time a read-transaction
     * is started
     */

    /* We must consider the previous transaction (the current one is not over) */
    I2S_Transaction *transactionFinished = (I2S_Transaction *)List_prev(&transactionPtr->queueElement);

    if (transactionFinished != NULL)
    {
        // No action needed as the transactions are in a ring list except make a copy of the last audio buffer pointer

        /* Data processing */
        latestAudioBufPtr.audioBufPtr = transactionFinished->bufPtr;
        /* bufSize is expressed in bytes but samples to consider are 16 bits long */
        latestAudioBufPtr.numOfSamples = transactionFinished->bufSize / sizeof(uint32_t);

        /* Start the treatment of the data */
        sem_post(&semDataReadyForTreatment);
    }
}

/* Initialize the peripherals for Collecting audio input via SPI MIC or BOOSTXL MIC */
int32_t i2s_mic_init(void)
{
    /* Prepare the semaphore */
    uint32_t retc = sem_init(&semDataReadyForTreatment, 0, 0);
    if (retc == -1)
    {
        return retc;
    }

    /*
     *  Open the I2S driver
     */
    I2S_init();

    I2S_Params_init(&i2sParams);

    i2sParams.samplingFrequency = SAMPLE_RATE;
    i2sParams.fixedBufferLength = BUFSIZE;
    i2sParams.memorySlotLength  = I2S_MEMORY_LENGTH_32BITS;
    i2sParams.bitsPerWord       = 32;
    i2sParams.SD0Use            = I2S_SD0_INPUT;
    i2sParams.SD0Channels       = I2S_CHANNELS_MONO;
    i2sParams.samplingEdge      = I2S_SAMPLING_EDGE_FALLING;
    i2sParams.writeCallback     = NULL;
    i2sParams.readCallback      = readCallbackFxn;
    i2sParams.errorCallback     = errCallbackFxn;
    i2sHandle                   = I2S_open(CONFIG_I2S_0, &i2sParams);
    if (i2sHandle == NULL)
    {
        /* Error Opening the I2S driver */
        retc = -2;
    }

    /* Initialize the queues and the I2S transactions */
    List_clearList(&i2sReadList);

    uint8_t k;
    /* Use the transactions buffer for the read queue */
    for (k = 0; k < NUMBUFS; k++)
    {
        I2S_Transaction_init(i2sTransactionList[k]);
        i2sTransactionList[k]->bufPtr  = i2sBufList[k];
        i2sTransactionList[k]->bufSize = BUFSIZE;
        List_put(&i2sReadList, (List_Elem *)i2sTransactionList[k]);
    }

    List_tail(&i2sReadList)->next = List_head(&i2sReadList); // Read buffers are queued in a ring-list
    List_head(&i2sReadList)->prev = List_tail(&i2sReadList);

    I2S_setReadQueueHead(i2sHandle, (I2S_Transaction *)List_head(&i2sReadList));

    /* Start I2S streaming */
    I2S_startClocks(i2sHandle);
    I2S_startRead(i2sHandle);

    return retc;
}

/* Initialize the peripherals for Collecting audio input via the I2S port */
void deinit_i2s_mic(void)
{
    uint32_t k;

    Display_printf(hSerial, 0, 0, "deinit_i2s_mic\n");

    I2S_stopRead(i2sHandle);
    I2S_stopClocks(i2sHandle);
    I2S_close(i2sHandle);
    i2sHandle = NULL;

    for (k = 0; k < NUMBUFS; k++)
    {
        List_remove(&i2sReadList, (List_Elem *)i2sTransactionList[k]);
    }
}

/* Initialize the peripherals for Collecting audio input via the I2S port */
int32_t reinit_i2s_mic(void)
{
    uint32_t retc;

    /*
     *  Open the I2S driver
     */
    I2S_init();

    i2sHandle = I2S_open(CONFIG_I2S_0, &i2sParams);
    if (i2sHandle == NULL)
    {
        /* Error Opening the I2S driver */
        retc = -2;
    }

    /* Initialize the queues and the I2S transactions */
    List_clearList(&i2sReadList);

    uint8_t k;
    /* Use the transactions buffer for the read queue */
    for (k = 0; k < NUMBUFS; k++)
    {
        I2S_Transaction_init(i2sTransactionList[k]);
        i2sTransactionList[k]->bufPtr  = i2sBufList[k];
        i2sTransactionList[k]->bufSize = BUFSIZE;
        List_put(&i2sReadList, (List_Elem *)i2sTransactionList[k]);
    }

    List_tail(&i2sReadList)->next = List_head(&i2sReadList); // Read buffers are queued in a ring-list
    List_head(&i2sReadList)->prev = List_tail(&i2sReadList);

    I2S_setReadQueueHead(i2sHandle, (I2S_Transaction *)List_head(&i2sReadList));

    /* Start I2S streaming */
    I2S_startClocks(i2sHandle);
    I2S_startRead(i2sHandle);

    Display_printf(hSerial, 0, 0, "reinit_i2s_mic: %d\n", retc);

    return retc;
}

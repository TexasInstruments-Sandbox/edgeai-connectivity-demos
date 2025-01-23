 /*
 * File      : sensorylib.h
 * Purpose   : Generic functional and structural interface template
 * Project   : T2SI/THF on deeply-embedded DSPs
 * Compiler  : 
 *
 * Copyright (C) 1995-2022 Sensory Inc., All Rights Reserved
 *
 * ******* SENSORY CONFIDENTIAL *************
 *
 *****************************************************************************
 *
 * This is the standard API header for Sensory's tslice technology running
 * in a typical deeply-embedded DSP. It may be used as an implementation,
 * or as the starting point for further discussion.
 * This represents the face of Sensory's code exposed to the firmware (FW).
 *
 * Adapt this as required for the target platform
 *
*/

#ifndef SENSORYLIB_H_INCLUDED
#define SENSORYLIB_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>

/*------------------  overview  ----------------*
 * The intended target is a DSP that makes frequent calls to a "decoder" 
 * with small "frames" of audio samples, usually 15 ms or 240 samples
 *-------------- definitions -------------------*/

// platform specific items follow. Enable by defining one and only one  
// platform (such as PC, VPC) -- preferably by a compiler option in the .IDE
// This file should include typedefs for u32, s16, etc used in the structure definitions.
#include "sensorytypes.h"

/* there needs to be a documented error enumeration. Sensory code will return
 * errors in the format needed, assumed here "unsigned short". Also see ERROR CODES below */
typedef unsigned short errors_t;

/*---------- application configuration  -------------------------------------------------*/

// t->maxResults and t->maxTokens affect memory allocation, so they
// must be explicitly defined in the t2siStruct before calling SensoryAlloc().

#define MAX_RESULTS           6  // rare to need more for triggers or commands
#define MAX_TOKENS          300  // adjust as needed during development (monitor outOfMemory return)

/*****************************************************************************************/
/*****************************************************************************************/
/*----- system definitions - customer SHOULD NOT change ANY of the parameters below -----*/
/*****************************************************************************************/
/*****************************************************************************************/

typedef BOOL (*LPSDCallback)(int channel);

#define BRICK_SIZE_MS		(15) // 15 milliseconds = 240 samples

#define FRAME_LEN           240  // frame length in samples
#define FRAME_LEN_MS         15  // frame length in msec 
#define BRICK_SIZE_SAMPLES (BRICK_SIZE_MS*16)

#define BACKOFF_MS          270 // backoff in msec -- must be integral multiple of 30
#define NUM_BACKOFF_FRAMES  (((BACKOFF_MS+29)/30)*2) // 360msec = 24 frames 

#define ADDITIONAL_ENDPOINT_BACKOFF_FRAMES 5    // 0 seems to work best to find the endpoint that does not include
                                                // tail end of the trigger (may require longer delay value).
                                                // 5 may be better if the command set includes initial phoneme(s) 
                                                // that are similar to the final phoneme of the trigger
                                                // used in FindEndpoints()

/*
AUDIO_BUFFER_MS sets the size of the buffer. This can be changed at the app level without recompiling
the Sensory library.

The audio buffer must an integral multiple of 30 msec, which is an integral multiple of all supported
brick sizes.

The minimum size depends on the application requirements.  The maximum size is determined by the maximum
positive value of integer type AUDIOINDEX.  For example, if AUDIOINDEX is defined as a short integer
(16-bits signed), the maximum size of the audio buffer is 2040 msec or 32640 samples.  This is the
largest positive short integer value for a buffer that is an integral multiple of 30 msec (480 samples).

Without trigger endpoint detection:

    For non-LPSD, the minimum size is generally 30 msec. It can be 0 msec for the case of a 15 msec
    frame size (240 samples), which means that no audio buffer exists.

    For LPSD, the minimum size should match BACKOFF_MS which is defined in this file as well.   A typical
    value for BACKOFF_MS (and thus a minimum value for AUDIO_BUFFER_MS) is 270 msec.   Note that the
    application developer cannot change BACKOFF_MS, it is integrated in the Sensory library.

With trigger endpoint detection:

    For non-LPSD, the size depends on the delay value (from the grammar or t2siStruct delay field)
    and the value of ADDITIONAL_ENDPOINT_BACKOFF_FRAMES in sensorylib.h.   The application developer
    cannot change the latter value because it is integrated into the Sensory library.
    Typically for endpoint detection it is recommended that the delay value of the grammar be
    overridden by setting the delay value of the t2siStruct to be 240 msec.  A typical value for
    the AUDIO_BUFFER_MS in this scenario is 360 msec.  This is suitable for a delay value of 240 msec
    and ADDITIONAL_ENDPOINT_BACKOFF_FRAMES up to 6.

    For LPSD,  the size of the audio buffer should be the sum of the requirements of LPSD and endpoint
    detection.   A typical value in this scenario is 630 msec which is the sum of 270 msec for LPSD
    and 360 msec for endpoint detection.

*/

// Suggested audio buffer size
// Buffer size in milliseconds must be a multiple of 15
#define AUDIO_BUFFER_MS   600
#define NUM_AUDIO_BUFFER_FRAMES   (AUDIO_BUFFER_MS/15) 
#define AUDIO_BUFFER_LEN (NUM_AUDIO_BUFFER_FRAMES*FRAME_LEN)

/*---------------- default t2siStruct parameters (usually suitable) ---------------------*/
#define T2SI_DEFAULT_SEP_MS     400 
#define T2SI_MAX_SEP_MS         900      
#define T2SI_MIN_SEP_MS         100     
#define T2SI_DEFAULT_TIMEOUT_COMMAND 3    // in seconds. NOTE: default for trigger = 0

#define T2SI_MIN_KNOB           1
#define T2SI_DEFAULT_KNOB       3         
#define T2SI_MAX_KNOB           5

#define N_WAY_MAX 12  // Do not change this; it has been compiled-in.

enum {SDET_NONE, SDET_LPSD=2, SDET_USE_GRAMMAR=3, SDET_EXTERNAL_LPSD=4};       // types of speech detectors to use
enum {SDET_LPSD_SILENCE, SDET_MAKING_BLOCKS, SDET_RECOGNIZING}; // states for the speech detectors 

typedef enum {
    RecoStateNone = 0, RecoStateFailed = 1, RecoStatePending = 2, RecoStateDone = 3, RecoStateError = 4
} RecoState;

/*-------------- structure definitions --------------*/

/* -----Sensory standard results structure ----------*/
// result_t must be defined, but may be UNused in some deeply-embedded DSPs
typedef struct {
  u16 wordID;           // word recognized (0 = SIL for T2SI)
  u16 inVocab;          // 1 if not NOTA or SIL 
  u16 isNOTA;
  s16 score;            // segmental score, may be negative for WS
  u16 gscore;           // garbage score, can be used to compute duration 
} result_t;

#ifdef SENSORY_LOGGING
typedef struct {
int event;
int frameCount;
int a;
int b;
} SENSORY_LOG_OBJ;
#endif

typedef s16 SAMPLE;       // type of samples used by sensory. we can only support s16 (short) currently
typedef s24 AUDIOINDEX;   // can be s24 or s32 for larger buffers. must be signed!

typedef struct {
    errors_t error;      // ERR_OK (0) if something recognized
    u16 channel;
    u16 wordID;
    u16 duration;         // length in 15 msec frames of utterance
    // state of recognition
    // RecoStateNone=0, RecoStateFailed=1, RecoStatePending=2, RecoStateDone=3, RecoStateError=4
    RecoState recoState;  
    // Countdown to recognition result; 0 if nothing pending
    u16 countDown;
    // state of speech detector:
    // SDET_LPSD_SILENCE=0, SDET_MAKING_BLOCKS=1, SDET_RECOGNIZING=2
    u16 sdet_state;
    // The below are set if there has been a recognition
    u64 brickStart, brickEnd;  // The start and end brick index
    u64 brickCount; // the current brick
    // Start and end index of recognized audio.
    // Wraps, so endIndex could be before startIndex
    // If negative, the index is not in the audio buffer
    AUDIOINDEX startIndex, endIndex;
    // Start and end of audio expressed as frames before the current
    AUDIOINDEX startBackupFrames, endBackupFrames;
    // Scores etc are reported for debugging purposes
    u16 finalScore;       // basic score from search
    s16 nnpqPass;         // report nnpq pass
    u16 nnpqThreshold;    // nnpq threshold from model
    s16 svScore;          // Speaker Verification score
    u16 numResults;       // returns actual number of results
    u16 garbageScore;     // comparison score used for finalScore
    int nnpqScore;        // report score for NNPQ test
    int stage3Score;      // report score for Stage-3 test
} RecoResult;


/*----- Sensory standard application structure ------*/
/* This public structure is required by Sensory technology
 * The entire structure can be zeroed (to use defaults) 
 * before SensoryAlloc is called, except for net and gram. 
 * spp must be set before SensoryProcessInit.
 * Many inputs (I) can be set to various values - consult 
 * Sensory for details.
 */

typedef struct {
    intptr_t net;         // (I) address of the net
    intptr_t gram;        // (I) address of the grammar
    intptr_t reserved;    // (O) reserved for use by Sensory. Do not modify.
    void* spp;            // (I) private technology memory pointer
    void* extras;         // (I,O) Was in appStruct_T. Optional pointer to any extra application data
    void* featureSource;  // (O) Was in appStruct_T. Will be set if this recognizer shares features with another
    u16 sdet_type;        // (I) type of speech detector: see enum above
    u16 timeout;          // (I) time in seconds to listen for speech to start (0=forever for trigger, 3s for commands)
    u16 maxTalkTime;      // (I) maximum talk time in seconds (0=use value in grammar)
    u16 separator;        // (I) trailing silence in milliseconds
    u16 maxResults;       // (I) call with max number of results, at least 1
    u16 maxTokens;        // (I) controls allocation for part of technology memory
    u16 knob;             // (I) 1..5, affects non-spotted command confidence
    s16 paramAOffset;     // (I) matches SDK paramAOffset, centered at 0 (= default)
    u16 enableLogging;    // (I) enable runtime logging feature
    u16 noVerify;         // (I) ignore SG_VERIFY in grammar (treat as UDT)
    u16 lpsdFixedThresh;  // (I) 0: use default, otherwise sets fixed part of threshold
    u16 delay;            // (I) 0: use default delay from grammar, otherwise selects delay in msec
    u16 initFromLast;     // (I) 1: initialize with state at end of last (previous) recognition.
    u16 epqMinSNR;        // (I) 0: use default grammar value, 0xFFFF means disable, else
                        //     -24..24 db encoded as 1..0xFB30, (u16)(pow(10,minSNR/10)*256)
    s16 SvThreshold;        // (I) 0: will be overwritten with gram model default setting
                        //     Otherwise, keep new setting unchanged
    u32 audioBufferLen;   // (I)  audio buffer length in samples
    s16* audioBuffer;     // (I) OPTIONAL external audio buffer to be used instead.
                        // Must be channels * audioBufferLen in count of s16.
                        // May be NULL and then THFMicro will take its own out of SPP
    s16 LPSDLatencyCounter; // (I) reduced LPSD latency counter
    // (I) Calledback to notify application is now recognizing.
    // Param is which channel. Return 1 to delay one frame, 0 otherwise
    LPSDCallback LPSDIncreasePowerMode;
    // (I) Callback to notify application Sensory is now not recognizing.
    // Param is which channel. Return 0.
    LPSDCallback LPSDDecreasePowerMode;
    u16 outOfMemory;      // (O) error counter for out of tokens (indicates MAX_TOKENS too small)
    u16 tokensPruned;     // (O) indicates that pruning took place
    u16 maxTokensUsed;    // (O) max tokens used so far
    u32 size;             // (O) persistent memory size
    u16 channels;         // (O) Number of channels being handled (usually 1)
    u16 depth;            // (O) Number of frames being handled in each channel at once (usually 1)
    u64 brickCount;       // (O) count of 15 ms audio bricks consumed
#ifdef SENSORY_LOGGING
    SENSORY_LOG_OBJ *sensoryLogStart;
    int sensoryLogIndex;
    int sensoryLogFull;
    int sensoryNumLogItems;
#endif
} t2siStruct;

/* This optional structure can be used for application needs */
typedef struct infoStruct_S
{
  u32 version;    // for example, version of Sensory technology
}infoStruct_T;

#ifndef NO_SHARE_FEATURES 
#define SHARE_FEATURES 
#endif

/*-------------- function prototypes: adapt for platform -------------------*/

/******************************************************************************
* Function Name     : SensoryInfo
* Parameters        : isp = pointer to structure in app
* Comment           : Optional for identification, etc. 
*                   : Fills in isp->version
******************************************************************************/
errors_t SensoryInfo(infoStruct_T *isp);

/******************************************************************************
* Function Name     : SensoryAlloc
* Parameters        : t = pointer to persistent structure common to app and Sensory
*                   : (assumes grammar/net supplied in t2siStruct as above.)
*                   : size = pointer to int to receive persistent memory size (in bytes)
* Comment           : Must be called to discover memory size needed.
*                   : The app could call this function, allocate "size" bytes for
*                   : Sensory's persistent memory, then put a pointer to that
*                   : in appStruct._t.spp. 
******************************************************************************/
errors_t SensoryAlloc(t2siStruct *t, unsigned int *size);

/******************************************************************************
* Function Name     : SensoryAllocMulti
* Parameters        : t = pointer to persistent structure common to app and Sensory
*                   : (assumes grammar/net supplied in t2siStruct as above.)
*                   : size = pointer to int to receive persistent memory size (in bytes)
*                   : channels and depth set up for number of audio streams,
*                   : and the number of audio frames in each stream, to process at once.
*                   : The SensoryAlloc default is one channel with one frame at a time.
* Comment           : Must be called to discover memory size needed.
*                   : The app could call this function, allocate "size" bytes for
*                   : Sensory's persistent memory, then put a pointer to that
*                   : in appStruct._t.spp.
******************************************************************************/
errors_t SensoryAllocMulti(t2siStruct* t, unsigned int* size, int channels, int depth);

/******************************************************************************
* Function Name     : SensoryProcessInit
* Parameters        : assumes grammar, net, and spp supplied in t2siStruct as above.
* Comment           : Mechanism for Sensory to prepare structures for recognition.
*                   : t2siStruct contains various app-controllable parameters.
******************************************************************************/
errors_t SensoryProcessInit(t2siStruct *t);

/******************************************************************************
* Function Name     : SensoryProcessRestart
* Parameters        : assumes grammar/net supplied in t2siStruct as above.
*                   : channel should be the channel that just had a recognition.
* Comment           : Mechanism for Sensory to restart recognition after success 
*                   : or error.  Faster than full initialization done by 
*                   : SensoryProcessInit().  Now optional in v8.2+
******************************************************************************/
errors_t SensoryProcessRestart(t2siStruct* t, int channel);

/******************************************************************************
* Function Name : SensoryProcessData
* Parameters    : Expects BRICK_SIZE_SAMPLES 16-bit samples at 16kHz in buffer pointed by brick.
* Comment       : The workhorse function, called repeatedly.
*               : Expects to be called every BRICK_SIZE_MS milliseconds.
*               : Should execute (on average) under the time associated with BRICK_SIZE_MS.
*               : Typically returns ERR_NOT_FINISHED, or ERR_OK upon recognition
*               : Calls SensoryProcessDataSamples(...) below
******************************************************************************/
RecoResult* SensoryProcessData(t2siStruct* t, SAMPLE *frame);

/******************************************************************************
* Function Name : SensoryProcessDataSamples
* Parameters    : Expects 16-bit samples at 16kHz in buffer pointed by samples.
* Comment       : Can take variable sized sample counts unlike SensoryProcessData
*               : Audio buffer size must be a multiple both of 240 and sampleCount
*               : sampleCount must be not more than 240 (15 ms)
******************************************************************************/
RecoResult* SensoryProcessDataSamples(t2siStruct* t, SAMPLE* samples, s16 sampleCount);

/******************************************************************************
* Function Name : SensoryProcessMultiData
* Parameters    : Expects multiple 240-wide frames of 16-bit samples - a list of lists
*               : frames[0] is the list of samples for channel 0 and so on
*               : Function usually returns ERR__NOT_FINISHED, but ERR_OK if 
                : something was recognized, or other error if there was one
*               : Results are returned via SensoryGetResult(t, channel, depth)
* Comment       : All frames must be 240 samples.
*               : Must correspond to SensoryAllocMulti setup for 
                : channels (count of lists) and depth (frame count in each list.)
******************************************************************************/
errors_t SensoryProcessMultiData(t2siStruct* t, SAMPLE** frames);

/******************************************************************************
* Function Name : SensoryGetResult
* Parameters    : Returns the result for a given channel at a given depth
*               : SensoryGetResult(t, 0, 0) returns the one result of SensoryProcessData
* Comment       : depth 0 is the oldest result
******************************************************************************/
RecoResult* SensoryGetResult(t2siStruct* t, int channel, int depth);

#ifdef SHARE_FEATURES
/******************************************************************************
* Function Name : SensoryFeatureCompatible
* Parameters    : Expects two initialized t2siStruct which are checked 
*               : for feature-compatibility
* Comment       : Returns TRUE if the two apps are feature-compatible.
*               : Used in case of multiple recognizers on same audio stream.
******************************************************************************/
BOOL SensoryFeatureCompatible(t2siStruct* src, t2siStruct* dst);

/******************************************************************************
* Function Name : SensoryConnectFeatures
* Parameters    : Expects two initialized t2siStruct which will get connected for 
*               : transmitting audio features, src to dst.  This will save feature processing time.
* Comment       : Will return ERR_OK if connected successfully or ERR_T2SI_FEATURE_MISMATCH.
*               : if the features are not compatible.
*               : Used in case of multiple recognizers on same audio stream.
******************************************************************************/
errors_t SensoryConnectFeatures(t2siStruct* src, t2siStruct* dst);

/******************************************************************************
* Function Name : SensoryProcessFeatures
* Parameters    : A t2siStruct which will take features from the source and process them
* Comment       : This works just like SensoryProcessData, but faster for compatible recognizers.
*               : Used in case of multiple recognizers on same audio stream.
******************************************************************************/
RecoResult* SensoryProcessFeatures(t2siStruct* t);
#endif  // SHARE_FEATURES

/******************************************************************************
* Function Name     : SensoryGetAudio
* Parameters        : t2siStruct pointer, channel number
* Comment           : Use this function to get a pointer to the audio for that channel
*                   : Returns NULL if channel number is out of bounds,
******************************************************************************/
SAMPLE* SensoryGetAudio(t2siStruct* t, int channel);

/******************************************************************************
* Function Name     : SensoryLibraryLicenseInfo
* Parameters        : pointers to values to receive outputs
* Comment           : Returns FALSE for an invalid license and TRUE for a valid one.
*                   :
*                   : This API gets limitations on duration and recognition events 
*                   : for a valid THF-Micro Library license.
*                   : If the value of seconds is 0, the license has no duration limit.
*                   : If the value of events is 0, the license has no event limit.
******************************************************************************/
BOOL SensoryLibraryLicenseInfo(unsigned* seconds, unsigned* events);

/******************************************************************************
* Function Name     : SensoryModelLicenseInfo
* Parameters        : t2siStruct pointer, pointers to values to receive outputs
* Comment           : Returns FALSE for an invalid license and TRUE for a valid one.
*                   :
*                   : This API gets limitations on duration and recognition events 
*                   : for a valid acoustic model license.
*                   : If the value of seconds is 0, the license has no duration limit.
*                   : If the value of events is 0, the license has no event limit.
******************************************************************************/
BOOL SensoryModelLicenseInfo(t2siStruct* t, unsigned* seconds, unsigned* events);

/*
 * Sensory Logging feature related
 */

#ifdef SENSORY_LOGGING
/******************************************************************************
* Function Name : SensoryLogInit
* Parameters    :
* log:          : Pass the logging buffer of data type of SENSORY_LOG_OBJ
* n             : Size of the logging buffer
* Comment       : Called once to initialize logging variables
******************************************************************************/
void SensoryLogInit(t2siStruct* app, SENSORY_LOG_OBJ *log, int n);

/******************************************************************************
* Function Name : SensoryLog
* Parameters    :
* event:        : Underlying logging event.
* a, b          : Underlying logging event's two arguments
* Comment       : Save the underlying logging function calls into the logging buffer
******************************************************************************/
void SensoryLog(t2siStruct *app, int event, int a, int b);

void SensoryPrintLog(t2siStruct* app);

/*
 * Sensory Logging event definition
 */
#define SENSORY_LOG_RESET          0
#define SENSORY_LOG_CUSTOMER1      1
#define SENSORY_LOG_CUSTOMER2      2 
#define SENSORY_LOG_INIT           3
#define SENSORY_LOG_AUDIO_OVERFLOW 4
#define SENSORY_LOG_OUT_OF_MEMORY  5
#define SENSORY_LOG_LPSD_ABOVE_T   6
#define SENSORY_LOG_LPSD_BELOW_T   7
#define SENSORY_LOG_WS_PASS        8
#define SENSORY_LOG_DPP            9
#define SENSORY_LOG_EPQ            10
#define SENSORY_LOG_RECOG          11
#define SENSORY_LOG_SCORES         12

#define SENSORY_LOG(p,a,b,c) SensoryLog((p),(a),(b),(c))
#else
#define SENSORY_LOG(p,a,b,c)
#endif

/*-------------- SENSORY ERROR CODES for technology routines  -------------------*/
// Sensory error codes are u08 format, will be mapped/cast into app-required values

//-- 0x: Generic errors 
#define ERR_OK                                  0x00    // generic pass return
#define ERR_NOT_OK                              0x01    // generic error return

//-- 1x, 2x: Data collection errors 
#define ERR_DATACOL_TIMEOUT                     0x11 // no utterance detected
#define ERR_DATACOL_TOO_SHORT                   0x13 // utterance was too short (never used in 7.2.1)
#define ERR_DATACOL_TOO_SOFT                    0x14 // utterance was too soft (never used in 7.2.1)

//-- 3x,4x: Recognition errors 
#define ERR_RECOG_FAIL                          0x31 // recognition failed
#define ERR_RECOG_LOW_CONF                      0x32 // recognition result doubtful
#define ERR_RECOG_MID_CONF                      0x33 // recognition result maybe

//-- 5x: T2SI errors       
#define ERR_T2SI_PSTORE                         0x50  // null persistent storage pointer
#define ERR_T2SI_BAD_VERSION                    0x51  // grammar version not supported
#define ERR_T2SI_NN_BAD_VERSION                 0x52  // net version not supported
#define ERR_T2SI_BAD_SETUP                      0x53  // gram or net not specified
#define ERR_T2SI_TRIG_NOTA                      0x54  // trigger NOTA - continues
#define ERR_T2SI_NN_MISMATCH                    0x55  // gram/net mismatch(# of models)
#define ERR_T2SI_TOO_MANY_RESULTS               0x56  // MAX_RESULTS is too small
#define ERR_T2SI_UNEXPECTED_NUM_EXTRA_MODELS    0x57  // too many extra number of models
#define ERR_T2SI_UNEXPECTED_NUM_EXTRA_GRAMS     0x58  // too many extra number of grammars
#define ERR_T2SI_UNEXPECTED_NUM_EXTRA_NETS      0x59  // too many extra number of nets
#define ERR_T2SI_UNEXPECTED_MFCC_TYPE           0x5A  // The MFCC type is not supported
#define ERR_T2SI_GRAMMAR_UNALIGNED              0x5B  // Grammar memory must be 4-byte aligned
#define ERR_T2SI_FEATURE_MISMATCH               0x5C  // SensoryProcessFeatures must have matching features

//-- 6x: DNN errors
#define ERR_DNN_BAD_VERSION                     0x60  // dnn not right format/version
#define ERR_DNN_TOO_MANY_NETS                   0x61  // too many dnn nets (> MAX_DNN_NETS)
#define ERR_DNN_BAD_FORMAT                      0x62
#define ERR_DNN_SV_OR_NNPQ_MISMATCH             0x63  // grammar for nnSv is not SV
#define ERR_DNN_UNALIGNED                       0x64  // Net memory must be 4-byte aligned
#define ERR_DNN_UNSUPPORTED                     0x65  // Unsupported DNN type (e.g. Stage3)

//-- Fx: Setup errors, token errors
#define ERR_HEADER_VERSION                      0xF0  // Header file version does not match library
#define ERR_N_WAY_LIMIT                         0xF1  // Too many or too few channels / depth
#define ERR_SETUP_NOT_SUPPORTED                 0xF2  // Settings incompatible with each other - not supported
#define ERR_AUDIOBUFFER_BAD_SIZE                0xF9  // Audio buffer not evenly divisible by sample count
#define ERR_SEARCH_PRUNED                       0xFA  // warning that search is being limited by maxTokens
#define ERR_MEMORY_CORRUPT                      0xFB  // RAM apparently changed
#define ERR_NULL_POINTER                        0xFC  // fatal null pointer
#define ERR_NOT_FINISHED                        0xFD  // non-fatal, need more data
#define ERR_NO_FREE_TOKENS                      0xFE  // no free tokens left
#define ERR_LICENSE                             0xFF  // license isn't valid or event limit reached

#define MAJOR_HEADER_VERS (8)
#define MINOR_HEADER_VERS (3)
#define POINT_HEADER_VERS (2)
#define SENSORY_HEADER_VERSION ((MAJOR_HEADER_VERS<<20)+(MINOR_HEADER_VERS<<12) + POINT_HEADER_VERS)

#define SensoryAlloc(t, size) SensoryAllocVersion(t, size, SENSORY_HEADER_VERSION, 1, 1)

#define SensoryAllocMulti(t, size, channels, depth) \
     SensoryAllocVersion(t, size, SENSORY_HEADER_VERSION, channels, depth)
/******************************************************************************
* Function Name     : SensoryAllocVersion
* Comment           : User should not call this directly.
*                   : Call SensoryAlloc(t, size) instead.
******************************************************************************/
errors_t SensoryAllocVersion(t2siStruct* t, unsigned int* size, unsigned int version, int channels, int depth);

#endif  //SENSORY_LIB_INCLUDED

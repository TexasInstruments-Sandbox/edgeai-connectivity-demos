/* Speech Recognition HMM Grammar Vocabulary Description file
 * Copyright (C) 2003-2022 Sensory, Inc. All Rights Reserved.
 * 
 *
 *            source: /tmp/tmp.W4ChnvM4au/sensory-thf-enUS-voice_genie-e2aa3f897.snsr
 *           created: Fri Oct 11 17:40:23 2024
 *   min lib version: 6.1.0
 *   operating point: 8
 *  production ready: yes
 *       license key: no
 *
 * Created by VoiceHub 2.10.0
 * Project: Door_Lock_and_Unlock_in_English_wakeword
 */

extern const unsigned short gs_wakeword_grammarLabel[];
#ifndef NETLABEL
#define NETLABEL
extern const unsigned short dnn_wakeword_netLabel[];
#endif

/* The following phrases (Hex format) correspond to the word IDs emitted by the recognizer. */
#define WAKEWORD_PHRASE_COUNT 3
#define WAKEWORD_PHRASE_0 "SILENCE"	/* Legacy system phrase */
#define WAKEWORD_PHRASE_1 "\x76\x6F\x69\x63\x65\x5F\x67\x65\x6E\x69\x65"	/* Phrase: voice_genie */
#define WAKEWORD_PHRASE_2 "nota"	/* Legacy system phrase */

/* Speech Recognition HMM Grammar Vocabulary Description file
 * Copyright (C) 2003-2022 Sensory, Inc. All Rights Reserved.
 * 
 *
 *            source: /tmp/tmp.hWpn9Ggwdv/trecs-en_US_12-13-6-0_0ffbd4a9d7b83946eebed70b249cfa4ebb845373.snsr
 *           created: Tue Nov 26 22:03:50 2024
 *   min lib version: 6.1.0
 *   operating point: 10
 *  production ready: yes
 *       license key: no
 *
 * Created by VoiceHub 2.11.3
 * Project: Toggle_on_and_off_LEDs_on_Launchpad_Example_command
 */

#ifndef CMDLABELEN
#define CMDLABELEN
extern const unsigned short gs_en_command_grammarLabel[];
extern const unsigned short dnn_en_command_netLabel[];
#endif

/* The following phrases (Hex format) correspond to the word IDs emitted by the recognizer. */
#define COMMAND_PHRASE_COUNT 4
#define COMMAND_PHRASE_0 "SILENCE"	/* Legacy system phrase */
#define COMMAND_PHRASE_1 "\x74\x6F\x67\x67\x6C\x65\x5F\x67\x72\x65\x65\x6E\x5F\x6C\x65\x64"	/* Phrase: toggle_green_led */
#define COMMAND_PHRASE_2 "\x74\x6F\x67\x67\x6C\x65\x5F\x72\x65\x64\x5F\x6C\x65\x64"	/* Phrase: toggle_red_led */
#define COMMAND_PHRASE_3 "nota"	/* Legacy system phrase */

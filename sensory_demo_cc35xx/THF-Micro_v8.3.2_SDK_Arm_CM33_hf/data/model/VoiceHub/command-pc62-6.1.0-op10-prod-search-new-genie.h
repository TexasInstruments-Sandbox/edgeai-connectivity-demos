/* Speech Recognition HMM Grammar Vocabulary Description file
 * Copyright (C) 2003-2022 Sensory, Inc. All Rights Reserved.
 * 
 *
 *            source: /tmp/tmp.V7L6HpLQ2b/trecs-en_US_12-13-6-0_e08b648a1d28746116edfb0264cb05b3bf518dd8.snsr
 *           created: Mon Aug  4 18:52:09 2025
 *   min lib version: 6.1.0
 *   operating point: 10
 *  production ready: yes
 *       license key: no
 *
 * Created by VoiceHub 2.13.16
 * Project: Command_Set_command
 */

extern const unsigned short gs_en_command_grammarLabel[];
#ifndef CMDNETLABEL
#define CMDNETLABEL
extern const unsigned short dnn_en_command_netLabel[];
#endif

/* The following phrases (Hex format) correspond to the word IDs emitted by the recognizer. */
#define COMMAND_PHRASE_COUNT 6
#define COMMAND_PHRASE_0 "SILENCE"	/* Legacy system phrase */
#define COMMAND_PHRASE_1 "\x74\x6F\x67\x67\x6C\x65\x5F\x67\x72\x65\x65\x6E\x5F\x6C\x65\x64"	/* Phrase: toggle_green_led */
#define COMMAND_PHRASE_2 "\x74\x6F\x67\x67\x6C\x65\x5F\x72\x65\x64\x5F\x6C\x65\x64"	/* Phrase: toggle_red_led */
#define COMMAND_PHRASE_3 "\x74\x6F\x67\x67\x6C\x65\x5F\x62\x6C\x75\x65\x5F\x6C\x65\x64"	/* Phrase: toggle_blue_led */
#define COMMAND_PHRASE_4 "\x74\x6F\x67\x67\x6C\x65\x5F\x61\x6C\x6C\x5F\x6C\x65\x64"	/* Phrase: toggle_all_led */
#define COMMAND_PHRASE_5 "nota"	/* Legacy system phrase */

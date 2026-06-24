#ifndef __SOUND_SEQUENCES_H__
#define __SOUND_SEQUENCES_H__

#include <Arduino.h>

/*
 * SoundSequences - Unified sound sequencing for Stern SB-100
 *
 * Provides named sound sequences that chain tones + silences cleanly,
 * eliminating overlap and scattered QueueSound calls throughout the codebase.
 *
 * Uses interrupt-and-mute: one active sequence, one muted sequence.
 * New sound interrupts active (if no large offset), active resumes when new finishes.
 * Timing: 75ms silences, 200ms spacing between tones (matches original SB-100).
 */

// Sound sequence entry (stored in PROGMEM)
struct SoundStep {
  byte tone;           // SND_* constant (or 0 for silence)
  unsigned int gap_ms; // ms from sequence start to play this step
};

// Sequence IDs (enum-like constants)
#define SEQ_SCORE_100          0
#define SEQ_SCORE_300          1
#define SEQ_SCORE_500          2
#define SEQ_SCORE_1000         3
#define SEQ_SCORE_3000         4
#define SEQ_SCORE_5000         5
#define SEQ_SCORE_6000         6
#define SEQ_SCORE_9000         7
#define SEQ_SCORE_10000        8
#define SEQ_SCORE_10           9

#define SEQ_ADVANCE_1          10
#define SEQ_ADVANCE_3          11

#define SEQ_POP_BUMPER         20
#define SEQ_TILT_WARNING       21
#define SEQ_TILT               22
#define SEQ_GAME_OVER          23
#define SEQ_MATCH_SPIN         24
#define SEQ_BONUS_COUNT        25
#define SEQ_STARTUP            26
#define SEQ_DRAIN              27

#define SEQ_FANFARE_ASCENDING  30
#define SEQ_FANFARE_5BANK      31

#define NUM_SOUND_SEQUENCES    32  // size of SoundSequenceTable

// Play a named sound sequence, optionally offset from CurrentTime
// Returns: total duration of sequence in ms (for chaining)
unsigned int PlaySoundSequence(byte seqID, unsigned long startOffset = 0);

// Get sequence metadata for display sync
// Returns: number of tones (excluding silence/sentinel)
byte GetSequenceToneCount(byte seqID);

// Returns: gap in ms between consecutive tones (assumes uniform spacing)
unsigned int GetSequenceToneSpacing(byte seqID);

// Extern declarations for sequence table access
extern const SoundStep* const SoundSequenceTable[] PROGMEM;

#endif

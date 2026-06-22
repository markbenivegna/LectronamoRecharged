#include <Arduino.h>
#include "RPU_Config.h"
#include "RPU.h"
#include "SoundSequences.h"
#include "Lectronamo.h"
#include "AudioHandler.h"

extern AudioHandler Audio;
extern unsigned long CurrentTime;

/*
  DEBUG OUTPUT FORMAT:
  SEQ[id]: START @ time ms (offset +offset ms)     - sequence queued
    SEQ[id] TONE[i]: tone=N @ time ms (gap +X ms)  - tone queued at absolute timestamp
    SEQ[id] SLEN[i]: silence @ time ms              - silence queued
  SEQ[id]: END duration=D ms                         - sequence complete

  Use this to identify sound collisions: overlapping tone timestamps = collision
*/

// Sequence definitions (PROGMEM - each step is {tone, gap_ms})
// Sentinel: {0xFF, 0}

// Score sequences — each plays the appropriate tone(s) with 200ms base spacing
// Silence automatically inserted after each tone at gap_ms + 75ms (Dick's timing)
// Note: 200ms gap ensures tones don't queue until previous silence (75ms) finishes
const SoundStep SEQ_SCORE_100_Data[] PROGMEM = {
  {SND_100_POINTS, 0},
  {0xFF, 0}
};

const SoundStep SEQ_SCORE_300_Data[] PROGMEM = {
  {SND_100_POINTS, 0},
  {SND_100_POINTS, 200},
  {SND_100_POINTS, 400},
  {0xFF, 0}
};

const SoundStep SEQ_SCORE_500_Data[] PROGMEM = {
  {SND_100_POINTS, 0},
  {SND_100_POINTS, 200},
  {SND_100_POINTS, 400},
  {SND_100_POINTS, 600},
  {SND_100_POINTS, 800},
  {0xFF, 0}
};

const SoundStep SEQ_SCORE_1000_Data[] PROGMEM = {
  {SND_1000_POINTS, 0},
  {0xFF, 0}
};

const SoundStep SEQ_SCORE_3000_Data[] PROGMEM = {
  {SND_1000_POINTS, 0},
  {SND_1000_POINTS, 200},
  {SND_1000_POINTS, 400},
  {0xFF, 0}
};

const SoundStep SEQ_SCORE_5000_Data[] PROGMEM = {
  {SND_1000_POINTS, 0},
  {SND_1000_POINTS, 200},
  {SND_1000_POINTS, 400},
  {SND_1000_POINTS, 600},
  {SND_1000_POINTS, 800},
  {0xFF, 0}
};

const SoundStep SEQ_SCORE_6000_Data[] PROGMEM = {
  {SND_1000_POINTS, 0},
  {SND_1000_POINTS, 200},
  {SND_1000_POINTS, 400},
  {SND_1000_POINTS, 600},
  {SND_1000_POINTS, 800},
  {SND_1000_POINTS, 1000},
  {0xFF, 0}
};

const SoundStep SEQ_SCORE_9000_Data[] PROGMEM = {
  {SND_1000_POINTS, 0},
  {SND_1000_POINTS, 200},
  {SND_1000_POINTS, 400},
  {SND_1000_POINTS, 600},
  {SND_1000_POINTS, 800},
  {SND_1000_POINTS, 1000},
  {SND_1000_POINTS, 1200},
  {SND_1000_POINTS, 1400},
  {SND_1000_POINTS, 1600},
  {0xFF, 0}
};

const SoundStep SEQ_SCORE_10000_Data[] PROGMEM = {
  {SND_10000_POINTS, 0},
  {0xFF, 0}
};

// Advance bonus sounds — deep tone (SND_10000_POINTS)
const SoundStep SEQ_ADVANCE_1_Data[] PROGMEM = {
  {SND_10000_POINTS, 0},
  {0xFF, 0}
};

const SoundStep SEQ_ADVANCE_3_Data[] PROGMEM = {
  {SND_10000_POINTS, 0},
  {SND_10000_POINTS, 200},
  {SND_10000_POINTS, 400},
  {0xFF, 0}
};

// Game event sounds
const SoundStep SEQ_POP_BUMPER_Data[] PROGMEM = {
  {SND_POP_BUMPER, 0},
  {0xFF, 0}
};

const SoundStep SEQ_TILT_WARNING_Data[] PROGMEM = {
  {SND_1000_POINTS, 0},
  {0xFF, 0}
};

const SoundStep SEQ_TILT_Data[] PROGMEM = {
  {SND_10000_POINTS, 0},
  {0xFF, 0}
};

const SoundStep SEQ_GAME_OVER_Data[] PROGMEM = {
  {SND_10_POINTS, 0},
  {SND_100_POINTS, 150},
  {SND_1000_POINTS, 300},
  {SND_10000_POINTS, 450},
  {SND_10_POINTS, 600},
  {SND_100_POINTS, 750},
  {SND_1000_POINTS, 900},
  {SND_10000_POINTS, 1050},
  {0xFF, 0}
};

const SoundStep SEQ_MATCH_SPIN_Data[] PROGMEM = {
  {SND_100_POINTS, 0},
  {0xFF, 0}
};

const SoundStep SEQ_BONUS_COUNT_Data[] PROGMEM = {
  {SND_100_POINTS, 0},
  {0xFF, 0}
};

const SoundStep SEQ_STARTUP_Data[] PROGMEM = {
  {SND_10000_POINTS, 0},
  {SND_1000_POINTS, 150},
  {SND_100_POINTS, 300},
  {SND_10_POINTS, 450},
  {SND_10000_POINTS, 600},
  {SND_1000_POINTS, 750},
  {SND_100_POINTS, 900},
  {SND_10_POINTS, 1050},
  {0xFF, 0}
};

// Drain sound — 3-tone dramatic warning (fast mid, mid, then deep)
const SoundStep SEQ_DRAIN_Data[] PROGMEM = {
  {SND_1000_POINTS, 0},
  {SND_1000_POINTS, 200},
  {SND_10000_POINTS, 700},
  {0xFF, 0}
};

// Fanfare sequences (all 150ms spacing for consistent feel)
const SoundStep SEQ_FANFARE_ASCENDING_Data[] PROGMEM = {
  {SND_100_POINTS, 0},
  {SND_1000_POINTS, 150},
  {SND_100_POINTS, 300},
  {SND_10_POINTS, 450},
  {0xFF, 0}
};

const SoundStep SEQ_FANFARE_5BANK_Data[] PROGMEM = {
  {SND_10000_POINTS, 0},
  {SND_1000_POINTS, 150},
  {SND_100_POINTS, 300},
  {SND_10_POINTS, 450},
  {0xFF, 0}
};

// Lookup table: maps seqID to PROGMEM data pointer
const SoundStep* const SoundSequenceTable[] PROGMEM = {
  SEQ_SCORE_100_Data,        // 0
  SEQ_SCORE_300_Data,        // 1
  SEQ_SCORE_500_Data,        // 2
  SEQ_SCORE_1000_Data,       // 3
  SEQ_SCORE_3000_Data,       // 4
  SEQ_SCORE_5000_Data,       // 5
  SEQ_SCORE_6000_Data,       // 6
  SEQ_SCORE_9000_Data,       // 7
  SEQ_SCORE_10000_Data,      // 8
  NULL,                      // 9 (unused)
  SEQ_ADVANCE_1_Data,        // 10
  SEQ_ADVANCE_3_Data,        // 11
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, // 12-19 (unused)
  SEQ_POP_BUMPER_Data,       // 20
  SEQ_TILT_WARNING_Data,     // 21
  SEQ_TILT_Data,             // 22
  SEQ_GAME_OVER_Data,        // 23
  SEQ_MATCH_SPIN_Data,       // 24
  SEQ_BONUS_COUNT_Data,      // 25
  SEQ_STARTUP_Data,          // 26
  SEQ_DRAIN_Data,            // 27
  NULL, NULL,                // 28-29 (unused)
  SEQ_FANFARE_ASCENDING_Data, // 30
  SEQ_FANFARE_5BANK_Data     // 31
};

unsigned int PlaySoundSequence(byte seqID, unsigned long startOffset) {
  // Bounds check
  if (seqID >= NUM_SOUND_SEQUENCES) {
    char buf[64];
    sprintf(buf, "SEQ: Invalid seqID %d\n", seqID);
    Serial.write(buf);
    return 0;
  }

  // Get PROGMEM pointer to this sequence
  const SoundStep* seqPtr = (const SoundStep*)pgm_read_ptr(&SoundSequenceTable[seqID]);
  if (!seqPtr) {
    char buf[64];
    sprintf(buf, "SEQ: Null pointer for seqID %d\n", seqID);
    Serial.write(buf);
    return 0;
  }

  unsigned int maxGap = 0;

  // Calculate total duration first
  for (int i = 0; ; i++) {
    SoundStep step;
    memcpy_P(&step, &seqPtr[i], sizeof(SoundStep));
    if (step.tone == 0xFF) {
      break;
    }
    if (step.gap_ms > maxGap) {
      maxGap = step.gap_ms;
    }
  }

  // Queue the sequence for playback
  // Interrupts current sequence, but only clears that sequence's queued sounds
  Audio.QueueSequence(seqID, startOffset);

  unsigned int silenceDuration = 75;  // Dick's timing: 75ms silences
  unsigned int duration = maxGap + silenceDuration;

  // Return total duration for caller's timing reference
  return duration;
}

byte GetSequenceToneCount(byte seqID) {
  // Bounds check
  if (seqID >= NUM_SOUND_SEQUENCES) return 0;

  // Get PROGMEM pointer
  const SoundStep* seqPtr = (const SoundStep*)pgm_read_ptr(&SoundSequenceTable[seqID]);
  if (!seqPtr) return 0;

  byte count = 0;
  for (int i = 0; ; i++) {
    SoundStep step;
    memcpy_P(&step, &seqPtr[i], sizeof(SoundStep));
    if (step.tone == 0xFF) break;  // Sentinel marks end
    count++;
  }
  return count;
}

unsigned int GetSequenceToneSpacing(byte seqID) {
  // Bounds check
  if (seqID >= NUM_SOUND_SEQUENCES) return 0;

  // Get PROGMEM pointer
  const SoundStep* seqPtr = (const SoundStep*)pgm_read_ptr(&SoundSequenceTable[seqID]);
  if (!seqPtr) return 0;

  // Get first two tones to determine spacing
  SoundStep step0, step1;
  memcpy_P(&step0, &seqPtr[0], sizeof(SoundStep));
  if (step0.tone == 0xFF) return 0;  // Single tone, no spacing

  memcpy_P(&step1, &seqPtr[1], sizeof(SoundStep));
  if (step1.tone == 0xFF) return 0;  // Only one tone

  // Return gap between first and second tone
  return step1.gap_ms - step0.gap_ms;
}

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

// Score sequences — tones on a uniform 175ms grid. The sequence player holds
// each tone 125ms, leaving a 50ms audible gap between tones (long-tone,
// short-gap, matching the original ROM's feel).
const SoundStep SEQ_SCORE_10_Data[] PROGMEM = {
  {SND_10_POINTS, 0},
  {0xFF, 0}
};

const SoundStep SEQ_SCORE_100_Data[] PROGMEM = {
  {SND_100_POINTS, 0},
  {0xFF, 0}
};

const SoundStep SEQ_SCORE_300_Data[] PROGMEM = {
  {SND_100_POINTS, 0},
  {SND_100_POINTS, 175},
  {SND_100_POINTS, 350},
  {0xFF, 0}
};

const SoundStep SEQ_SCORE_500_Data[] PROGMEM = {
  {SND_100_POINTS, 0},
  {SND_100_POINTS, 175},
  {SND_100_POINTS, 350},
  {SND_100_POINTS, 525},
  {SND_100_POINTS, 700},
  {0xFF, 0}
};

const SoundStep SEQ_SCORE_1000_Data[] PROGMEM = {
  {SND_1000_POINTS, 0},
  {0xFF, 0}
};

const SoundStep SEQ_SCORE_3000_Data[] PROGMEM = {
  {SND_1000_POINTS, 0},
  {SND_1000_POINTS, 175},
  {SND_1000_POINTS, 350},
  {0xFF, 0}
};

const SoundStep SEQ_SCORE_5000_Data[] PROGMEM = {
  {SND_1000_POINTS, 0},
  {SND_1000_POINTS, 175},
  {SND_1000_POINTS, 350},
  {SND_1000_POINTS, 525},
  {SND_1000_POINTS, 700},
  {0xFF, 0}
};

const SoundStep SEQ_SCORE_6000_Data[] PROGMEM = {
  {SND_1000_POINTS, 0},
  {SND_1000_POINTS, 175},
  {SND_1000_POINTS, 350},
  {SND_1000_POINTS, 525},
  {SND_1000_POINTS, 700},
  {SND_1000_POINTS, 875},
  {0xFF, 0}
};

const SoundStep SEQ_SCORE_9000_Data[] PROGMEM = {
  {SND_1000_POINTS, 0},
  {SND_1000_POINTS, 175},
  {SND_1000_POINTS, 350},
  {SND_1000_POINTS, 525},
  {SND_1000_POINTS, 700},
  {SND_1000_POINTS, 875},
  {SND_1000_POINTS, 1050},
  {SND_1000_POINTS, 1225},
  {SND_1000_POINTS, 1400},
  {0xFF, 0}
};

const SoundStep SEQ_SCORE_10000_Data[] PROGMEM = {
  {SND_10000_POINTS, 0},
  {0xFF, 0}
};

// Advance bonus sounds — laser tone (SND_ADD_BONUS)
const SoundStep SEQ_ADVANCE_1_Data[] PROGMEM = {
  {SND_ADD_BONUS, 0},
  {0xFF, 0}
};

const SoundStep SEQ_ADVANCE_3_Data[] PROGMEM = {
  {SND_ADD_BONUS, 0},
  {SND_ADD_BONUS, 250},
  {SND_ADD_BONUS, 500},
  {0xFF, 0}
};

// Combined score + advance sequences, interleaved on a uniform 120ms grid.
// Both open with a single score tone followed by an advance.
const SoundStep SEQ_SCORE_3000_WITH_ADVANCE_3_Data[] PROGMEM = {
  {SND_1000_POINTS, 0},      // Score tone 1
  {SND_ADD_BONUS, 120},      // Advance tone 1
  {SND_1000_POINTS, 240},    // Score tone 2
  {SND_ADD_BONUS, 360},      // Advance tone 2
  {SND_1000_POINTS, 480},    // Score tone 3
  {SND_ADD_BONUS, 600},      // Advance tone 3
  {0xFF, 0}
};

const SoundStep SEQ_SCORE_5000_WITH_ADVANCE_3_Data[] PROGMEM = {
  {SND_1000_POINTS, 0},      // Score tone 1
  {SND_ADD_BONUS, 120},      // Advance tone 1
  {SND_1000_POINTS, 240},    // Score tone 2
  {SND_1000_POINTS, 360},    // Score tone 3
  {SND_ADD_BONUS, 480},      // Advance tone 2
  {SND_1000_POINTS, 600},    // Score tone 4
  {SND_1000_POINTS, 720},    // Score tone 5
  {SND_ADD_BONUS, 840},      // Advance tone 3
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


const SoundStep SEQ_MATCH_SPIN_Data[] PROGMEM = {
  {SND_100_POINTS, 0},
  {0xFF, 0}
};

const SoundStep SEQ_BONUS_COUNT_Data[] PROGMEM = {
  {SND_100_POINTS, 0},
  {0xFF, 0}
};


// Drain sound — 3-tone dramatic warning (fast mid, mid, then deep)
const SoundStep SEQ_DRAIN_Data[] PROGMEM = {
  {SND_1000_POINTS, 0},
  {SND_1000_POINTS, 175},
  {SND_10000_POINTS, 700},
  {0xFF, 0}
};

// Fanfare sequences (all 130ms spacing for consistent feel)
const SoundStep SEQ_FANFARE_ASCENDING_Data[] PROGMEM = {
  {SND_100_POINTS, 0},
  {SND_1000_POINTS, 130},
  {SND_100_POINTS, 260},
  {SND_10_POINTS, 390},
  {0xFF, 0}
};

const SoundStep SEQ_FANFARE_5BANK_Data[] PROGMEM = {
  {SND_10000_POINTS, 0},
  {SND_1000_POINTS, 130},
  {SND_100_POINTS, 260},
  {SND_10_POINTS, 390},
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
  SEQ_SCORE_10_Data,         // 9 (10-point score)
  SEQ_ADVANCE_1_Data,        // 10
  SEQ_ADVANCE_3_Data,        // 11
  SEQ_SCORE_3000_WITH_ADVANCE_3_Data,  // 12
  SEQ_SCORE_5000_WITH_ADVANCE_3_Data,  // 13
  NULL, NULL, NULL, NULL, NULL, NULL, // 14-19 (unused)
  SEQ_POP_BUMPER_Data,       // 20
  SEQ_TILT_WARNING_Data,     // 21
  SEQ_TILT_Data,             // 22
  NULL,                      // 23 (GAME_OVER — hard-coded in ShowMatchSequence)
  SEQ_MATCH_SPIN_Data,       // 24
  SEQ_BONUS_COUNT_Data,      // 25
  NULL,                      // 26 (STARTUP — hard-coded in setup())
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

  unsigned int lastToneHold = 125;  // matches the sequence player's final-tone hold
  unsigned int duration = maxGap + lastToneHold;

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

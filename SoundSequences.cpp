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

// Score sequences — each plays the appropriate tone(s) with 125ms base spacing
// Silence automatically inserted after each tone at gap_ms + 150
const SoundStep SEQ_SCORE_100_Data[] PROGMEM = {
  {SND_100_POINTS, 0},
  {0xFF, 0}
};

const SoundStep SEQ_SCORE_300_Data[] PROGMEM = {
  {SND_100_POINTS, 0},
  {SND_100_POINTS, 125},
  {SND_100_POINTS, 250},
  {0xFF, 0}
};

const SoundStep SEQ_SCORE_500_Data[] PROGMEM = {
  {SND_100_POINTS, 0},
  {SND_100_POINTS, 125},
  {SND_100_POINTS, 250},
  {SND_100_POINTS, 375},
  {SND_100_POINTS, 500},
  {0xFF, 0}
};

const SoundStep SEQ_SCORE_1000_Data[] PROGMEM = {
  {SND_1000_POINTS, 0},
  {0xFF, 0}
};

const SoundStep SEQ_SCORE_3000_Data[] PROGMEM = {
  {SND_1000_POINTS, 0},
  {SND_1000_POINTS, 125},
  {SND_1000_POINTS, 250},
  {0xFF, 0}
};

const SoundStep SEQ_SCORE_5000_Data[] PROGMEM = {
  {SND_1000_POINTS, 0},
  {SND_1000_POINTS, 125},
  {SND_1000_POINTS, 250},
  {SND_1000_POINTS, 375},
  {SND_1000_POINTS, 500},
  {0xFF, 0}
};

const SoundStep SEQ_SCORE_6000_Data[] PROGMEM = {
  {SND_1000_POINTS, 0},
  {SND_1000_POINTS, 125},
  {SND_1000_POINTS, 250},
  {SND_1000_POINTS, 375},
  {SND_1000_POINTS, 500},
  {SND_1000_POINTS, 625},
  {0xFF, 0}
};

const SoundStep SEQ_SCORE_9000_Data[] PROGMEM = {
  {SND_1000_POINTS, 0},
  {SND_1000_POINTS, 125},
  {SND_1000_POINTS, 250},
  {SND_1000_POINTS, 375},
  {SND_1000_POINTS, 500},
  {SND_1000_POINTS, 625},
  {SND_1000_POINTS, 750},
  {SND_1000_POINTS, 875},
  {SND_1000_POINTS, 1000},
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

const SoundStep SEQ_STARTUP_Data[] PROGMEM = {
  {SND_10000_POINTS, 0},
  {SND_1000_POINTS, 200},
  {SND_100_POINTS, 400},
  {SND_10_POINTS, 600},
  {0xFF, 0}
};

// Drain sound — 3-tone dramatic warning (fast mid, mid, then deep)
const SoundStep SEQ_DRAIN_Data[] PROGMEM = {
  {SND_1000_POINTS, 0},
  {SND_1000_POINTS, 200},
  {SND_10000_POINTS, 700},
  {0xFF, 0}
};

// Fanfare sequences
const SoundStep SEQ_FANFARE_ASCENDING_Data[] PROGMEM = {
  {SND_10_POINTS, 0},
  {SND_100_POINTS, 150},
  {SND_1000_POINTS, 300},
  {SND_10000_POINTS, 450},
  {0xFF, 0}
};

const SoundStep SEQ_FANFARE_5BANK_Data[] PROGMEM = {
  {SND_10000_POINTS, 0},
  {SND_1000_POINTS, 350},
  {SND_100_POINTS, 700},
  {SND_10_POINTS, 1050},
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
  if (seqID >= (sizeof(SoundSequenceTable) / sizeof(SoundSequenceTable[0]))) {
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
  char buf[128];

  sprintf(buf, "SEQ[%d]: START @ %lu ms (offset +%lu ms)\n", seqID, CurrentTime, startOffset);
  Serial.write(buf);

  // Read and queue each step from PROGMEM
  for (int i = 0; ; i++) {
    // Read step from PROGMEM
    SoundStep step;
    memcpy_P(&step, &seqPtr[i], sizeof(SoundStep));

    // Check for sentinel
    if (step.tone == 0xFF) {
      break;
    }

    // Look ahead to see if this is the last tone
    SoundStep nextStep;
    memcpy_P(&nextStep, &seqPtr[i + 1], sizeof(SoundStep));
    boolean isLastTone = (nextStep.tone == 0xFF);

    // Queue the tone
    unsigned long playTime = CurrentTime + startOffset + step.gap_ms;
    Audio.QueueSound(step.tone, AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS, playTime);

    sprintf(buf, "  SEQ[%d] TONE[%d]: tone=%d @ %lu ms (gap +%d ms)\n", seqID, i, step.tone, playTime, step.gap_ms);
    Serial.write(buf);

    // Track max gap for duration calculation
    if (step.gap_ms > maxGap) {
      maxGap = step.gap_ms;
    }

    // Auto-insert silence after this tone (special handling for drain sound's last tone)
    unsigned int silenceDuration;
    if (seqID == 24) {
      silenceDuration = 50;  // SEQ_MATCH_SPIN
    } else if (seqID == 27 && isLastTone) {
      silenceDuration = 400; // SEQ_DRAIN — hold only the last tone longer
    } else {
      silenceDuration = 150;
    }
    unsigned long silenceTime = playTime + silenceDuration;
    Audio.QueueSound(0, AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS, silenceTime);

    sprintf(buf, "  SEQ[%d] SLEN[%d]: silence @ %lu ms\n", seqID, i, silenceTime);
    Serial.write(buf);
  }

  unsigned int silenceDuration = (seqID == 24) ? 50 : 150;  // 24 = SEQ_MATCH_SPIN
  unsigned int duration = maxGap + silenceDuration;
  sprintf(buf, "SEQ[%d]: END duration=%d ms\n", seqID, duration);
  Serial.write(buf);

  // Return total duration: last tone start + tone decay
  return duration;
}

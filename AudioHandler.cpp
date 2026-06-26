/****************************************************************
 * Class - AudioHandler
 * 
   * This class wraps the different audio output options for 
   * pinball machines (WAV Trigger, SB-100, SB-300, S&T
   * -51, etc.) to provide different output options. Additionally,
   * it adds a bunch of audio management features:
   * 
   *   1) Different volume controls for FX, callouts, and music
   *   2) Automatically ducks music behind callouts
   *   3) Supports background soundtracks or looping songs
   *   4) A sound can be queued to play at a future time
   *   5) Callouts can be given different priorities
   *   6) Queued callouts at the same priority will be stacked
 *   
 *   
 * Typical usage:
 *   A global variable of class "AudioHandler" is declared
 *     (ex: "AudioHandler Audio;")
 * 
 *   During the setup() function:
 *    Audio.InitDevices(AUDIO_PLAY_TYPE_WAV_TRIGGER | AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS); // declare what type of audio should be supported
 *    Audio.StopAllAudio(); // Stop any currently playing sounds 
 *    Audio.SetMusicDuckingGain(12); // negative gain applied to music when callouts are being played
 *    Audio.QueueSound(SOUND_EFFECT_MACHINE_INTRO, AUDIO_PLAY_TYPE_WAV_TRIGGER, CurrentTime+1200); // Play machine startup sound in 1.2 s
 *   
 *   Volumes set (pulled from EEPROM or set in setup routine):
 *    Audio.SetMusicVolume(MusicVolume); // value from 0-10 (inclusive)
 *    Audio.SetSoundFXVolume(SoundEffectsVolume); // value from 0-10 (inclusive)
 *    Audio.SetNotificationsVolume(CalloutsVolume); // value from 0-10 (inclusive)
 *   
 *   During the loop():
 *    Audio.Update(CurrentTime);
 *   
 *   During game play:
 *    Audio.PlayBackgroundSong(songNum, true); // loop a background song
 *    Audio.PlaySound(soundEffectNum, AUDIO_PLAY_TYPE_WAV_TRIGGER); // play sound effect through wav trigger
 *    Audio.QueueSound(0x02, AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS, CurrentTime); // Queue sound card command for now
 *    Audio.QueueNotification(soundEffectNum, VoiceNotificationDurations[soundEffectNum-SOUND_EFFECT_VP_VOICE_NOTIFICATIONS_START], priority, CurrentTime); // Queue notification
 *    
 *   End of ball:
 *    Audio.StopAllAudio(); // Stop audio
 */


#include <Arduino.h>
#include "AudioHandler.h"
#include "SoundSequences.h"

extern unsigned long CurrentTime;


#if defined(RPU_OS_USE_WAV_TRIGGER) || defined(RPU_OS_USE_WAV_TRIGGER_1p3)
// **************************************************************
void wavTrigger::start(void) {

uint8_t txbuf[5];

  versionRcvd = false;
  sysinfoRcvd = false;
  WTSerial.begin(57600);
  flush();

  // Request version string
  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x05;
  txbuf[3] = CMD_GET_VERSION;
  txbuf[4] = EOM;
  WTSerial.write(txbuf, 5);

  // Request system info
  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x05;
  txbuf[3] = CMD_GET_SYS_INFO;
  txbuf[4] = EOM;
  WTSerial.write(txbuf, 5);
}

// **************************************************************
void wavTrigger::flush(void) {

int i;
//uint8_t dat;

  rxCount = 0;
  rxLen = 0;
  rxMsgReady = false;
  for (i = 0; i < MAX_NUM_VOICES; i++) {
    voiceTable[i] = 0xffff;
  }
  while(WTSerial.available())
    /*dat = */WTSerial.read();
}


// **************************************************************
void wavTrigger::update(void) {

if (RPU_OS_HARDWARE_REV<=3) return;

int i;
uint8_t dat;
uint8_t voice;
uint16_t track;

  rxMsgReady = false;
  while (WTSerial.available() > 0) {
    dat = WTSerial.read();
    if ((rxCount == 0) && (dat == SOM1)) {
      rxCount++;
    }
    else if (rxCount == 1) {
      if (dat == SOM2)
        rxCount++;
      else {
        rxCount = 0;
        //Serial.print("Bad msg 1\n");
      }
    }
    else if (rxCount == 2) {
      if (dat <= MAX_MESSAGE_LEN) {
        rxCount++;
        rxLen = dat - 1;
      }
      else {
        rxCount = 0;
        //Serial.print("Bad msg 2\n");
      }
    }
    else if ((rxCount > 2) && (rxCount < rxLen)) {
      rxMessage[rxCount - 3] = dat;
      rxCount++;
    }
    else if (rxCount == rxLen) {
      if (dat == EOM)
        rxMsgReady = true;
      else {
        rxCount = 0;
        //Serial.print("Bad msg 3\n");
      }
    }
    else {
      rxCount = 0;
      //Serial.print("Bad msg 4\n");
    }

    if (rxMsgReady) {
      switch (rxMessage[0]) {

        case RSP_TRACK_REPORT:
          track = rxMessage[2];
          track = (track * 256) + rxMessage[1] + 1;
          voice = rxMessage[3];
          if (voice < MAX_NUM_VOICES) {
            if (rxMessage[4] == 0) {
              if (track == voiceTable[voice])
                voiceTable[voice] = 0xffff;
            }
            else
              voiceTable[voice] = track;
          }
          // ==========================
          //Serial.print("Track ");
          //Serial.print(track);
          //if (rxMessage[4] == 0)
          //  Serial.print(" off\n");
          //else
          //  Serial.print(" on\n");
          // ==========================
        break;

        case RSP_VERSION_STRING:
          for (i = 0; i < (VERSION_STRING_LEN - 1); i++)
            version[i] = rxMessage[i + 1];
          version[VERSION_STRING_LEN - 1] = 0;
          versionRcvd = true;
          // ==========================
//          Serial.write("WAV Version: ");
//          Serial.write(version);
//          Serial.write("\n");
          // ==========================
        break;

        case RSP_SYSTEM_INFO:
          numVoices = rxMessage[1];
          numTracks = rxMessage[3];
          numTracks = (numTracks * 256) + rxMessage[2];
          sysinfoRcvd = true;
          // ==========================
          ///\Serial.print("Sys info received\n");
          // ==========================
        break;

      }
      rxCount = 0;
      rxLen = 0;
      rxMsgReady = false;

    } // if (rxMsgReady)

  } // while (WTSerial.available() > 0)
  
}

// **************************************************************
bool wavTrigger::isTrackPlaying(int trk) {

int i;
bool fResult = false;

  update();
  for (i = 0; i < MAX_NUM_VOICES; i++) {
    if (voiceTable[i] == ((uint16_t)trk))
      fResult = true;
  }
  
  return fResult;
}


int wavTrigger::getPlayingTrack(int voiceNum) {
  if (voiceNum>=MAX_NUM_VOICES || voiceNum<0) return 0xFFFF;
  return (voiceTable[voiceNum]);
}



// **************************************************************
void wavTrigger::masterGain(int gain) {

uint8_t txbuf[7];
unsigned short vol;

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x07;
  txbuf[3] = CMD_MASTER_VOLUME;
  vol = (unsigned short)gain;
  txbuf[4] = (uint8_t)vol;
  txbuf[5] = (uint8_t)(vol >> 8);
  txbuf[6] = EOM;
  WTSerial.write(txbuf, 7);
}

// **************************************************************
void wavTrigger::setAmpPwr(bool enable) {

uint8_t txbuf[6];

    txbuf[0] = SOM1;
    txbuf[1] = SOM2;
    txbuf[2] = 0x06;
    txbuf[3] = CMD_AMP_POWER;
    txbuf[4] = enable;
    txbuf[5] = EOM;
    WTSerial.write(txbuf, 6);
}

// **************************************************************
void wavTrigger::setReporting(bool enable) {

uint8_t txbuf[6];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x06;
  txbuf[3] = CMD_SET_REPORTING;
  txbuf[4] = enable;
  txbuf[5] = EOM;
  WTSerial.write(txbuf, 6);
}

// **************************************************************
bool wavTrigger::getVersion(char *pDst, int len) {

int i;

  update();
  if (!versionRcvd) {
    return false;
  }
  for (i = 0; i < (VERSION_STRING_LEN - 1); i++) {
    if (i >= (len - 1))
      break;
    pDst[i] = version[i];
  }
  pDst[++i] = 0;
  return true;
}

// **************************************************************
int wavTrigger::getNumTracks(void) {

  update();
  return numTracks;
}

// **************************************************************
void wavTrigger::trackPlaySolo(int trk) {
  
  trackControl(trk, TRK_PLAY_SOLO);
}

// **************************************************************
void wavTrigger::trackPlaySolo(int trk, bool lock) {
  
  trackControl(trk, TRK_PLAY_SOLO, lock);
}

// **************************************************************
void wavTrigger::trackPlayPoly(int trk) {
  
  trackControl(trk, TRK_PLAY_POLY);
}

// **************************************************************
void wavTrigger::trackPlayPoly(int trk, bool lock) {
  
  trackControl(trk, TRK_PLAY_POLY, lock);
}

// **************************************************************
void wavTrigger::trackLoad(int trk) {
  
  trackControl(trk, TRK_LOAD);
}

// **************************************************************
void wavTrigger::trackLoad(int trk, bool lock) {
  
  trackControl(trk, TRK_LOAD, lock);
}

// **************************************************************
void wavTrigger::trackStop(int trk) {
  trackControl(trk, TRK_STOP);
}

// **************************************************************
void wavTrigger::trackPause(int trk) {

  trackControl(trk, TRK_PAUSE);
}

// **************************************************************
void wavTrigger::trackResume(int trk) {

  trackControl(trk, TRK_RESUME);
}

// **************************************************************
void wavTrigger::trackLoop(int trk, bool enable) {
 
  if (enable)
    trackControl(trk, TRK_LOOP_ON);
  else
    trackControl(trk, TRK_LOOP_OFF);
}

// **************************************************************
void wavTrigger::trackControl(int trk, int code) {
  
uint8_t txbuf[8];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x08;
  txbuf[3] = CMD_TRACK_CONTROL;
  txbuf[4] = (uint8_t)code;
  txbuf[5] = (uint8_t)trk;
  txbuf[6] = (uint8_t)(trk >> 8);
  txbuf[7] = EOM;
  WTSerial.write(txbuf, 8);
}

// **************************************************************
void wavTrigger::trackControl(int trk, int code, bool lock) {
  
uint8_t txbuf[9];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x09;
  txbuf[3] = CMD_TRACK_CONTROL_EX;
  txbuf[4] = (uint8_t)code;
  txbuf[5] = (uint8_t)trk;
  txbuf[6] = (uint8_t)(trk >> 8);
  txbuf[7] = lock;
  txbuf[8] = EOM;
  WTSerial.write(txbuf, 9);
}

// **************************************************************
void wavTrigger::stopAllTracks(void) {

uint8_t txbuf[5];
  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x05;
  txbuf[3] = CMD_STOP_ALL;
  txbuf[4] = EOM;
  WTSerial.write(txbuf, 5);
}

// **************************************************************
void wavTrigger::resumeAllInSync(void) {

uint8_t txbuf[5];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x05;
  txbuf[3] = CMD_RESUME_ALL_SYNC;
  txbuf[4] = EOM;
  WTSerial.write(txbuf, 5);
}

// **************************************************************
void wavTrigger::trackGain(int trk, int gain) {

uint8_t txbuf[9];
unsigned short vol;

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x09;
  txbuf[3] = CMD_TRACK_VOLUME;
  txbuf[4] = (uint8_t)trk;
  txbuf[5] = (uint8_t)(trk >> 8);
  vol = (unsigned short)gain;
  txbuf[6] = (uint8_t)vol;
  txbuf[7] = (uint8_t)(vol >> 8);
  txbuf[8] = EOM;
  WTSerial.write(txbuf, 9);
}

// **************************************************************
void wavTrigger::trackFade(int trk, int gain, int time, bool stopFlag) {

uint8_t txbuf[12];
unsigned short vol;

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x0c;
  txbuf[3] = CMD_TRACK_FADE;
  txbuf[4] = (uint8_t)trk;
  txbuf[5] = (uint8_t)(trk >> 8);
  vol = (unsigned short)gain;
  txbuf[6] = (uint8_t)vol;
  txbuf[7] = (uint8_t)(vol >> 8);
  txbuf[8] = (uint8_t)time;
  txbuf[9] = (uint8_t)(time >> 8);
  txbuf[10] = stopFlag;
  txbuf[11] = EOM;
  WTSerial.write(txbuf, 12);
}

// **************************************************************
void wavTrigger::samplerateOffset(int offset) {

uint8_t txbuf[7];
unsigned short off;

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x07;
  txbuf[3] = CMD_SAMPLERATE_OFFSET;
  off = (unsigned short)offset;
  txbuf[4] = (uint8_t)off;
  txbuf[5] = (uint8_t)(off >> 8);
  txbuf[6] = EOM;
  WTSerial.write(txbuf, 7);
}

// **************************************************************
void wavTrigger::setTriggerBank(int bank) {

uint8_t txbuf[6];

  txbuf[0] = SOM1;
  txbuf[1] = SOM2;
  txbuf[2] = 0x06;
  txbuf[3] = CMD_SET_TRIGGER_BANK;
  txbuf[4] = (uint8_t)bank;
  txbuf[5] = EOM;
  WTSerial.write(txbuf, 6);
}

#endif



AudioHandler::AudioHandler() {
  curSoundtrack = NULL;
  curSoundtrackEntries = 0;
  soundFXGain = 0;
  notificationsGain = 0;
  musicGain = 0;
  ClearSoundQueue();
  ClearSoundCardQueue();
  ClearNotificationStack();

  activeSequence.seqID = 0xFF;
  activeSequence.currentToneIndex = 0;
  activeSequence.priority = 0;
  activeSequence.startTime = 0;
  activeSequence.startOffset = 0;
  activeSequence.isProtected = false;
  activeSequence.pausedSeqID = 0xFF;
  activeSequence.pausedCurrentToneIndex = 0;
  activeSequence.pausedPriority = 0;
  activeSequence.pausedIsProtected = false;
  activeSequence.pausedStartTime = 0;
  activeSequence.pausedStartOffset = 0;
  currentBackgroundTrack = BACKGROUND_TRACK_NONE;
  musicStopped = true;
  soundtrackRandomOrder = true;
  nextSoundtrackPlayTime = 0;
  backgroundSongEndTime = 0;
  nextVoiceNotificationPlayTime = 0;
  
  voiceNotificationStackFirst = 0;
  voiceNotificationStackLast = 0;
  currentNotificationPriority = 0;
  currentNotificationPlaying = INVALID_SOUND_INDEX;
  musicDucking = 20;
  soundFXDucking = 20;

  for (int count=0; count<NUMBER_OF_SONGS_REMEMBERED; count++) lastSongsPlayed[count] = BACKGROUND_TRACK_NONE;

  InitSoundEffectQueue();
}

AudioHandler::~AudioHandler() {
}


boolean AudioHandler::InitDevices(byte audioType) {


#if defined(RPU_OS_USE_WAV_TRIGGER) || defined(RPU_OS_USE_WAV_TRIGGER_1p3)
  if (audioType & AUDIO_PLAY_TYPE_WAV_TRIGGER) {
    // WAV Trigger startup at 57600
    wTrig.start();
    delay(10);
    wTrig.stopAllTracks();
    wTrig.samplerateOffset(0);
    wTrig.setReporting(true);
  }
#endif

  if (audioType & AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS) {
#ifdef RPU_OS_USE_SB300
    InitSB300Registers();
    PlaySB300StartupBeep();
#endif

#if defined(RPU_OS_USE_WTYPE_1_SOUND) || defined(RPU_OS_USE_WTYPE_2_SOUND)
#endif
  }

  return true;
}



int AudioHandler::ConvertVolumeSettingToGain(byte volumeSetting) {
  if (volumeSetting==0) return -70;
  if (volumeSetting>10) return 0;
  return volumeToGainConversion[volumeSetting];
}


void AudioHandler::SetSoundFXVolume(byte s_volume) {
  soundFXGain = ConvertVolumeSettingToGain(s_volume);
}

void AudioHandler::SetNotificationsVolume(byte s_volume) {
  notificationsGain = ConvertVolumeSettingToGain(s_volume);
}

void AudioHandler::SetMusicVolume(byte s_volume) {
  musicGain = ConvertVolumeSettingToGain(s_volume);
}

void AudioHandler::SetMusicDuckingGain(byte s_ducking) {
  musicDucking = s_ducking;
}

void AudioHandler::SetSoundFXDuckingGain(byte s_ducking) {
  soundFXDucking = s_ducking;
}


boolean AudioHandler::StopSound(unsigned short soundIndex) {
#if defined(RPU_OS_USE_WAV_TRIGGER) || defined(RPU_OS_USE_WAV_TRIGGER_1p3)
  wTrig.trackStop(soundIndex);
#else
  (void)soundIndex;
#endif
  return false;
}

boolean AudioHandler::StopAllMusic() {
  curSoundtrack = NULL;
#if defined(RPU_OS_USE_WAV_TRIGGER) || defined(RPU_OS_USE_WAV_TRIGGER_1p3)
  if (currentBackgroundTrack!=BACKGROUND_TRACK_NONE) {
//    char buf[128];
//    sprintf(buf, "Stopping background music (%d)\n", currentBackgroundTrack);
//    Serial.write(buf);
    wTrig.trackStop(currentBackgroundTrack);
    currentBackgroundTrack = BACKGROUND_TRACK_NONE;
    musicStopped = true;
    return true;
  }
#endif
  currentBackgroundTrack = BACKGROUND_TRACK_NONE;
  return false;
}


void AudioHandler::ClearNotificationStack(byte priority) {
  if (priority==10) {
    voiceNotificationStackFirst = 0;
    voiceNotificationStackLast = 0;
    for (byte count=0; count<VOICE_NOTIFICATION_STACK_SIZE; count++) {
      voiceNotificationNumStack[count] = VOICE_NOTIFICATION_STACK_EMPTY;
      voiceNotificationDuration[count] = 0;
      voiceNotificationPriorityStack[count] = 0;

    }
  } else {
    byte tempFirst = voiceNotificationStackFirst;
    byte tempLast = voiceNotificationStackLast;
    while (tempFirst != tempLast) {
      if (voiceNotificationPriorityStack[tempFirst]<=priority) {
        voiceNotificationNumStack[tempFirst] = INVALID_SOUND_INDEX;
      }
      tempFirst += 1;
      if (tempFirst >= VOICE_NOTIFICATION_STACK_SIZE) tempFirst = 0;
    }    
  }
}


boolean AudioHandler::StopCurrentNotification(byte priority) {
  nextVoiceNotificationPlayTime = 0;

  if (currentNotificationPlaying!=INVALID_SOUND_INDEX && currentNotificationPriority<=priority) {
#if defined(RPU_OS_USE_WAV_TRIGGER) || defined(RPU_OS_USE_WAV_TRIGGER_1p3)
    wTrig.trackStop(currentNotificationPlaying);
#endif
//    currentNotificationPlaying = INVALID_SOUND_INDEX;
    nextVoiceNotificationPlayTime = 1;
    currentNotificationPriority = 0;
    return true;
  }
  return false;
}


int AudioHandler::SpaceLeftOnNotificationStack() {
  if (voiceNotificationStackFirst>=VOICE_NOTIFICATION_STACK_SIZE || voiceNotificationStackLast>=VOICE_NOTIFICATION_STACK_SIZE) return 0;
  if (voiceNotificationStackLast>=voiceNotificationStackFirst) return ((VOICE_NOTIFICATION_STACK_SIZE-1) - (voiceNotificationStackLast-voiceNotificationStackFirst));
  return (voiceNotificationStackFirst - voiceNotificationStackLast) - 1;
}


void AudioHandler::PushToNotificationStack(unsigned int notification, unsigned int duration, byte priority) {
  // If the switch stack last index is out of range, then it's an error - return
  if (SpaceLeftOnNotificationStack() == 0) return;

  voiceNotificationNumStack[voiceNotificationStackLast] = notification;
  voiceNotificationDuration[voiceNotificationStackLast] = duration;
  voiceNotificationPriorityStack[voiceNotificationStackLast] = priority;

  voiceNotificationStackLast += 1;
  if (voiceNotificationStackLast >= VOICE_NOTIFICATION_STACK_SIZE) {
    // If the end index is off the end, then wrap
    voiceNotificationStackLast = 0;
  }
}



byte AudioHandler::GetTopNotificationPriority() {
  byte startStack = voiceNotificationStackFirst;
  byte endStack = voiceNotificationStackLast;
  if (startStack==endStack) return 0;

  byte topPriorityFound = 0;

  while (startStack!=endStack) {
    if (voiceNotificationPriorityStack[startStack]>topPriorityFound) topPriorityFound = voiceNotificationPriorityStack[startStack];
    startStack += 1;
    if (startStack >= VOICE_NOTIFICATION_STACK_SIZE) startStack = 0;
  }

  return topPriorityFound;
}

void AudioHandler::DuckCurrentSoundEffects() {

  // We can't duck if we don't have bi-directional communication
  // So <=3 revs have to return
  if (RPU_OS_HARDWARE_REV<=3) return;
  
#if defined (RPU_OS_USE_WAV_TRIGGER) || defined (RPU_OS_USE_WAV_TRIGGER_1p3)
  for (int count=0; count<MAX_NUM_VOICES; count++) {
    int trackNum = wTrig.getPlayingTrack(count);
    if (trackNum!=((int)0xFFFF) && trackNum!=((int)currentBackgroundTrack) && trackNum!=((int)currentNotificationPlaying)) {
      // This is a sound effect that needs to be ducked
      wTrig.trackFade(trackNum, soundFXGain - soundFXDucking, 500, 0);
    }
  }
#endif  
  
}



boolean AudioHandler::QueuePrioritizedNotification(unsigned short notificationIndex, unsigned short notificationLength, byte priority, unsigned long currentTime) {
#if defined (RPU_OS_USE_WAV_TRIGGER) || defined (RPU_OS_USE_WAV_TRIGGER_1p3)
  // if everything on the queue has a lower priority, kill all those
  byte topQueuePriority = GetTopNotificationPriority();
  if (priority>topQueuePriority) {
    ClearNotificationStack();  
  }

  // If there's nothing playing, we can play it now
  if (currentNotificationPlaying == INVALID_SOUND_INDEX) {
    if (currentBackgroundTrack != BACKGROUND_TRACK_NONE) {
      wTrig.trackFade(currentBackgroundTrack, musicGain - musicDucking, 500, 0);
    }
    DuckCurrentSoundEffects();
    if (notificationLength) nextVoiceNotificationPlayTime = currentTime + (unsigned long)(notificationLength);
    else nextVoiceNotificationPlayTime = 0;

    wTrig.trackPlayPoly(notificationIndex);
    wTrig.trackGain(notificationIndex, notificationsGain);
    currentNotificationStartTime = currentTime;
    
    currentNotificationPlaying = notificationIndex;
    currentNotificationPriority = priority;
  } else {
    PushToNotificationStack(notificationIndex, notificationLength, priority);
  }
#else
  // Phony stuff to get rid of warnings
  (void)notificationIndex;
  (void)notificationLength;
  (void)priority;
  (void)currentTime;
#endif

  return true;
}

void AudioHandler::OutputTracksPlaying() {
#if defined (RPU_OS_USE_WAV_TRIGGER) || defined (RPU_OS_USE_WAV_TRIGGER_1p3)
  int i;
  char buf[256];
  sprintf(buf, "nothing");
  Serial.write("Looking for playing tracks\n");
  wTrig.getVersion(buf, 256);
  Serial.write("Version: ");
  Serial.write(buf);
  Serial.write("\n");
  for (i=0; i<1000; i++) {
  
    if (wTrig.isTrackPlaying(i)) {
      sprintf(buf, "Track %d playing\n", i);
      Serial.write(buf);
    }
  }
#endif
}


boolean AudioHandler::ServiceNotificationQueue(unsigned long currentTime) {
  boolean queueStillHasEntries = true;
#if defined (RPU_OS_USE_WAV_TRIGGER) || defined (RPU_OS_USE_WAV_TRIGGER_1p3)
  boolean playNextNotification = false;

  if (nextVoiceNotificationPlayTime != 0) { 
    if (currentTime > nextVoiceNotificationPlayTime) {
      playNextNotification = true;
    }
  } else {
    if (currentNotificationPlaying!=INVALID_SOUND_INDEX && !wTrig.isTrackPlaying(currentNotificationPlaying)) { 
      if (currentTime>(currentNotificationStartTime+100)) playNextNotification = true;
    }
  }
  
  if (playNextNotification) {
    byte nextPriority = 0;
    unsigned int nextNotification = VOICE_NOTIFICATION_STACK_EMPTY;
    unsigned int nextDuration = 0;

    // Current notification done, see if there's another
    if (voiceNotificationStackLast>=VOICE_NOTIFICATION_STACK_SIZE) voiceNotificationStackLast = (VOICE_NOTIFICATION_STACK_SIZE-1);
    while (voiceNotificationStackFirst != voiceNotificationStackLast) {
      nextPriority = voiceNotificationPriorityStack[voiceNotificationStackFirst];
      nextNotification = voiceNotificationNumStack[voiceNotificationStackFirst];
      nextDuration = voiceNotificationDuration[voiceNotificationStackFirst];

      voiceNotificationStackFirst += 1;
      if (voiceNotificationStackFirst >= VOICE_NOTIFICATION_STACK_SIZE) voiceNotificationStackFirst = 0;
      if (nextNotification!=INVALID_SOUND_INDEX) break;
    }

    if (nextNotification != VOICE_NOTIFICATION_STACK_EMPTY) {
      if (currentBackgroundTrack != BACKGROUND_TRACK_NONE) {
        wTrig.trackFade(currentBackgroundTrack, musicGain - musicDucking, 500, 0);
      }
      DuckCurrentSoundEffects();
      if (nextDuration!=0) nextVoiceNotificationPlayTime = currentTime + (unsigned long)(nextDuration);
      else nextVoiceNotificationPlayTime = 0;
      wTrig.trackPlayPoly(nextNotification);
      wTrig.trackGain(nextNotification, notificationsGain);
      currentNotificationStartTime = currentTime;
      currentNotificationPlaying = nextNotification;
      currentNotificationPriority = nextPriority;
    } else {
      // No more notifications -- set the volume back up and clear the variable
      if (currentBackgroundTrack != BACKGROUND_TRACK_NONE) {
        wTrig.trackFade(currentBackgroundTrack, musicGain, 1500, 0);
      }
      nextVoiceNotificationPlayTime = 0;
      currentNotificationPlaying = INVALID_SOUND_INDEX;
      currentNotificationPriority = 0;
      queueStillHasEntries = false;
    }
    
  } else {
    queueStillHasEntries = false;
  }
#else
  (void)currentTime;
#endif

  return queueStillHasEntries;
}



boolean AudioHandler::StopAllNotifications(byte priority) {
  ClearNotificationStack(priority);
  return StopCurrentNotification(priority);
}

boolean AudioHandler::StopAllSoundFX() {
#if defined(RPU_OS_USE_WAV_TRIGGER) || defined(RPU_OS_USE_WAV_TRIGGER_1p3)
  wTrig.stopAllTracks();
#endif
  ClearSoundCardQueue();
  ClearSoundQueue();
  return false;
}


boolean AudioHandler::StopAllAudio() {
  boolean anythingPlaying = false;
  if (StopAllMusic()) anythingPlaying = true;
  if (StopAllNotifications()) anythingPlaying = true;
  if (StopAllSoundFX()) anythingPlaying = true;
  return anythingPlaying;  
}

void AudioHandler::InitSB300Registers() {
#ifdef RPU_OS_USE_SB300
  RPU_PlaySB300SquareWave(1, 0x00); // Write 0x00 to CR2 (Timer 2 off, continuous mode, 16-bit, C2 clock, CR3 set)
  RPU_PlaySB300SquareWave(0, 0x00); // Write 0x00 to CR3 (Timer 3 off, continuous mode, 16-bit, C3 clock, not prescaled)
  RPU_PlaySB300SquareWave(1, 0x01); // Write 0x00 to CR2 (Timer 2 off, continuous mode, 16-bit, C2 clock, CR1 set)
  RPU_PlaySB300SquareWave(0, 0x00); // Write 0x00 to CR1 (Timer 1 off, continuous mode, 16-bit, C1 clock, timers allowed)
#endif
}


void AudioHandler::PlaySB300StartupBeep() {
#ifdef RPU_OS_USE_SB300
  RPU_PlaySB300SquareWave(1, 0x92); // Write 0x92 to CR2 (Timer 2 on, continuous mode, 16-bit, E clock, CR3 set)
  RPU_PlaySB300SquareWave(0, 0x92); // Write 0x92 to CR3 (Timer 3 on, continuous mode, 16-bit, E clock, not prescaled)
  RPU_PlaySB300SquareWave(4, 0x02); // Set Timer 2 to 0x0200
  RPU_PlaySB300SquareWave(5, 0x00); 
  RPU_PlaySB300SquareWave(6, 0x80); // Set Timer 3 to 0x8000
  RPU_PlaySB300SquareWave(7, 0x00);
  RPU_PlaySB300Analog(0, 0x02);
#endif
}


void AudioHandler::ClearSoundCardQueue() {
#ifdef RPU_OS_USE_SB300
  for (int count=0; count<SOUND_CARD_QUEUE_SIZE; count++) {
    soundCardQueue[count].playTime = 0;
  }
#endif
}


void AudioHandler::ClearSoundQueue() {
  for (int count=0; count<SOUND_QUEUE_SIZE; count++) {
    soundQueue[count].playTime = 0;
    soundQueue[count].priority = 0;
  }
  activeSequence.seqID = 0xFF;
  activeSequence.currentToneIndex = 0;
  activeSequence.priority = 0;
  activeSequence.startTime = 0;
  activeSequence.startOffset = 0;
  activeSequence.isProtected = false;
  activeSequence.pausedSeqID = 0xFF;
  activeSequence.pausedCurrentToneIndex = 0;
  activeSequence.pausedPriority = 0;
  activeSequence.pausedIsProtected = false;
  activeSequence.pausedStartTime = 0;
  activeSequence.pausedStartOffset = 0;
}


boolean AudioHandler::PlaySound(unsigned short soundIndex, byte audioType, byte overrideVolume) {

  boolean soundPlayed = false;
  int gain = soundFXGain;
  if (currentNotificationPlaying!=INVALID_SOUND_INDEX) {
    // reduce gain (by ducking amount) if there's a notification playing
    gain -= soundFXDucking;
  }
  if (overrideVolume!=0xFF) gain = ConvertVolumeSettingToGain(overrideVolume);

  if (audioType==AUDIO_PLAY_TYPE_CHIMES) {
#if defined(RPU_OS_USE_SB100)
//    RPU_PlaySB100Chime((byte)soundIndex);
    soundPlayed = true;
#endif
  } else if (audioType==AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS) {
#ifdef RPU_OS_USE_DASH51
    RPU_PlaySoundDash51((byte)soundIndex);
    soundPlayed = true;
#endif
#ifdef RPU_OS_USE_S_AND_T
    RPU_PlaySoundSAndT((byte)soundIndex);
    soundPlayed = true;
#endif
#ifdef RPU_OS_USE_SB100
    RPU_PlaySB100((byte)soundIndex);
    soundPlayed = true;
#endif
  } else if (audioType==AUDIO_PLAY_TYPE_WAV_TRIGGER) {
#if defined(RPU_OS_USE_WAV_TRIGGER) || defined(RPU_OS_USE_WAV_TRIGGER_1p3)
#ifdef RPU_OS_USE_WAV_TRIGGER
    wTrig.trackStop(soundIndex);
#endif

    wTrig.trackPlayPoly(soundIndex);
    wTrig.trackGain(soundIndex, gain);
    soundPlayed = true;
#endif
  }
  (void)gain;
  (void)soundIndex;

  return soundPlayed;  
}


boolean AudioHandler::FadeSound(unsigned short soundIndex, int fadeGain, int numMilliseconds, boolean stopTrack) {
  boolean soundFaded = false;
#if defined(RPU_OS_USE_WAV_TRIGGER) || defined(RPU_OS_USE_WAV_TRIGGER_1p3)
  wTrig.trackFade(soundIndex, fadeGain, numMilliseconds, stopTrack);
  soundFaded = true;
#endif
  (void)soundIndex;
  (void)fadeGain;
  (void)numMilliseconds;
  (void)stopTrack;
  return soundFaded;
}



int AudioHandler::QueueSound(unsigned short soundIndex, byte audioType, unsigned long timeToPlay, byte overrideVolume, byte priority) {
  for (int count=0; count<SOUND_QUEUE_SIZE; count++) {
    if (soundQueue[count].playTime==0) {
      soundQueue[count].soundIndex = soundIndex;
      soundQueue[count].audioType = audioType;
      soundQueue[count].playTime = timeToPlay;
      soundQueue[count].overrideVolume = overrideVolume;
      soundQueue[count].priority = priority;
      soundQueue[count].seqID = 0xFF;  // Mark as not part of sequence yet


      return count;  // Return the index
    }
  }

  return -1;  // Failed
}


boolean AudioHandler::QueueSequence(byte seqID, unsigned long startOffset) {
  // Bounds check
  if (seqID >= NUM_SOUND_SEQUENCES) {
    return false;
  }

  // Get sequence pointer from PROGMEM
  const SoundStep* seqPtr = (const SoundStep*)pgm_read_ptr(&SoundSequenceTable[seqID]);
  if (!seqPtr) {
    return false;
  }

  // Diagnostic logging for pop-bumper sequences
  if (seqID == 20) {  // SEQ_POP_BUMPER
    char buf[96];
    sprintf(buf, "POP_BUMPER: SEQ QUEUE START seqID=20 @ CurrentTime=%lu startOffset=%lu\n",
            CurrentTime, startOffset);
    Serial.write(buf);
  }

  // Read first step
  SoundStep firstStep;
  memcpy_P(&firstStep, &seqPtr[0], sizeof(SoundStep));
  if (firstStep.tone == 0xFF) {
    return false;  // Empty sequence
  }

  // Allow multiple sequences to be queued as long as they don't overlap in time
  // Find the latest playTime of any existing sequence to detect actual collisions
  // IMPORTANT: Skip paused sequences - they shouldn't block new sequences from queueing.
  // Paused sequences are waiting to resume; they must not prevent active gameplay.
  unsigned long maxExistingPlayTime = 0;
  byte blockingSeqID = 0xFF;
  for (int i = 0; i < SOUND_QUEUE_SIZE; i++) {
    if (soundQueue[i].playTime > 0 && soundQueue[i].playTime > CurrentTime && soundQueue[i].seqID != 0xFF) {
      // Skip tones from paused sequences - they shouldn't block new sequences
      if (activeSequence.pausedSeqID != 0xFF && soundQueue[i].seqID == activeSequence.pausedSeqID) {
        continue;
      }
      // Track the latest time any existing sequence event is scheduled
      if (soundQueue[i].playTime > maxExistingPlayTime) {
        maxExistingPlayTime = soundQueue[i].playTime;
        blockingSeqID = soundQueue[i].seqID;
      }
    }
  }

  // Note: activeSequence checking was removed because it was too strict and blocked
  // legitimate sequence chaining. Overlap detection using soundQueue alone is sufficient.
  // (See audio system audit doc for full analysis)

  // Calculate when this new sequence will actually start playing
  unsigned long newSeqStartTime = CurrentTime + startOffset + firstStep.gap_ms;

  // Handle overlap: if blocking sequence is paused, clear it to prevent orphaned tones from blocking forever
  if (maxExistingPlayTime > 0 && newSeqStartTime <= maxExistingPlayTime) {
    // Pop-bumpers are unblockable - clear whatever is in the way instead of rejecting
    if (seqID == 20) {  // SEQ_POP_BUMPER
      // Clear all tones from the blocking sequence to make room for this pop-bumper
      int clearedCount = 0;
      for (int i = 0; i < SOUND_QUEUE_SIZE; i++) {
        if (soundQueue[i].seqID == blockingSeqID) {
          soundQueue[i].playTime = 0;
          soundQueue[i].seqID = 0xFF;
          clearedCount++;
        }
      }
      if (clearedCount > 0) {
        char buf[96];
        sprintf(buf, "POP_BUMPER: CLEARED BLOCKING seqID=%d (cleared %d tones) @ CurrentTime=%lu\n",
                blockingSeqID, clearedCount, CurrentTime);
        Serial.write(buf);
      }
      // Pop-bumper proceeds (not rejected)
    } else if (blockingSeqID == activeSequence.pausedSeqID) {
      // For other sequences: if blocking sequence is paused, clear it
      for (int i = 0; i < SOUND_QUEUE_SIZE; i++) {
        if (soundQueue[i].seqID == blockingSeqID) {
          soundQueue[i].playTime = 0;
          soundQueue[i].seqID = 0xFF;
        }
      }
      // Don't reject - allow the new sequence to queue
    } else {
      // Active sequence is blocking a non-pop-bumper - reject
      return false;
    }
  }

  // All sequences can interrupt each other (paused/resumed via interrupt system)
  // Spinner is gated separately via protectedSoundUntilTime check in handler

  // Pop-bumpers don't support pause/resume, so clear all old pop-bumper tones when queuing a new one.
  // EXCEPTION: preserve any silence tone (0x00) that's about to play within 50ms.
  // This prevents old pop-bumper tones from layering with new ones while preserving the
  // current sequence's silence tone that stops the hardware from holding the laser tone.
  if (seqID == 20) {  // SEQ_POP_BUMPER
    for (int i = 0; i < SOUND_QUEUE_SIZE; i++) {
      if (soundQueue[i].seqID == 20 && soundQueue[i].playTime > 0) {
        // Preserve silence tones (0x00) that are about to play very soon (next 50ms)
        // These are part of the current sequence and needed to stop the hardware tone
        boolean isSilenceSoonToPlay = (soundQueue[i].soundIndex == 0x00 &&
                                       soundQueue[i].playTime >= CurrentTime &&
                                       soundQueue[i].playTime <= (CurrentTime + 50));
        if (!isSilenceSoonToPlay) {
          soundQueue[i].playTime = 0;
          soundQueue[i].seqID = 0xFF;
        }
      }
    }
  }

  // Start this sequence as active
  unsigned long playTime = CurrentTime + startOffset + firstStep.gap_ms;
  int toneIndex = QueueSound(firstStep.tone, AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS, playTime, 0xFF, 50);
  if (toneIndex < 0) {
    return false;
  }
  // Mark this sound as part of this sequence immediately
  soundQueue[toneIndex].seqID = seqID;

  // Auto-insert silence after this tone (Dick's timing: 75ms)
  unsigned int silenceDuration = 75;
  if (seqID == 27) {  // SEQ_DRAIN special case
    // Check if this is the last tone
    SoundStep nextStep;
    memcpy_P(&nextStep, &seqPtr[1], sizeof(SoundStep));
    if (nextStep.tone == 0xFF) {
      silenceDuration = 400;
    }
  }
  unsigned long silenceTime = playTime + silenceDuration;
  int silenceIndex = QueueSound(0, AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS, silenceTime, 0xFF, 50);
  // Mark silence as part of this sequence too
  if (silenceIndex >= 0) {
    soundQueue[silenceIndex].seqID = seqID;
  }

  // If there's an active sequence, save it as paused before we overwrite it
  // EXCEPTION: Pop-bumpers are not saved as paused. They're short (80ms) and don't need
  // resumption. Preventing pop-bumper pause/resume eliminates accumulation of paused
  // pop-bumper tones in the queue that cause phantom sounds and blocking.
  if (activeSequence.seqID != 0xFF && activeSequence.seqID != 20) {  // seqID 20 = SEQ_POP_BUMPER
    activeSequence.pausedSeqID = activeSequence.seqID;
    activeSequence.pausedCurrentToneIndex = activeSequence.currentToneIndex;
    activeSequence.pausedPriority = activeSequence.priority;
    activeSequence.pausedIsProtected = activeSequence.isProtected;
    activeSequence.pausedStartTime = activeSequence.startTime;
    activeSequence.pausedStartOffset = activeSequence.startOffset;
  }

  // For protected sequences, queue all remaining tones immediately to prevent interruption
  // All sequences are protected; spinner has its own protectedSoundUntilTime gate to prevent unwanted queueing
  boolean isProtectedSeq = true;
  byte currentToneIdx = 1;

  if (isProtectedSeq) {
    // Loop through and queue all remaining tones
    SoundStep step;
    while (true) {
      memcpy_P(&step, &seqPtr[currentToneIdx], sizeof(SoundStep));
      if (step.tone == 0xFF) break;  // End of sequence

      // Queue this tone
      unsigned long tonePlayTime = CurrentTime + startOffset + step.gap_ms;
      int idx = QueueSound(step.tone, AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS, tonePlayTime, 0xFF, 50);
      if (idx >= 0) {
        soundQueue[idx].seqID = seqID;
        if (seqID == 20) {  // SEQ_POP_BUMPER
          char buf[96];
          sprintf(buf, "POP_BUMPER: TONE QUEUED idx=%d tone=0x%02X playTime=%lu\n",
                  idx, step.tone, tonePlayTime);
          Serial.write(buf);
        }
      }

      // Queue silence after this tone
      silenceDuration = 75;
      if (seqID == 27) {  // SEQ_DRAIN: check if next is sentinel
        SoundStep nextStep;
        memcpy_P(&nextStep, &seqPtr[currentToneIdx + 1], sizeof(SoundStep));
        if (nextStep.tone == 0xFF) {
          silenceDuration = 400;
        }
      }
      unsigned long toneSilenceTime = tonePlayTime + silenceDuration;
      int silIdx = QueueSound(0, AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS, toneSilenceTime, 0xFF, 50);
      if (silIdx >= 0) {
        soundQueue[silIdx].seqID = seqID;
      }

      currentToneIdx++;
    }
  }

  activeSequence.seqID = seqID;
  activeSequence.currentToneIndex = currentToneIdx;  // Point to sentinel (or next tone if unprotected)
  activeSequence.priority = 50;  // Default priority for all sequences now
  activeSequence.isProtected = isProtectedSeq;
  activeSequence.startTime = CurrentTime;
  activeSequence.startOffset = startOffset;

  return true;
}


void AudioHandler::ResumeActiveSequence(unsigned long currentTime) {
  // If no active sequence, nothing to do
  if (activeSequence.seqID == 0xFF) {
    return;
  }

  // Get sequence pointer
  const SoundStep* seqPtr = (const SoundStep*)pgm_read_ptr(&SoundSequenceTable[activeSequence.seqID]);
  if (!seqPtr) {
    activeSequence.seqID = 0xFF;
    return;
  }

  // Read the next step
  SoundStep step;
  memcpy_P(&step, &seqPtr[activeSequence.currentToneIndex], sizeof(SoundStep));

  // Check for end of sequence (sentinel)
  if (step.tone == 0xFF) {
    // Sequence is complete. Check if there's a paused sequence to restore
    if (activeSequence.pausedSeqID != 0xFF) {
      // Before restoring, check if paused sequence has also finished
      const SoundStep* pausedSeqPtr = (const SoundStep*)pgm_read_ptr(&SoundSequenceTable[activeSequence.pausedSeqID]);
      if (pausedSeqPtr) {
        SoundStep pausedStep;
        memcpy_P(&pausedStep, &pausedSeqPtr[activeSequence.pausedCurrentToneIndex], sizeof(SoundStep));
        // If paused sequence is also at sentinel, it's already done — don't resume it
        if (pausedStep.tone != 0xFF) {
          // Restore paused sequence
          activeSequence.seqID = activeSequence.pausedSeqID;
          activeSequence.currentToneIndex = activeSequence.pausedCurrentToneIndex;
          activeSequence.priority = activeSequence.pausedPriority;
          activeSequence.isProtected = activeSequence.pausedIsProtected;
          activeSequence.startTime = activeSequence.pausedStartTime;
          activeSequence.startOffset = activeSequence.pausedStartOffset;
          // Clear paused fields
          activeSequence.pausedSeqID = 0xFF;
          // Recursively resume the restored sequence (queue its next tone)
          ResumeActiveSequence(currentTime);
          return;
        }
      }
      // Paused sequence is done or invalid, just clear it
      activeSequence.pausedSeqID = 0xFF;
    }
    // Clear active sequence
    activeSequence.seqID = 0xFF;
    activeSequence.isProtected = false;
    return;
  }

  // Queue the next tone
  // When resuming, respect original timing but don't queue in the past
  unsigned long originalPlayTime = activeSequence.startTime + activeSequence.startOffset + step.gap_ms;
  unsigned long playTime = max(originalPlayTime, currentTime);
  int toneIndex = QueueSound(step.tone, AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS, playTime, 0xFF, 50);
  if (toneIndex >= 0) {
    // Mark this sound as part of this sequence immediately
    soundQueue[toneIndex].seqID = activeSequence.seqID;

    // Auto-insert silence after this tone (Dick's timing: 75ms)
    unsigned int silenceDuration = 75;
    if (activeSequence.seqID == 27) {  // SEQ_DRAIN special case
      // Check if this is the last tone
      SoundStep nextStep;
      memcpy_P(&nextStep, &seqPtr[activeSequence.currentToneIndex + 1], sizeof(SoundStep));
      if (nextStep.tone == 0xFF) {
        silenceDuration = 400;
      }
    }
    unsigned long silenceTime = playTime + silenceDuration;
    int silenceIndex = QueueSound(0, AUDIO_PLAY_TYPE_ORIGINAL_SOUNDS, silenceTime, 0xFF, 50);
    // Mark silence as part of this sequence too
    if (silenceIndex >= 0) {
      soundQueue[silenceIndex].seqID = activeSequence.seqID;
    }

    activeSequence.currentToneIndex++;
  }
}


boolean AudioHandler::QueueSoundCardCommand(byte scFunction, byte scRegister, byte scData, unsigned long startTime) {
#ifdef RPU_OS_USE_SB300
  for (int count=0; count<SOUND_QUEUE_SIZE; count++) {
    if (soundCardQueue[count].playTime==0) {
      soundCardQueue[count].soundFunction = scFunction;
      soundCardQueue[count].soundRegister = scRegister;
      soundCardQueue[count].soundByte = scData;
      soundCardQueue[count].playTime = startTime;
      return true;
    }
  }
#else 
  // Phony stuff to get rid of warnings
  unsigned long totalval = scFunction + scRegister + scData + startTime;
  totalval += 1;
#endif
  return false;
}


void AudioHandler::InitSoundEffectQueue() {
#if defined(RPU_WTYPE_1_SOUND)
  CurrentSoundPlaying.soundEffectNum = 0;
  CurrentSoundPlaying.requestedPlayTime = 0;
  CurrentSoundPlaying.playUntil = 0;
  CurrentSoundPlaying.priority = 0;
  CurrentSoundPlaying.inUse = false;

  for (byte count = 0; count < SOUND_EFFECT_QUEUE_SIZE; count++) {
    SoundEffectQueue[count].soundEffectNum = 0;
    SoundEffectQueue[count].requestedPlayTime = 0;
    SoundEffectQueue[count].playUntil = 0;
    SoundEffectQueue[count].priority = 0;
    SoundEffectQueue[count].inUse = false;
  }
#endif
}

boolean AudioHandler::PlaySoundCardWhenPossible(unsigned short soundEffectNum, unsigned long currentTime, unsigned long requestedPlayTime, unsigned long playUntil, byte priority) {

#if defined(RPU_OS_USE_WTYPE_1_SOUND) || defined(RPU_OS_USE_WTYPE_2_SOUND)
  byte count = 0;
  for (count = 0; count < SOUND_EFFECT_QUEUE_SIZE; count++) {
    if (SoundEffectQueue[count].inUse == false) break;
  }
  if (count == SOUND_EFFECT_QUEUE_SIZE) return false;
  SoundEffectQueue[count].soundEffectNum = soundEffectNum;
  SoundEffectQueue[count].requestedPlayTime = requestedPlayTime + currentTime;
  SoundEffectQueue[count].playUntil = playUntil + requestedPlayTime + currentTime;
  SoundEffectQueue[count].priority = priority;
  SoundEffectQueue[count].inUse = true;
#else 
  // Phony stuff to get rid of warnings
  (void)soundEffectNum;
  (void)currentTime;
  (void)requestedPlayTime;
  (void)playUntil;
  (void)priority;
  return false;
#endif
  return true;
}




boolean AudioHandler::ServiceSoundQueue(unsigned long currentTime) {
  boolean soundCommandSent = false;

  // Check for orphaned/layered pop-bumper tones
  int popBumperCount = 0;
  for (int count=0; count<SOUND_QUEUE_SIZE; count++) {
    if (soundQueue[count].playTime!=0 && soundQueue[count].playTime > currentTime && soundQueue[count].seqID == 20) {
      popBumperCount++;
    }
  }
  if (popBumperCount > 1) {
    char buf[96];
    sprintf(buf, "POP_BUMPER: WARNING - %d tones in queue (potential phantom layering) @ CurrentTime=%lu\n",
            popBumperCount, currentTime);
    Serial.write(buf);
  }

  // Find the earliest ready sound to play
  int earliestIndex = -1;
  unsigned long earliestTime = currentTime + 1000000;
  for (int count=0; count<SOUND_QUEUE_SIZE; count++) {
    if (soundQueue[count].playTime!=0 && soundQueue[count].playTime <= currentTime) {
      if (soundQueue[count].playTime < earliestTime) {
        earliestTime = soundQueue[count].playTime;
        earliestIndex = count;
      }
    }
  }

  // Process only the earliest sound
  if (earliestIndex >= 0) {
    // Diagnostic logging for pop-bumper tones
    if (soundQueue[earliestIndex].seqID == 20) {  // SEQ_POP_BUMPER
      char buf[96];
      sprintf(buf, "POP_BUMPER: TONE PLAYED idx=%d tone=0x%02X @ CurrentTime=%lu\n",
              earliestIndex, soundQueue[earliestIndex].soundIndex, currentTime);
      Serial.write(buf);
    }

    PlaySound(soundQueue[earliestIndex].soundIndex, soundQueue[earliestIndex].audioType, soundQueue[earliestIndex].overrideVolume);
    soundQueue[earliestIndex].playTime = 0;
    soundQueue[earliestIndex].priority = 0;
    soundCommandSent = true;

    // After playing a sound, check if we should resume the active sequence
    ResumeActiveSequence(currentTime);
  }

  return soundCommandSent;
}

boolean AudioHandler::ServiceSoundCardQueue(unsigned long currentTime) {
#ifdef RPU_OS_USE_SB300
  boolean soundCommandSent = false;
  for (int count=0; count<SOUND_CARD_QUEUE_SIZE; count++) {
    if (soundCardQueue[count].playTime!=0 && soundCardQueue[count].playTime<currentTime) {
      if (soundCardQueue[count].soundFunction==SB300_SOUND_FUNCTION_SQUARE_WAVE) {
        RPU_PlaySB300SquareWave(soundCardQueue[count].soundRegister, soundCardQueue[count].soundByte);   
      } else if (soundCardQueue[count].soundFunction==SB300_SOUND_FUNCTION_ANALOG) {
        RPU_PlaySB300Analog(soundCardQueue[count].soundRegister, soundCardQueue[count].soundByte);   
      }
      soundCardQueue[count].playTime = 0;
      soundCommandSent = true;
    }
  }

  return soundCommandSent;
#elif defined(RPU_OS_USE_WTYPE_1_SOUND) || defined(RPU_OS_USE_WTYPE_2_SOUND) 
  byte highestPrioritySound = 0xFF;
  byte queuePriority = 0;

  for (byte count = 0; count < SOUND_EFFECT_QUEUE_SIZE; count++) {
    // Skip sounds that aren't in use
    if (SoundEffectQueue[count].inUse == false) continue;

    // If a sound has expired, flush it
    if (currentTime > SoundEffectQueue[count].playUntil) {
      SoundEffectQueue[count].inUse = false;
    } else if (currentTime > SoundEffectQueue[count].requestedPlayTime) {
      // If this sound is ready to be played, figure out its priority
      if (SoundEffectQueue[count].priority > queuePriority) {
        queuePriority = SoundEffectQueue[count].priority;
        highestPrioritySound = count;
      } else if (SoundEffectQueue[count].priority == queuePriority) {
        if (highestPrioritySound != 0xFF) {
          if (SoundEffectQueue[highestPrioritySound].requestedPlayTime > SoundEffectQueue[count].requestedPlayTime) {
            // The priorities are equal, but this sound was requested before, so switch to it
            highestPrioritySound = count;
          }
        }
      }
    }
  }

  if (CurrentSoundPlaying.inUse && (currentTime > CurrentSoundPlaying.playUntil)) {
    CurrentSoundPlaying.inUse = false;
  }

  boolean soundCommandSent = false;
  if (highestPrioritySound != 0xFF) {
    if (CurrentSoundPlaying.inUse == false || (CurrentSoundPlaying.inUse && CurrentSoundPlaying.priority < queuePriority)) {
      // Play new sound
      CurrentSoundPlaying.soundEffectNum = SoundEffectQueue[highestPrioritySound].soundEffectNum;
      CurrentSoundPlaying.requestedPlayTime = SoundEffectQueue[highestPrioritySound].requestedPlayTime;
      CurrentSoundPlaying.playUntil = SoundEffectQueue[highestPrioritySound].playUntil;
      CurrentSoundPlaying.priority = SoundEffectQueue[highestPrioritySound].priority;
      CurrentSoundPlaying.inUse = true;
      SoundEffectQueue[highestPrioritySound].inUse = false;
      RPU_PushToSoundStack(CurrentSoundPlaying.soundEffectNum, 8);
      soundCommandSent = true;
    }
  }
  return soundCommandSent;

#else 
  // Phony stuff to get rid of warnings
  (void)currentTime;
  return false;
#endif
}


boolean AudioHandler::PlayBackgroundSoundtrack(AudioSoundtrack *soundtrackArray, unsigned short numSoundtrackEntries, unsigned long currentTime, boolean randomOrder) {
  StopAllMusic();
  if (soundtrackArray==NULL) return false;

  curSoundtrack = soundtrackArray;
  curSoundtrackEntries = numSoundtrackEntries; 
  soundtrackRandomOrder = randomOrder;
  if (currentTime!=0) backgroundSongEndTime = currentTime-1;
  else backgroundSongEndTime = 0;
  
  return true;
}

boolean AudioHandler::PlayBackgroundSong(unsigned short trackIndex, boolean loopTrack) {
  StopAllMusic();
  boolean trackPlayed = false;

  if (trackIndex!=BACKGROUND_TRACK_NONE) {
    musicStopped = false;
    currentBackgroundTrack = trackIndex;
#if defined(RPU_OS_USE_WAV_TRIGGER) || defined(RPU_OS_USE_WAV_TRIGGER_1p3)
#ifdef RPU_OS_USE_WAV_TRIGGER_1p3
    wTrig.trackPlayPoly(trackIndex, true);
    trackPlayed = true;
//    Serial.write("Playing background song\n");
#else
    wTrig.trackPlayPoly(trackIndex);
    trackPlayed = true;
#endif
    if (loopTrack) wTrig.trackLoop(trackIndex, true);
    wTrig.trackGain(trackIndex, musicGain);
#endif
  }
  (void)loopTrack;
  
  return trackPlayed;  
}


void AudioHandler::StartNextSoundtrackSong(unsigned long currentTime) {

  if (curSoundtrack==NULL) return;
  
  unsigned int retSong = (currentTime%curSoundtrackEntries);
  boolean songRecentlyPlayed = false;

  unsigned int songCount = 0;
  for (songCount=0; songCount<curSoundtrackEntries; songCount++) {
    for (byte count=0; count<NUMBER_OF_SONGS_REMEMBERED; count++) {
      if (lastSongsPlayed[count]==curSoundtrack[retSong].TrackIndex) {
        songRecentlyPlayed = true;        
        break;
      }
    }
    if (!songRecentlyPlayed) break;
    retSong = (retSong+1);
    songRecentlyPlayed = false;
    if (retSong>=curSoundtrackEntries) retSong = 0;
  }

  // Record this song in the array
  for (byte count=(NUMBER_OF_SONGS_REMEMBERED-1); count>0; count--) lastSongsPlayed[count] = lastSongsPlayed[count-1];
  lastSongsPlayed[0] = curSoundtrack[retSong].TrackIndex;

  backgroundSongEndTime = (((unsigned long)curSoundtrack[retSong].TrackLength) * 1000) + currentTime;
  
  if (currentBackgroundTrack!=BACKGROUND_TRACK_NONE) {
#if defined(RPU_OS_USE_WAV_TRIGGER) || defined(RPU_OS_USE_WAV_TRIGGER_1p3)
    wTrig.trackFade(currentBackgroundTrack, -80, 2000, 1);
#endif
  }
  currentBackgroundTrack = curSoundtrack[retSong].TrackIndex;
  musicStopped = false;

#if defined(RPU_OS_USE_WAV_TRIGGER) || defined(RPU_OS_USE_WAV_TRIGGER_1p3)
#ifdef RPU_OS_USE_WAV_TRIGGER_1p3
  wTrig.trackPlayPoly(currentBackgroundTrack, true);
#else
  wTrig.trackPlayPoly(currentBackgroundTrack);
#endif
  wTrig.trackGain(currentBackgroundTrack, musicGain);
#endif

}



void AudioHandler::ManageBackgroundSong(unsigned long currentTime) {
  if (curSoundtrack==NULL /* && currentBackgroundTrack!=BACKGROUND_TRACK_NONE */) {
#if defined(RPU_OS_USE_WAV_TRIGGER) || defined(RPU_OS_USE_WAV_TRIGGER_1p3)
      if (currentBackgroundTrack!=BACKGROUND_TRACK_NONE) {
        if (wTrig.isTrackPlaying(currentBackgroundTrack)) {
          musicStopped = false;
        } else {
          musicStopped = true;
        }
      }
#endif
  } else {

    if (backgroundSongEndTime!=0) {
      if (currentTime>=backgroundSongEndTime) {
        StartNextSoundtrackSong(currentTime);
      }
    } else {
#if defined(RPU_OS_USE_WAV_TRIGGER) || defined(RPU_OS_USE_WAV_TRIGGER_1p3)
      if (!wTrig.isTrackPlaying(currentBackgroundTrack)) {
        StartNextSoundtrackSong(currentTime);
      }
#endif
    }
  }
}


boolean AudioHandler::Update(unsigned long currentTime) {
#if defined(RPU_OS_USE_WAV_TRIGGER) || defined(RPU_OS_USE_WAV_TRIGGER_1p3)
  wTrig.update();
#endif
  boolean queueHasEntries = false;
  ManageBackgroundSong(currentTime);
  ServiceSoundQueue(currentTime);
  ServiceSoundCardQueue(currentTime);
  if (ServiceNotificationQueue(currentTime)) queueHasEntries = true;
  return queueHasEntries;
}

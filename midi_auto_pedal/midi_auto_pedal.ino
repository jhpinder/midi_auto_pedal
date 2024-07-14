#include <MIDI.h>

#define SWELL_CHANNEL   1
#define GREAT_CHANNEL   2
#define PEDAL_CHANNEL   3
#define CHOIR_CHANNEL   4
#define SOLO_CHANNEL    5

MIDI_CREATE_DEFAULT_INSTANCE();


bool  activeNotes[128];
int   currentPedalNote      = -1;
int   lastPedalNote         = -1;
bool  lowestNoteWasReleased = false;
bool  cutoff                = false;


void setup() {
  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.turnThruOff();
}

void loop() {
  if (cutoff) {
    delay(10000000);
    return;
  }
  MIDI.read();
  sendPedalNotes();
}


void handleNoteOn(byte channel, byte note, byte velocity) {
  if (channel == PEDAL_CHANNEL) {
    return;
  }
  activeNotes[note] = true;
  if (lowestNoteWasReleased) {
    lowestNoteWasReleased = note > lastPedalNote;
  }
}

void handleNoteOff(byte channel, byte note, byte velocity) {
  if (channel == PEDAL_CHANNEL) {
    return;
  }
  activeNotes[note] = false;
  lowestNoteWasReleased = (lastPedalNote == note)
    && (lowestNoteOn() > note)
    && (autoPedalDisabled());
}


void sendPedalNotes() {
  currentPedalNote = lowestNoteOn();
  if (currentPedalNote == lastPedalNote) {
    return;
  }
  if (lastPedalNote >= 0) {
    MIDI.sendNoteOff(lastPedalNote, 0, PEDAL_CHANNEL);
  }
  lastPedalNote = currentPedalNote;

  if (currentPedalNote < 0) {
    return;
  }

  if (lowestNoteWasReleased) {
    return;
  }
  
  MIDI.sendNoteOn(currentPedalNote, 64, PEDAL_CHANNEL);

}

int lowestNoteOn() {
  for (int i = 0; i < 128; i++) {
    if (activeNotes[i]) {
      return i;
    }
  }
  return -1;
}

bool autoPedalDisabled() {
  byte numNotes = 0;
  byte largestGap = 0;
  byte bottom = -1;
  byte top = -1;
  for (int i = 0; i < 128; i++) {
    if (activeNotes[i]) {
      top = i;
      largestGap = top - bottom > largestGap && bottom >= 0 ? top - bottom : largestGap;
      numNotes++;;
      bottom = i;
    }
  }
  return numNotes > 2 && largestGap < 8;
}

void panicButton() {
  for (byte i = 0; i < 128; i++) {
    MIDI.sendNoteOff(i, 0, PEDAL_CHANNEL);
  }
  cutoff = true;
}


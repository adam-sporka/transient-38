////////////////////////////////////////////////////////////////
// CONSTANTS & GLOBAL VARIABLES
////////////////////////////////////////////////////////////////

#include "pitches.h"
const uint16_t sample_rate = 32000; // Samples per second

////////////////////////////////////////////////////////////////
// SYNTH
////////////////////////////////////////////////////////////////

#include "synthesizer.h"
ARDUINO_VOLATILE CSynthesizer synth;
byte midi_panic = 0;

#include "song.h"
#include "editor.h"
#include "midi.h"


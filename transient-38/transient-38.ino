#define ARDUINO_VOLATILE volatile
#define ARDUINO_INLINE __attribute__((always_inline))

////////////////////////////////////////////////////////////////
// LCD
////////////////////////////////////////////////////////////////

#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
#include "display-setup.h"

////////////////////////////////////////////////////////////////
// KEYBOARD
////////////////////////////////////////////////////////////////

#include "keys.h"
#include "lightweight-keypad.h"
volatile CLightweightKeypad keypad;

const uint16_t xtal_divider = 500; // xtal_divider * sample_rate should be 16,000,000
// const uint16_t xtal_divider = 1000; // xtal_divider * sample_rate should be 16,000,000

#include "main.h"

void lcdPrintFromProgmem(const char* progmemStr) {
  char c;
  while ((c = pgm_read_byte(progmemStr++))) {
    lcd.write(c);
  }
}

////////////////////////////////////////////////////////////////
void setup()
{
  Serial.begin(31250);

  // INIT LCD
  lcd.begin();
  lcd.backlight();
  init_charset();  
  lcd.cursor_on();

  // INIT KEYPAD
  keypad.initialize();

  // INIT OUTPUT
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);

  // "AUDIO THREAD"
  cli() ;                      // Disable interrupts during setup
    TCCR1A = 0;                // Clear control register A
    TCCR1B = 0;                // Clear control register B
    TCNT1  = 0;                // Initialize counter value
    OCR1A = xtal_divider - 1;  // Compare match register (num ticks between interrupts)
    TCCR1B |= (1 << WGM12);    // CTC mode
    TCCR1B |= (1 << CS10);     // No prescaler (prescaler = 1)
    TIMSK1 |= (1 << OCIE1A);   // Enable timer compare interrupt
  sei();                       // Enable global interrupts

  editor.flashMessage(F("Transient 38 /v3"), 255);
  editor.flashMessageL2(F("by AdamJ/MovSD"));
}

////////////////////////////////////////////////////////////////

unsigned char kbd_tick = 0;
unsigned char midi_tick = 15;

ISR(TIMER1_COMPA_vect)
{
  checkMIDI();

  bool update_thread = synth.onSample();
  if (update_thread == 1)
  {
    song.onTick_();
  }

  if (kbd_tick == 0)
  {
    kbd_tick = 60;
    keypad.update();
  }
  else
  {
    kbd_tick--;
  }
}

////////////////////////////////////////////////////////////////
unsigned int loop_div = 0;
void loop()
{
  if (loop_div == 0) {
    loop_div = 1024;
    editor.updateFromLoop();
  }
  loop_div--;
}

// LIGHTWEIGHT 4-BY-4 KEYPAD
// Copyright (c) 2025 Adam Sporka
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#ifndef __LIGHTWEIGHT_KEYPAD__
#define __LIGHTWEIGHT_KEYPAD__

#define LWK_ROWS 4
#define LWK_COLS 4
// byte LWK_PINS_ROWS[LWK_ROWS] = {5, 4, 3, 2};
// byte LWK_PINS_COLS[LWK_COLS] = {6, 7, 8, 9};

byte LWK_PINS_ROWS_(byte row)
{
  switch (row)
  {
    case 0: return 5;
    case 1: return 4;
    case 2: return 3;
    case 3: return 2;
  }
}

byte LWK_PINS_COLS_(byte col)
{
  switch (col)
  {
    case 0: return 6;
    case 1: return 7;
    case 2: return 8;
    case 3: return 9;
  }
}

#define LWK_START 0x20
#define LWK_DEACTIVATE_ROW 0x30
#define LWK_ACTIVATE_ROW 0x40
#define LWK_READ_COLUMN 0x50
#define LWK_END 0x60
#define LWK_SEQ_LENGTH 26

class CLightweightKeypad
{
private:

public:
  ARDUINO_VOLATILE uint16_t held;
  ARDUINO_VOLATILE uint16_t pressed;

  // Instructions for the scanner
  // Perform one at a time, one per each update()
  const byte scanlist[LWK_SEQ_LENGTH] =
  {
    LWK_START           | 0,

    LWK_ACTIVATE_ROW    | 3,
    LWK_DEACTIVATE_ROW  | 0,

    LWK_READ_COLUMN     | 0,
    LWK_READ_COLUMN     | 1,
    LWK_READ_COLUMN     | 2,
    LWK_READ_COLUMN     | 3,

    LWK_ACTIVATE_ROW    | 0,
    LWK_DEACTIVATE_ROW  | 1,

    LWK_READ_COLUMN     | 0,
    LWK_READ_COLUMN     | 1,
    LWK_READ_COLUMN     | 2,
    LWK_READ_COLUMN     | 3,

    LWK_ACTIVATE_ROW    | 1,
    LWK_DEACTIVATE_ROW  | 2,

    LWK_READ_COLUMN     | 0,
    LWK_READ_COLUMN     | 1,
    LWK_READ_COLUMN     | 2,
    LWK_READ_COLUMN     | 3,

    LWK_ACTIVATE_ROW    | 2,
    LWK_DEACTIVATE_ROW  | 3,

    LWK_READ_COLUMN     | 0,
    LWK_READ_COLUMN     | 1,
    LWK_READ_COLUMN     | 2,
    LWK_READ_COLUMN     | 3,

    LWK_END             | 0,
  };

  ARDUINO_VOLATILE byte step;
  ARDUINO_VOLATILE byte scanned_key;
  ARDUINO_VOLATILE byte last_key;

public:
  void initialize()
  {
    for (int r = 0; r < LWK_ROWS; r++) 
    {
      pinMode(LWK_PINS_ROWS_(r), OUTPUT);  
      digitalWrite(LWK_PINS_ROWS_(r), LOW);
    }
    for (int c = 0; c < LWK_COLS; c++)
    {
      pinMode(LWK_PINS_COLS_(c), INPUT_PULLUP);  
    }
    step = 0;         // Step in the "action list"
    scanned_key = 0;  // Key being currently looked at
    last_key = 0xff;     // Last key detected
    pressed = 0;      // Bit field of just pressed keys
    held = 0;         // Bit field of held keys
  }

  // Call this often.
  // To complete one scanning cycle, you need 26 calls of this function.
  // However, each call only does very little, so you can safely use this
  // in a kHz timer callback.
  ARDUINO_INLINE void update()
  {
    byte current_instruction = scanlist[step] & 0xf0;
    byte current_parameter = scanlist[step] & 0x0f;

    switch (current_instruction)
    {
    case LWK_START:
      break;
    case LWK_ACTIVATE_ROW:
      digitalWrite(LWK_PINS_ROWS_(current_parameter), HIGH);
      break;
    case LWK_DEACTIVATE_ROW:
      digitalWrite(LWK_PINS_ROWS_(current_parameter), LOW);
      break;
    case LWK_READ_COLUMN:
      {
        uint16_t weight = 1 << scanned_key;
        // Key is down
        if (digitalRead(LWK_PINS_COLS_(current_parameter)) == LOW)
        {
          // Just pressed
          if (!(held & weight))
          {
            pressed = pressed | weight;
            held = held | weight;
            last_key = scanned_key;
          }
          // Held already
          else 
          {
            pressed = pressed & (~weight);
          }
        }
        // Key is up
        else
        {
          pressed = pressed & (~weight);
          held = held & (~weight);
        }
        // Advance the scanner coordinates
        scanned_key = (scanned_key + 1) & 0x0f;
      }
      break;
    case LWK_END:
      break;
    }

    // Increment step counter and wrap around
    step++;
    if (step >= LWK_SEQ_LENGTH)
    {
      step = 0;
    }
  }

  ARDUINO_INLINE bool isPressed(byte key)
  {
    uint16_t weight = 1 << key;
    bool b = held & weight;
    return b;
  }

  ARDUINO_INLINE bool consumePressed(byte key)
  {
    uint16_t weight = 1 << key;
    bool b = pressed & weight;
    pressed = pressed & (~weight);
    return b;
  }

  ARDUINO_INLINE bool consumePressedDigit(byte& out)
  {
    if (consumePressed(KEY_DIGIT_0)) { out = 0; return true; }
    if (consumePressed(KEY_DIGIT_1)) { out = 1; return true; }
    if (consumePressed(KEY_DIGIT_2)) { out = 2; return true; }
    if (consumePressed(KEY_DIGIT_3)) { out = 3; return true; }
    if (consumePressed(KEY_DIGIT_4)) { out = 4; return true; }
    if (consumePressed(KEY_DIGIT_5)) { out = 5; return true; }
    if (consumePressed(KEY_DIGIT_6)) { out = 6; return true; }
    if (consumePressed(KEY_DIGIT_7)) { out = 7; return true; }
    if (consumePressed(KEY_DIGIT_8)) { out = 8; return true; }
    if (consumePressed(KEY_DIGIT_9)) { out = 9; return true; }
    return false;
 }

  // Return 0 for no key detected.
  // Return 1--16 if a key has been pressed.
  ARDUINO_INLINE byte getLastKey()
  {
    byte to_report = last_key;
    last_key = 0xff;
    return to_report;
  }
};

#endif // __LIGHTWEIGHT_KEYPAD__
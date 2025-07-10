enum EEditorMode
{
  EDITOR_MODE_SONG = 0,
  EDITOR_MODE_PATTERNS = 1,
  EDITOR_MODE_PERFORM = 2,
  EDITOR_MODE_MAX = 3
};

class CEditor
{
  unsigned int flash_message_timer;
  EEditorMode mode = EDITOR_MODE_SONG;

  unsigned char song_pos = 0;
  unsigned char voice_pos = 0;

  unsigned char current_pattern = 0;
  unsigned char pattern_pos = 0;
  unsigned char current_instrument = 0;
  unsigned char current_octave = 4;

public:
  inline void printPadded(uint8_t num)
  {
    if (num == 0) lcd.print(F("00"));
    else if (num < 10)
    {
      lcd.write('0');
      lcd.write(num + '0');
    }
    else {
      lcd.write((num / 10) + '0');
      lcd.write((num % 10) + '0');
    }
  }

  void flashMessage(const char* message, byte timeout = 64)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(message);
    flash_message_timer = timeout;
  }

  void flashMessageL2(const char* message)
  {
    lcd.setCursor(0, 1);
    lcd.print(message);
  }

  void flashMessageL2N(unsigned char number)
  {
    lcd.setCursor(0, 1);
    lcd.write(number + '0');
  }

#ifdef ARDUINO
  void flashMessage(const __FlashStringHelper* message, byte timeout = 64)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(message);
    flash_message_timer = timeout;
  }
#endif

#ifdef ARDUINO
  void flashMessageL2(const __FlashStringHelper* message)
  {
    lcd.setCursor(0, 1);
    lcd.print(message);
  }
#endif

  void updateScreen()
  {
    if (flash_message_timer == 1)
    {
      flash_message_timer = 0;
      redrawModeScreen();
      return;
    }
    else if (flash_message_timer)
    {
      flash_message_timer--;
      return;
    }
  }

  void redrawModeScreen()
  {
    switch (mode)
    {
    case EDITOR_MODE_SONG:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("Pos V1 V2 V3 Tem"));
      drawSongVariables();
      break;
    case EDITOR_MODE_PATTERNS:
      lcd.clear();
      drawPattern();
      if (keypad.isPressed(KEY_SHIFT))
      {
        drawStatus();
      }
      else
      {
        drawOctaves();
      }
      updatePatternCursor();
      break;
    case EDITOR_MODE_PERFORM:
      lcd.clear();
      drawPerformStatus();
      break;
    }
  }

  void switchMode(EEditorMode new_mode)
  {
    mode = new_mode;
    switch (new_mode)
    {
      case EDITOR_MODE_SONG: flashMessage(F("SONG")); break;
      case EDITOR_MODE_PATTERNS: flashMessage(F("PATTERNS")); break;
      case EDITOR_MODE_PERFORM: flashMessage(F("PERFORM")); break;
    }
  }

  ////////
  // SONG MODE

  void drawSongVariables()
  {
    lcd.setCursor(0, 1);
    printPadded(song_pos);

    if (song_pos == song.song_length - 1)
    {
      lcd.write(']');
    }
    else
    {
      lcd.write(' ');
    }

    for (byte v = 0; v < 3; v++)
    {
      lcd.setCursor(v * 3 + 4, 1);
      printPadded(song.orders[song_pos].v[v]);
    }

    lcd.setCursor(13, 1);
    printPadded(song.tempo_odd);
    if (song.tempo_swing)
    {
      lcd.write('s');
    }
    else lcd.write(' ');
    updateVoiceCursor();
  }

  void prevSongPos()
  {
    song_pos = (song_pos + 31) & 31;
    drawSongVariables();
    updateVoiceCursor();
  }

  void nextSongPos()
  {
    song_pos = (song_pos + 1) & 31;
    drawSongVariables();
    updateVoiceCursor();
  }

  void nextVoice()
  {
    voice_pos = (voice_pos + 1) % 3;
    updateVoiceCursor();
  }

  void updateVoiceCursor()
  {
    lcd.setCursor((voice_pos * 3) + 5, 1);
  }

  void updateSongMode()
  {
    if (keypad.isPressed(KEY_SHIFT))
    {
      if (keypad.consumePressed(KEY_TEMPO_LOWER)) {
        song.setTempo(song.getTempo() - 1);
        drawSongVariables();
      }
      if (keypad.consumePressed(KEY_TEMPO_HIGHER)) {
        song.setTempo(song.getTempo() + 1);
        drawSongVariables();
      }
      if (keypad.consumePressed(KEY_TEMPO_SWING)) {
        song.setSwing(!song.tempo_swing);
        drawSongVariables();
      }
      if (keypad.consumePressed(KEY_TEMPO_SETLAST)) {
        song.song_length = song_pos + 1;
        drawSongVariables();
      }
    }
    else
    {
      byte d;
      if (keypad.consumePressedDigit(d))
      {
        byte new_o = song.orders[song_pos].v[voice_pos];
        new_o = new_o % 10;
        new_o = new_o * 10;
        new_o += d;
        if (new_o > 31)
        {
          new_o = 31;
        }
        song.orders[song_pos].v[voice_pos] = new_o;
        drawSongVariables();
      }

      if (keypad.consumePressed(KEY_NEXT_VOICE))
      {
        nextVoice();
      }

      if (keypad.consumePressed(KEY_NEXT_SONG_POS))
      {
        nextSongPos();
      }

      if (keypad.consumePressed(KEY_PREV_SONG_POS))
      {
        prevSongPos();
      }
    }
  }

  ////////
  // SONG_PATTERN MODE

  void drawPattern()
  {
    lcd.setCursor(0, 0);
    for (int a = 0; a < 16; a++)
    {
      lcd.write(getChar(patterns[current_pattern].notes[a].getNote()));
    }
  }

  void drawStatus()
  {
    lcd.setCursor(0, 1);
    lcd.print(F("PTN. "));
    printPadded(current_pattern);
    lcd.write(' ');
  }

  void drawOctaves()
  {
    lcd.setCursor(0, 1);
    for (int a = 0; a < 16; a++)
    {
      byte note = patterns[current_pattern].notes[a].getNote();
      if (note > 0 && note < 127) lcd.write(getOctave(note));
      else lcd.write(' ');
    }
  }

  void updatePatternCursor()
  {
    lcd.setCursor(pattern_pos, 0);
  }

  void enterNote(byte note_name)
  {
    byte note = (note_name % 12);
    byte octave = current_octave - (note == 11 ? 1 : 0);
    note = note + octave * 12;
    patterns[current_pattern].notes[pattern_pos].setNote(note);
    patterns[current_pattern].notes[pattern_pos].setInstrument(current_instrument);
    drawPattern();
    drawOctaves();
    pattern_pos++;
    if (pattern_pos >= 16) pattern_pos = 0;
    updatePatternCursor();
  }

  byte prev_shift = false;
  void updatePatternMode()
  {
    if (keypad.isPressed(KEY_SHIFT))
    {
      if (!prev_shift)
      {
        drawStatus();
        prev_shift = true;
      }

      if (keypad.consumePressed(KEY_CLEAR_PTN))
      {
        patterns[current_pattern].clear();
        pattern_pos = 0;
        drawPattern();
        updatePatternCursor();
      }
      if (keypad.consumePressed(KEY_CLEAR_SPACE))
      {
        patterns[current_pattern].notes[pattern_pos].clear();
        drawPattern();
        pattern_pos++;
        if (pattern_pos >= 16) pattern_pos = 0;
        updatePatternCursor();
      }
      if (keypad.consumePressed(KEY_CLEAR_BREAK))
      {
        patterns[current_pattern].notes[pattern_pos].setNote(127);
        drawPattern();
        pattern_pos++;
        if (pattern_pos >= 16) pattern_pos = 0;
        updatePatternCursor();
      }
      if (keypad.consumePressed(KEY_PREV_PATTERN))
      {
        current_pattern = (current_pattern + 31) & 0x1f;
        song.pattern_to_play = current_pattern;
        drawPattern();
        drawStatus();
        updatePatternCursor();
      }
      if (keypad.consumePressed(KEY_NEXT_PATTERN))
      {
        current_pattern = (current_pattern + 1) & 0x1f;
        song.pattern_to_play = current_pattern;
        drawPattern();
        drawStatus();
        updatePatternCursor();
      }
    }
    else
    {
      if (prev_shift)
      {
        drawOctaves();
        prev_shift = false;
      }

      if (keypad.consumePressed(KEY_LEFT))
      {
        pattern_pos = (pattern_pos + 15) & 0xf;
        updatePatternCursor();
      }
      if (keypad.consumePressed(KEY_RIGHT))
      {
        pattern_pos = (pattern_pos + 1) & 0xf;
        updatePatternCursor();
      }
      if (keypad.consumePressed(KEY_OCTAVE_PREV))
      {
        current_octave--;
        if (current_octave < 1) current_octave = 1;
        flashMessage(F("OCTAVE"));
        flashMessageL2N(current_octave);
      }
      if (keypad.consumePressed(KEY_OCTAVE_NEXT))
      {
        current_octave++;
        if (current_octave > 7) current_octave = 7;
        flashMessage(F("OCTAVE"));
        flashMessageL2N(current_octave);
      }
      if (keypad.consumePressed(KEY_INSTR_NEXT))
      {
        if (current_instrument < 7) current_instrument++;
        flashMessage(F("INSTRUMENT"));
        flashMessageL2(instruments[current_instrument].name);
      }
      if (keypad.consumePressed(KEY_INSTR_PREV))
      {
        if (current_instrument > 0) current_instrument--;
        flashMessage(F("INSTRUMENT"));
        flashMessageL2(instruments[current_instrument].name);
      }
      if (keypad.consumePressed(KEY_B)) enterNote(59 + keypad.isPressed(KEY_SHARP));
      if (keypad.consumePressed(KEY_C)) enterNote(60 + keypad.isPressed(KEY_SHARP));
      if (keypad.consumePressed(KEY_D)) enterNote(62 + keypad.isPressed(KEY_SHARP));
      if (keypad.consumePressed(KEY_E)) enterNote(64 + keypad.isPressed(KEY_SHARP));
      if (keypad.consumePressed(KEY_F)) enterNote(65 + keypad.isPressed(KEY_SHARP));
      if (keypad.consumePressed(KEY_G)) enterNote(67 + keypad.isPressed(KEY_SHARP));
      if (keypad.consumePressed(KEY_A)) enterNote(69 + keypad.isPressed(KEY_SHARP));
    }
  }

  ////////
  // SONG_PATTERN MODE

  void drawPerformStatus()
  {
    if (mono_voice == 0xff)
    {
      lcd.setCursor(0,0);
      lcd.print(F("POLYPHONIC  "));
      lcd.setCursor(10, 0);
    }
    else if (mono_voice == 0x7f)
    {
      lcd.setCursor(0,0);
      lcd.print(F("UNISON      "));
      lcd.setCursor(6, 0);
    }
    else
    {
      lcd.setCursor(0,0);
      lcd.print(F("MONO VOICE "));
      lcd.write(mono_voice + 1 + '0');
      lcd.setCursor(11, 0);
    }
  }

  void updatePerformMode()
  {
    if (keypad.isPressed(KEY_SHIFT))
    {
    }
    else
    {
      if (keypad.consumePressed(KEY_PANIC))
      {
        synth.panic();
        flashMessage(F("PANIC!"));
        midi_panic = 1;
      }
      if (keypad.consumePressed(KEY_MONO_1))
      {
        mono_voice = 0;
        drawPerformStatus();
      }
      if (keypad.consumePressed(KEY_MONO_2))
      {
        mono_voice = 1;
        drawPerformStatus();
      }
      if (keypad.consumePressed(KEY_MONO_3))
      {
        mono_voice = 2;
        drawPerformStatus();
      }
      if (keypad.consumePressed(KEY_POLY))
      {
        mono_voice = 0xff;
        drawPerformStatus();
      }
      if (keypad.consumePressed(KEY_UNISON))
      {
        mono_voice = 0x7f;
        drawPerformStatus();
      }
    }
  }

  ////////
  // COMMON

  bool first_tick = true;
  void updateFromLoop()
  {
    updateScreen();
    // SHIFT
    if (keypad.isPressed(KEY_SHIFT))
    {
      if (first_tick)
      {
        song.demo();
        flashMessage(F("DEMO"));
        first_tick = false;
        return;
      }
      if (keypad.consumePressed(KEY_MODE))
      {
        switch (mode)
        {
        case EDITOR_MODE_SONG: switchMode(EEditorMode::EDITOR_MODE_PATTERNS); break;
        case EDITOR_MODE_PATTERNS: switchMode(EEditorMode::EDITOR_MODE_PERFORM); break;
        case EDITOR_MODE_PERFORM: switchMode(EEditorMode::EDITOR_MODE_SONG); break;
        }
        return;
      }
    }
    else
    {
      if (keypad.consumePressed(KEY_TRANSPORT))
      {
        if (song.isPlaying())
        {
          flashMessage(F("STOP"));
          song.stop();
        }
        else
        {
          if (mode == EDITOR_MODE_PATTERNS) 
          {
            flashMessage(F("PLAYING SONG_PATTERN"));
            song.play_pattern(current_pattern);
          }
          if (mode == EDITOR_MODE_SONG)
          {
            flashMessage(F("PLAYING SONG"));
            song.restart();
          }
          if (mode == EDITOR_MODE_PERFORM)
          {
            flashMessage(F("PLAYING SONG"));
            song.restart();
          }
        }
      }
      first_tick = false;
    }

    switch (mode)
    {
      case EDITOR_MODE_SONG: updateSongMode(); break;
      case EDITOR_MODE_PATTERNS: updatePatternMode(); break;
      case EDITOR_MODE_PERFORM: updatePerformMode(); break;
    }
  }
};

CEditor editor;

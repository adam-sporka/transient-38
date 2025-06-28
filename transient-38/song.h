////////////////////////////////////////////////////////////////
struct NOTE
{
  byte N;   // .NNNNNNN ... note number
  byte IV;  // VRvvviii ...
            //   V = vibrato
            //   R = ramp
            //   vvv = volume decrement 0--7 (i.e. higher numer == lower volume)
            //   iii = instrument 0--7

  void pack(byte midi_note, byte instrument, byte volume_decrement, bool vibrato, bool ramp)
  {
    N = midi_note & 0x7f;
    IV = (instrument & 0x7) | ((volume_decrement & 0x7) << 3) | (vibrato ? 0x80 : 0) | (ramp ? 0x40 : 0);
  }

  void setNote(byte midi_note)
  {
    N = (midi_note & 0x7f) | (N & 0x80);
  }

  inline setInstrument(byte instrument)
  {
    IV = (IV & 0b11111000) | (instrument & 0x7);
  }

  inline byte getInstr()
  {
    return IV & 0x7;
  }

  inline byte getVolumeDecrement()
  {
    return (IV >> 3) & 0x7;
  }

  inline bool getRamp()
  {
    return IV & (1 << 6);
  }

  inline bool getVibrato()
  {
    return IV & (1 << 7);
  }

  inline byte getNote()
  {
    return N & 0x7f;
  }

  void clear()
  {
    N = 0;
    IV= 0;
  }
};

byte getChar(byte note)
{
  if (note == 0)
  {
    return '.';
  }
  if (note == 127)
  {
    return 0b11101011; // https://mil.ufl.edu/4744/docs/lcdmanual/characterset.html
  }
  byte pitch = note % 12;
  switch (pitch)
  {
    case 0: return 'C';
    case 1: return 0;
    case 2: return 'D';
    case 3: return 1;
    case 4: return 'E';
    case 5: return 'F';
    case 6: return 2;
    case 7: return 'G';
    case 8: return 3;
    case 9: return 'A';
    case 10: return 4;
  }
  return 'B';
}

////////////////////////////////////////////////////////////////
struct PATTERN
{
  NOTE notes[16];

  void clear()
  {
    for (int a = 0; a < 16; a++)
    {
      notes[a].N = 0;
      notes[a].IV = 0;
    }
  }

  void make44Beat()
  {
    notes[0].pack(1, 4, 0, 0, 0);
    notes[1].pack(1, 6, 0, 0, 0);
    notes[2].pack(1, 6, 0, 0, 0);
    notes[3].pack(1, 4, 0, 0, 0);
    notes[4].pack(1, 5, 0, 0, 0);
    notes[5].pack(1, 6, 0, 0, 0);
    notes[6].pack(1, 4, 0, 0, 0);
    notes[7].pack(1, 6, 0, 0, 0);
    notes[8].pack(1, 6, 0, 0, 0);
    notes[9].pack(1, 6, 0, 0, 0);
    notes[10].pack(1, 4, 0, 0, 0);
    notes[11].pack(1, 6, 0, 0, 0);
    notes[12].pack(1, 5, 0, 0, 0);
    notes[13].pack(1, 6, 0, 0, 0);
    notes[14].pack(1, 6, 0, 0, 0);
    notes[15].pack(1, 6, 0, 0, 0);
  }

  void make44Beat2()
  {
    notes[0].pack(1, 4, 0, 0, 0);
    notes[1].pack(1, 6, 0, 0, 0);
    notes[2].pack(1, 6, 0, 0, 0);
    notes[3].pack(1, 4, 0, 0, 0);
    notes[4].pack(1, 5, 0, 0, 0);
    notes[5].pack(1, 6, 0, 0, 0);
    notes[6].pack(1, 4, 0, 0, 0);
    notes[7].pack(1, 6, 0, 0, 0);
    notes[8].pack(1, 6, 0, 0, 0);
    notes[9].pack(1, 6, 0, 0, 0);
    notes[10].pack(1, 4, 0, 0, 0);
    notes[11].pack(1, 6, 0, 0, 0);
    notes[12].pack(60, 7, 0, 0, 0);
    notes[13].pack(60, 7, 0, 0, 0);
    notes[14].pack(55, 7, 0, 0, 0);
    notes[15].pack(50, 7, 0, 0, 0);
  }

  void makeBass(byte xpose)
  {
    notes[0].pack(xpose + 36, 0, 0, 0, 0);
    notes[2].pack(127, 0, 0, 0, 0);
    notes[3].pack(xpose + 36, 0, 0, 0, 0);
    notes[4].pack(xpose + 46, 0, 0, 0, 0);
    notes[6].pack(xpose + 48, 0, 0, 1, 0);
    notes[10].pack(xpose + 36, 0, 0, 0, 0);
    notes[11].pack(xpose + 35, 0, 0, 0, 1);
    notes[12].pack(127, 0, 0, 0, 0);
  }

  void makeArp1(byte xpose)
  {
    notes[0].pack(xpose + 48 + 7, 2, 0, 0, 0);
    notes[1].pack(xpose + 48 + 10, 2, 1, 0, 0);
    notes[2].pack(xpose + 48 + 12, 2, 2, 0, 0);
    notes[3].pack(xpose + 48 + 7, 2, 0, 0, 0);
    notes[4].pack(xpose + 48 + 10, 2, 1, 0, 0);
    notes[5].pack(xpose + 48 + 12, 2, 2, 0, 0);
  }

  void makeMelody()
  {
    notes[0].pack(60 + 7, 3, 0, 0, 0);
    notes[1].pack(60 + 5, 3, 1, 0, 1);
    notes[2].pack(127, 3, 7, 0, 0);
    notes[3].pack(60 + 7, 3, 0, 1, 0);
    notes[8].pack(60 + 5, 3, 0, 1, 0);
    notes[9].pack(60 + 3, 3, 0, 1, 0);
    notes[10].pack(127, 3, 7, 0, 0);
    notes[11].pack(60 + 5, 3, 0, 1, 0);
  }

  void makeMelody2()
  {
    notes[0].pack(60 + 5, 3, 0, 0, 0);
    notes[2].pack(60 + 3, 3, 1, 0, 0);
    notes[4].pack(60 + 0, 3, 0, 0, 0);
    notes[6].pack(60 + 3, 3, 0, 0, 1);
    notes[7].pack(60 + 7, 3, 0, 1, 0);
    notes[10].pack(60 + 0, 3, 0, 0, 0);
    notes[12].pack(127, 3, 7, 0, 0);
  }
};

// Global pattern pool
volatile PATTERN patterns[32];

////////////////////////////////////////////////////////////////
struct ORDER
{
  byte v[3];
  inline byte getPtn(byte voice)
  {
    return v[voice];
  }

  void choosePatterns(byte p1, byte p2, byte p3)
  {
    v[0] = p1;
    v[1] = p2;
    v[2] = p3;
  }
};

////////////////////////////////////////////////////////////////
#define PLAY_SONG 1
#define PLAY_PATTERN 2

struct SONG
{
  ORDER orders[32];
  byte song_length = 16;

  byte pos_order = 0;
  byte pos_pattern = 0;
  byte tempo_odd = 8;
  byte tempo_even = 4;
  bool tempo_swing = true;
  byte tick_counter = 0;

  byte transport = 0;
  byte play_mode = 0;
  byte pattern_to_play = 0;

  void setTempo(byte tempo)
  {
    if (tempo < 2) tempo = 2;
    if (tempo > 32) tempo = 32;
    tempo_odd = tempo;
    if (tempo_swing)
    {
      tempo_even = tempo_odd >> 1;
    }
    else
    {
      tempo_even = tempo_odd;
    }
  }

  byte getTempo()
  {
    return tempo_odd;
  }

  void setSwing(bool swing)
  {
    tempo_swing = swing;
    if (tempo_swing) tempo_even = tempo_odd >> 1;
    else tempo_even = tempo_odd;
  }

  void seek_begin()
  {
    pos_order = 0;
    pos_pattern = 0;
    tick_counter = 0;
  }

  bool isPlaying()
  {
    return (transport != 0);
  }

  void restart()
  {
    seek_begin();
    play_mode = PLAY_SONG;
    transport = 1;
  }

  void play_pattern(byte pattern)
  {
    play_mode = PLAY_PATTERN;
    pattern_to_play = pattern;
    pos_pattern = 0;
    transport = 1;
  }

  void stop()
  {
    for (byte v = 0; v < 3; v++)
    {
      synth.endNote(v);
    }
    transport = 0;
  }

  void onTick()
  {
    if (!transport)
    {
      return;
    }

    if (tick_counter == 0)
    {
      volatile ORDER &o = orders[pos_order];
      for (byte v = 0; v < 3; v++)
      {
        if (play_mode == PLAY_PATTERN)
        {
          if (v > 0) break;
        }

        volatile PATTERN *p;
        if (play_mode == PLAY_SONG) p = &patterns[o.v[v]];
        else p = &patterns[pattern_to_play];
        volatile NOTE &n = p->notes[pos_pattern];
        
        byte note = n.getNote();
        if (note == 127)
        {
          synth.endNote(v);
        }
        else if (note > 0)
        {
          synth.startNote(v, note, instruments[n.getInstr()], n.getVolumeDecrement(), n.getVibrato(), n.getRamp());
        }
        else
        {
          synth.setRuntimeParams(v, n.getVolumeDecrement(), n.getVibrato(), n.getRamp());
        }
      }

      tick_counter = pos_pattern % 2 ? tempo_even : tempo_odd;

      pos_pattern++;
      if (pos_pattern == 16)
      {
        pos_pattern = 0;
        pos_order++;

        if (pos_order >= song_length)
        {
          pos_order = 0;
        }
      }
    }
    else
    {
      tick_counter--;
    }
  }

  byte letterToNote(char c)
  {
    switch (c)
    {
      case 'C': return 60;
      case 'D': return 62;
      case 'E': return 64;
      case 'F': return 65;
      case 'G': return 67;
      case 'A': return 69;
      case 'B': return 71;
    }
    return 71;
  }

  void stringProgrammer(byte pattern, const __FlashStringHelper* input)
  {
    if (pattern > 31)
    {
      return;
    }
    byte state = 0;
    byte pattern_pos = 0;
    byte note = 0;
    byte vibrato = false;
    byte ramp = false;
    byte octave = 4;
    byte instrument = 0;
    byte volume = 0;
    PATTERN *p = patterns + pattern;

    const char* data = (const char*)input;
    for (byte i = 0; ; i++)
    {
      char c = pgm_read_byte_near(data++);
      // Immediate
      if (c == '*') { p->notes[pattern_pos++].pack(60, 4, 0, 0, 0); }
      if (c == '+') { p->notes[pattern_pos++].pack(60, 5, 0, 0, 0); }
      if (c == '.') { p->notes[pattern_pos++].pack(60, 6, 0, 0, 0); }
      if (c == '(') { p->notes[pattern_pos++].pack(64, 7, 0, 0, 0); }
      if (c == ')') { p->notes[pattern_pos++].pack(59, 7, 0, 0, 0); }        

      // Compound
      if (c >= 'A' && c <= 'G') {
        vibrato = false;
        ramp = false;
        note = letterToNote(c) % 12; 
      }
      if (c == '#') note++;
      if (c >= '0' && c <= '7') { octave = c - '0'; };
      if (c == '~') vibrato = true;
      if (c == '/') ramp = true;
      if (c == '_') volume = 2;
      if (c >= 'a' && c <= 'h') instrument = c - 'a';

      if (c == ';')
      {
        p->notes[pattern_pos++].pack(note + (octave + 1) * 12, instrument, volume, vibrato, ramp);
      }

      // Break
      if (c == '-')
      {
        p->notes[pattern_pos++].pack(0, 0, volume, vibrato, ramp);
      }

      if (c == 'x')
      {
        p->notes[pattern_pos++].pack(0x7f, 0, volume, vibrato, ramp);
      }

      if (c == '\0') break;  // End of string
    } 
  }

  void demo()
  {
    stringProgrammer(30, F(".... (... ..(. ...."));
    stringProgrammer(0, F("aC2;--- C2;A#3;-C3; x---;A1;-xA#1;-"));
    orders[0].choosePatterns(0, 30, 31);
    orders[1].choosePatterns(0, 30, 31);
    stringProgrammer(29, F("*... +... *.(* +())"));
    orders[2].choosePatterns(0, 29, 31);
    orders[3].choosePatterns(0, 29, 31);

    stringProgrammer(1, F("a *-C2-;+...*C2;-*+A#0)"));
    stringProgrammer(2, F("c_ C4;D4;G4;C4;D4;G4;xxC4;D4;G4;C4;D4;G4;xx"));

    stringProgrammer(3, F("d C5;-C5;xG4~;--A;A#;A;A#;-G;"));
    stringProgrammer(4, F("d F4;-F4;G4;F4;D#4;C;D#4~/;----x"));

    orders[0].choosePatterns(1, 2, 31);
    orders[1].choosePatterns(1, 2, 31);
    orders[2].choosePatterns(1, 2, 3);
    orders[3].choosePatterns(1, 2, 4);
    orders[4].choosePatterns(1, 2, 3);
    stringProgrammer(5, F("d F4;D#;F;G;F;D#;C;C;x"));
    orders[5].choosePatterns(1, 2, 5);
    orders[6].choosePatterns(1, 2, 3);
    orders[7].choosePatterns(1, 2, 4);
    orders[8].choosePatterns(1, 2, 3);
    orders[9].choosePatterns(1, 2, 5);

    stringProgrammer(6, F("a *-C#2-;+...*A#1;-*+A#1;)"));
    stringProgrammer(7, F("c_ C#4;D#;F;C#;D#;F;xx;F;G;G#;F;G;G#;xx"));
    stringProgrammer(8, F("c_ C4;D;E;C;D;E;xxC;D;E;C;D;E;xx"));
    stringProgrammer(13, F("c_ ----x"));
    stringProgrammer(9, F("d F;G;G#;A#;C5;C#;D#;C;C;C#;C;G#4;G;G#;C;E;"));
    stringProgrammer(10, F("d -------G4;x"));
    stringProgrammer(11, F("a *-C2-;+..A#1;x"));
    stringProgrammer(12, F("c_ C4;D;E;C;D;E;F#;G;"));

    orders[10].choosePatterns(6, 7, 9);
    orders[11].choosePatterns(1, 8, 13);
    orders[12].choosePatterns(6, 7, 9);
    orders[13].choosePatterns(11, 12, 10);
    song_length = 14;
  }

  void demo_()
  {
    patterns[0].make44Beat();
    patterns[1].make44Beat2();
    patterns[0x10].makeBass(0);
    patterns[0x11].makeBass(3);
    patterns[0x12].makeBass(5);
    patterns[0x13].makeBass(8);
    patterns[0x14].makeArp1(0);
    patterns[0x15].makeArp1(3);
    patterns[0x16].makeArp1(5);
    patterns[0x17].makeArp1(5);
    patterns[0x18].makeMelody();
    patterns[0x19].makeMelody2();

    orders[0].choosePatterns ( 1, 0x10, 0x18);
    orders[1].choosePatterns ( 0, 0x11, 0x19);
    orders[2].choosePatterns ( 0, 0x12, 31);
    orders[3].choosePatterns ( 1, 0x13, 31);

    orders[4].choosePatterns ( 0,   31, 0x14);
    orders[5].choosePatterns ( 0,   31, 0x14);
    orders[6].choosePatterns ( 0,   31, 0x14);
    orders[7].choosePatterns (31,   31, 0x14);

    orders[8].choosePatterns ( 0, 0x10, 0x14);
    orders[9].choosePatterns ( 0, 0x11, 0x15);
    orders[10].choosePatterns(0,  0x12, 0x16);
    orders[11].choosePatterns(31, 0x13, 0x17);

    orders[12].choosePatterns( 0, 0x10, 0x18);
    orders[13].choosePatterns( 0, 0x11, 0x19);
    orders[14].choosePatterns(0,  0x12, 0x1a);
    orders[15].choosePatterns(31, 0x13, 0x1b);
  }

  SONG()
  {
    for (int o = 0; o < 32; o++)
    {
      patterns[o].clear();
    }
    for (int o = 0; o < 32; o++)
    {
      orders[o].choosePatterns(0, 10, 20);
    }
  }
};

volatile SONG song;

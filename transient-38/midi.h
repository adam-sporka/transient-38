#ifdef ARDUINO

byte midi_input;
byte would_be_note;
byte params_remaining = 0;
byte commandByte;

void checkMIDI()
{
  if (midi_panic == 1)
  {
    midi_input = 0;
    would_be_note = 0;
    params_remaining = 0;
    commandByte = 0;
    midi_panic = 0;
  }

  if (Serial.available())
  {
    midi_input = Serial.read();
    if (params_remaining == 0)
    {
      if (midi_input < 128)
      {
        params_remaining = 1;
        if (commandByte >= 128 && commandByte <= 159)
        {
          would_be_note = midi_input; 
        }
      }
      else if (midi_input < 240)
      {
        params_remaining = 2;
        commandByte = midi_input;
      }
    }
    else
    {
      if ((params_remaining == 2) && (commandByte >= 128) && (commandByte <= 159))
      {
        would_be_note = midi_input;
      }
      if ((params_remaining == 1) && (commandByte >= 144) && (commandByte <= 159))
      {
        if (midi_input > 0) 
        {
          midi_note_in = would_be_note;
          midi_note_in_channel = commandByte & 0xf;
        }
        else
        {
          midi_note_kill = would_be_note;
          midi_note_kill_channel = commandByte & 0xf;
        }
      }
      if ((params_remaining == 1) && (commandByte >= 128) && (commandByte <= 143))
      {
        midi_note_kill = would_be_note;
        midi_note_kill_channel = commandByte & 0xf;
      }
      params_remaining--;
    }

/*
    if (midi_note_in)
    {
      synth.onMidiNoteOn(midi_note_in_channel, midi_note_in, 0xf);
      midi_note_in = 0;
    }

    if (midi_note_kill)
    {
      synth.onMidiNoteOff(midi_note_kill_channel, midi_note_kill, 0xf);
      midi_note_kill = 0;
    }
*/
/*
    if (midi_note_in || midi_note_kill)
    {
      Serial.print(F("ON "));
      Serial.print(midi_note_in);
      Serial.print(F(" OFF "));
      Serial.println(midi_note_kill);
      midi_note_in = 0;
      midi_note_kill = 0;
    }
*/
  }
}
    
#endif
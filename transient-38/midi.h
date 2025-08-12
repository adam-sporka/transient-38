#ifdef ARDUINO

byte midi_input;
byte would_be_note;
byte params_remaining = 0;
byte commandByte;
byte is_sysex = 0;

ARDUINO_INLINE checkMIDI_Internal()
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

    if (!is_sysex && (midi_input == 0xf0))
    {
      is_sysex = 1;
      return;
    }

    if (is_sysex && (midi_input == 0xf7))
    {
      is_sysex = 0;
      return;
    }

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
          midi_note_in_[midi_note_in_wr] = would_be_note;
          midi_note_in_channel_[midi_note_in_wr] = commandByte & 0xf;
          midi_note_in_wr = (midi_note_in_wr + 1) & CONTROL_BUFFER_MASK;
        }
        else
        {
          midi_note_kill_[midi_note_kill_wr] = would_be_note;
          midi_note_kill_channel_[midi_note_kill_wr] = commandByte & 0xf;
          midi_note_kill_wr = (midi_note_kill_wr + 1) & CONTROL_BUFFER_MASK;
        }
      }
      if ((params_remaining == 1) && (commandByte >= 128) && (commandByte <= 143))
      {
        midi_note_kill_[midi_note_kill_wr] = would_be_note;
        midi_note_kill_channel_[midi_note_kill_wr] = commandByte & 0xf;
        midi_note_kill_wr = (midi_note_kill_wr + 1) & CONTROL_BUFFER_MASK;
      }
      params_remaining--;
    }
  }
}

ARDUINO_INLINE void checkMIDI()
{
  checkMIDI_Internal();
}
    
#endif
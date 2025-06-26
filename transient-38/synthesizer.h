#include "lfo.h"
#include "envelope.h"
#include "random.h"

#define SAMPLES_IN_TICK 640

LFO vibrato;
LFO pwm;

#define INSTR_FLAG_DO_DROP_SPD  0x07 // 0 = no drop; 1 --> 1; 2 --> 2; 3 --> 4; 4 --> 8; 5 --> 16; 6 --> 32; 7 --> 64
#define INSTR_FLAG_IS_NOISE     0x08
#define INSTR_FLAG_DO_RETRIG    0x10
#define INSTR_FLAG_DO_RAMP      0x20
#define INSTR_FLAG_DO_VIBRATO   0x40
#define INSTR_FLAG_DO_PWM       0x80

#define INSTR_ENVELOPE_PERCUSSIVE 0x08

#define INSTR_DELAYED_VIBRATO     0x80

byte midi_note_in = 0;
byte midi_note_in_channel = 0;
byte midi_note_kill = 0;
byte midi_note_kill_channel = 0;
byte mono_voice = 0xff; // 0xff == poly

struct INSTRUMENT
{
  byte attack_decay = 0; // DDDDpAAA   attack & decay
  byte defvolume_pwmf = 5;  // v.PP.VVV   delayed vibrato & volume & PWM factor
  byte drop_flags = 0;   // PVRrNddd   drop speed & other flags
  byte forced_pitch = 0; // .mmmmmmm   MIDI note that is forced; 0 == no forcing
  const __FlashStringHelper* name;

  inline byte getAtack() { return attack_decay & 0x07; }
  inline byte isPercussive() { return attack_decay & INSTR_ENVELOPE_PERCUSSIVE; }
  inline byte getDecay() { return (attack_decay & 0xf0) >> 4; }

  inline byte getDefaultVolume() { return defvolume_pwmf & 0x07; }
  inline byte getPwmFactor() { return (defvolume_pwmf & 0x30) >> 4; }
  inline bool doDelayedVibrato() { return defvolume_pwmf & INSTR_DELAYED_VIBRATO; } 

  inline byte getDropSpeed() { return (drop_flags & 0x7) ? 1 << ((drop_flags & 0x07) - 1) : 0; }
  inline bool doNoise() { return drop_flags & INSTR_FLAG_IS_NOISE; }
  inline bool doRetrig() { return drop_flags & INSTR_FLAG_DO_RETRIG; }
  inline bool doRamp() { return drop_flags & INSTR_FLAG_DO_RAMP; }
  inline bool doVibrato() { return drop_flags & INSTR_FLAG_DO_VIBRATO; }
  inline bool doPwm() { return drop_flags & INSTR_FLAG_DO_PWM; }

  inline byte getForcedPitch() { return forced_pitch; }

  void setAttackDecay(byte attack, byte decay, bool percussive)
  {
    attack_decay = (attack & 7) | (percussive ? INSTR_ENVELOPE_PERCUSSIVE : 0) | ((decay & 0xf) << 4);
  }

  // Volume = 0 .. 7
  // PWM factor = 0, 1, 2, 3
  void setDefaultVolumePwm(byte volume, byte pwm_factor, bool delayed_vibrato)
  {
    defvolume_pwmf = (volume & 7) | ((pwm_factor & 3) << 4) | ((delayed_vibrato & 1) << 7);
  }

  void setFlags(byte drop_speed, byte flags)
  {
    drop_flags = (drop_speed & 7) | flags;
  }

  void setForcedPitch(byte forced_pitch_)
  {
    forced_pitch = forced_pitch_;
  }

  void setBass()
  {
    name = F("BASS");
    setAttackDecay(0, 1, false);
    setDefaultVolumePwm(4, 1, false);
    setFlags(0, INSTR_FLAG_DO_RETRIG);
    setForcedPitch(0);
  }

  void setSquare()
  {
    name = F("SQUARE");
    setAttackDecay(0, 0, false);
    setDefaultVolumePwm(5, 0, false);
    setFlags(0, INSTR_FLAG_DO_RETRIG);
    setForcedPitch(0);
  }

  void setArp()
  {
    name = F("ARP");
    setAttackDecay(0, 1, true);
    setDefaultVolumePwm(4, 0, false);
    setFlags(0, INSTR_FLAG_DO_RETRIG);
    setForcedPitch(0);
  }

  void setLead()
  {
    name = F("LEAD");
    setAttackDecay(0, 1, false);
    setDefaultVolumePwm(4, 0, true);
    setFlags(0, INSTR_FLAG_DO_PWM);
    setForcedPitch(0);
  }

  void setBassDrum()
  {
    name = F("B.DR");
    setAttackDecay(0, 2, true);
    setDefaultVolumePwm(4, 0, false);
    setFlags(7, INSTR_FLAG_DO_RETRIG);
    setForcedPitch(48);
  }

  void setSnareDrum()
  {
    name = F("S.DR");
    setAttackDecay(0, 1, true);
    setDefaultVolumePwm(5, 1, false);
    setFlags(4, INSTR_FLAG_IS_NOISE | INSTR_FLAG_DO_RETRIG);
    setForcedPitch(96);
  }

  void setHihat()
  {
    name = F("HHAT");
    setAttackDecay(0, 0, true);
    setDefaultVolumePwm(1, 0, false);
    setFlags(0, INSTR_FLAG_IS_NOISE | INSTR_FLAG_DO_RETRIG);
    setForcedPitch(110);
  }

  void setToms()
  {
    name = F("TOMS");
    setAttackDecay(0, 1, true);
    setDefaultVolumePwm(4, 0, false);
    setFlags(4, INSTR_FLAG_DO_RETRIG);
    setForcedPitch(0);
  }
};

volatile INSTRUMENT instruments[8];

////////////////////////////////////////////////////////////////
class CVoice
{
public:
  ENVELOPE volume_envelope;
  unsigned int target_period;
  unsigned char forced_pitch = 0;  // Forced MIDI node, no matter what; 0 --> follow target_pitch

  bool do_retrig = false;
  bool do_ramp = false;
  bool do_vibrato = false;
  bool do_delayed_vibrato = false;
  bool do_pwm = true;
  bool do_noise = false;
  unsigned char drop_speed = 0; // Actual, period-change-per-frame, pitch changing downwards.
  unsigned char pwm_factor = 0; // Default PWM factor. 0 = half-cycle, 1 = 1/4th cycle, 2 == 1/8th cycle

  signed char current_volume_decrement = 0;
  unsigned char ramp_counter = 0;

  unsigned int current_period;
  signed char current_volume = 0;
  unsigned int A, B; // square wave length
  unsigned int counter = 0;
  unsigned char phase = 0; // square wave cycle (0 or 1)
  unsigned char level = 0; // Low or high

  byte midi_note = 0;

  unsigned int age = 0;

  bool mark_gate_on;
  bool mark_gate_off;

public:
  CVoice(int v)
  {
    volume_envelope.setup(0, 3, 0, 4, true);
    if (v == 2) do_noise = true;
  }

  void setInstrument(volatile INSTRUMENT &i)
  {
    volume_envelope.setup(0, i.getDefaultVolume(), i.getAtack(), i.getDecay(), i.isPercussive());
    do_noise = i.doNoise();
    do_pwm = i.doPwm();
    do_retrig = i.doRetrig();
    do_ramp = i.doRamp();
    do_vibrato = i.doVibrato();
    pwm_factor = i.getPwmFactor();
    drop_speed = i.getDropSpeed();
    forced_pitch = i.getForcedPitch();
    do_delayed_vibrato = i.doDelayedVibrato();
  }

  unsigned char onSample()
  {
    if ((!volume_envelope.isPlaying()) || A == 0 || B == 0)
    {
      return 0;
    }

    // End of the first part of the duty cycle
    if (phase == 0 && counter >= A)
    {
      phase = 1;
      counter = 0;
      if (!do_noise || (getRandom8())) level = !level;
      // level = !level;
    }
    // End of the second part of the duty cycle
    else if (phase == 1 && counter >= B)
    {
      phase = 0;
      counter = 0;
      if (!do_noise || (getRandom8())) level = !level;
      // level = !level;
    }

    counter++;

    return level ? current_volume : 0;
  }

  void setPitch(unsigned char note, byte unison)
  {
    target_period = getPitch(forced_pitch ? forced_pitch : note);
    if (unison == 0) target_period += 1;
    if (unison == 2) target_period -= 1;
    midi_note = note;
  }

  void setRuntimeParams(signed char volume_decrement, bool do_vibrato_, bool do_ramp_)
  {
    current_volume_decrement = volume_decrement;
    if (do_vibrato_) do_vibrato = true;
    if (do_ramp_) do_ramp = true;
  }

  void gateOn()
  {
    if (!volume_envelope.isPlaying())
    {
      current_period = target_period;
    }
    mark_gate_on = true;
    age = 0;
  }

  bool isPlaying()
  {
    return mark_gate_on || volume_envelope.isPlaying();
  }

  void gateOff()
  {
    mark_gate_off = true;
  }

  void panic()
  {
    volume_envelope.panic();
  }

  void onTick()
  {
    if (mark_gate_on)
    {
      mark_gate_on = false;
      volume_envelope.gateOn(do_retrig);
    }

    if (mark_gate_off)
    {
      mark_gate_off = false;
      volume_envelope.gateOff();
    }

    if (volume_envelope.isPlaying())
    {
      current_period = target_period;
      A = 0;
    }
    else
    {
      age++;
      if (do_ramp)
      {
        if (target_period < current_period)
        {
          current_period -= 1;
        }
        else if (target_period > current_period)
        {
          current_period += 1;
        }
      }
      else
      {
        current_period = target_period;
      }

      if (do_delayed_vibrato && (age > 32))
      {
        do_vibrato = true;
      }
    }

    // Pitch drop
    target_period += drop_speed;

    A = ((current_period + (do_vibrato ? vibrato.getVal() : 0)) >> (pwm_factor + 1)) + (do_pwm ? pwm.getVal() : 0);
    B = (current_period + (do_vibrato ? vibrato.getVal() : 0)) - A;

    volume_envelope.update();
    current_volume = volume_envelope.val - current_volume_decrement;
    if (current_volume < 0) current_volume = 0;
  }
};

class CSynthesizer
{
public:
  unsigned long ticks_served = 0;
  unsigned long samples_served = 0;

private:
  unsigned int samples_since_tick = 0;
  unsigned char next_value = 0;

  bool last1;
  bool last2;
  bool last3;
  bool last4;
  bool now;

public:
  CVoice op1;
  CVoice op2;
  CVoice op3;
  CVoice* voices[3];
  byte last_keys[8];

public:
  CSynthesizer() : op1(0), op2(1), op3(2)
  {
    vibrato.setup(-2, 2, 1);
    pwm.setup(-20, 20, 1);

    instruments[0].setBass();
    instruments[1].setSquare();
    instruments[2].setArp();
    instruments[3].setLead();
    instruments[4].setBassDrum();
    instruments[5].setSnareDrum();
    instruments[6].setHihat();
    instruments[7].setToms();

    voices[0] = &op1;
    voices[1] = &op2;
    voices[2] = &op3;

    for (int a = 0; a < 8; a++)
    {
      last_keys[a] = 0;
    }
  }

  void startNote(byte op, byte midi_note, volatile INSTRUMENT &i, signed char volume_decrement, bool do_vibrato, bool do_ramp, byte unison = 0xff)
  {
    voices[op]->setInstrument(i);
    voices[op]->setRuntimeParams(volume_decrement, do_vibrato, do_ramp);
    voices[op]->setPitch(midi_note, unison);
    voices[op]->gateOn();
  }

  void onMidiNoteOn(byte channel, byte midi_note, byte velocity)
  {
    if (mono_voice == 0xff)
    {
      bool found = false;
      unsigned int highest_age = 0;
      byte oldest_voice = 0xff;
      for (int a = 0; a < 3; a++)
      {
        if (!voices[a]->isPlaying())
        {
          // Start playing here
          startNote(a, midi_note, instruments[channel & 0x7], 0, 0, 0);
          found = true;
          break;
        }
        else
        {
          if (voices[a]->age > highest_age)
          {
            highest_age = voices[a]->age;
            oldest_voice = a;
          }
        }
      }
      if (!found)
      {
        startNote(oldest_voice, midi_note, instruments[channel & 0x7], 0, 0, 0);
      }
    }
    else
    {
      if (mono_voice == 0x7f)
      {
        for (byte a = 0; a < 3; a++)
        {
          startNote(a, midi_note, instruments[channel & 0x7], 0, 0, 0, a);
        }
      }
      else
      {
        startNote(mono_voice, midi_note, instruments[channel & 0x7], 0, 0, 0);
      }
      bool found = false;
      for (byte a = 0; a < 8; a++)
      {
        if (last_keys[a] == 0)
        {
          last_keys[a] = midi_note;
          found = true;
          break;
        }
      }
      if (!found)
      {
        last_keys[0] = midi_note;
      }
    }
  }

  void onMidiNoteOff(byte channel, byte midi_note, byte velocity)
  {
    if (mono_voice == 0xff)
    {
      for (int a=0; a<3; a++)
      {
        if (voices[a]->volume_envelope.isPlaying())
        {
          if (voices[a]->midi_note == midi_note)
          {
            endNote(a);
            break;
          }
        }
      }
    }
    else
    {
      byte highest = 0;
      byte stop = 0;
      for (unsigned char b = 0; b < 8; b++)
      {
        if (last_keys[b] == midi_note)
        {
          last_keys[b] = 0;
          stop = midi_note;
        }
        else if (last_keys[b] > highest)
        {
          highest = last_keys[b];          
        }
      }
      if (highest)
      {
        if (mono_voice == 0x7f)
        {
          for (byte a = 0; a < 3; a++)
          {
            startNote(a, highest, instruments[channel & 0x7], 0, 0, 0, a);
          }
        }
        else
        {
          startNote(mono_voice, highest, instruments[channel & 0x7], 0, 0, 0);
        }
      }
      else if (stop)
      {
        if (mono_voice == 0x7f)
        {
          for (byte a = 0; a < 3; a++)
          {
            endNote(a);
          }
        }
        else
        {
          endNote(mono_voice);
        }
      }
    }
  }

  void setRuntimeParams(byte op, signed char volume_decrement, bool do_vibrato, bool do_ramp)
  {
    voices[op]->setRuntimeParams(volume_decrement, do_vibrato, do_ramp);
  }

  void endNote(byte op)
  {
    voices[op]->gateOff();
  }

  void panic()
  {
    voices[0]->panic();
    voices[1]->panic();
    voices[2]->panic();

    for (int a = 0; a < 8; a++)
    {
      last_keys[a] = 0;
    }
  }

  byte onSample()
  {
    byte update_thread = 0;
    samples_served++;

    // Write thre _previous_ values, so that this happens as the first operation of the interrupt handler.
    // Also, write only if there is a change.
    now = next_value & 1; if ((last1 & !now) | (!last1 & now)) { digitalWrite(10, now); last1 = now; }
    now = next_value & 2; if ((last2 & !now) | (!last2 & now)) { digitalWrite(11, now); last2 = now; }
    now = next_value & 4; if ((last3 & !now) | (!last3 & now)) { digitalWrite(12, now); last3 = now; }
    now = next_value & 8; if ((last4 & !now) | (!last4 & now)) { digitalWrite(13, now); last4 = now; }

    if (midi_note_in)
    {
      onMidiNoteOn(midi_note_in_channel, midi_note_in, 127);
      midi_note_in = 0;
    }

    if (midi_note_kill)
    {
      onMidiNoteOff(midi_note_kill_channel, midi_note_kill, 127);
      midi_note_kill = 0;
    }

    if (samples_since_tick == 640)
    {
      onTick();
      samples_since_tick = 0;
    }

    if (samples_since_tick == 320)
    {
      update_thread = 1;
    }

    unsigned char out = 0;
    out += op1.onSample();
    out += op2.onSample();
    out += op3.onSample();
    next_value = out;

    // Get sample
    // Calculate next value, store to `next_value`

    samples_since_tick++;
    return update_thread;
  }
  
  void onTick()
  {
    ticks_served++;

    // Update modulators
    vibrato.update();
    pwm.update();

    // Update envelopes in voices
    op1.onTick();
    op2.onTick();
    op3.onTick();
  }
};

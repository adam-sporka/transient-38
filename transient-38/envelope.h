#define ENV_PHASE_OUTSIDE 0
#define ENV_PHASE_ATTACK 1
#define ENV_PHASE_DECAY 2
#define ENV_PHASE_HOLD 3

struct ENVELOPE
{
  bool oneshot;
  signed char min;
  signed char max;
  unsigned char attack_rate; // How many ticks between advancing from min to max
  unsigned char decay_rate; // How many ticks between advancing from max to min
  signed char val = 0;
  unsigned char counter = 0;
  unsigned char phase = ENV_PHASE_OUTSIDE;

  void setup(signed char min_, signed char max_, unsigned char attack_rate_, unsigned char decay_rate_, bool oneshot_)
  {
    min = min_;
    max = max_;
    attack_rate = attack_rate_;
    decay_rate = decay_rate_;
    oneshot = oneshot_;
  }

  ARDUINO_INLINE bool isPlaying()
  {
    return phase != ENV_PHASE_OUTSIDE;
  }

  ARDUINO_INLINE void gateOn(bool do_retrig)
  {
    if (do_retrig || ((!do_retrig) && (phase != ENV_PHASE_HOLD)))
    {
      phase = ENV_PHASE_ATTACK;
      counter = 0;
      val = min;
    }
  }

  ARDUINO_INLINE void gateOff()
  {
    phase = ENV_PHASE_DECAY;
    counter = 0;
  }

  ARDUINO_INLINE void kill()
  {
    phase = ENV_PHASE_OUTSIDE;
    val = min;
    counter = 0;
  }

  ARDUINO_INLINE void panic()
  {
    phase = ENV_PHASE_OUTSIDE;
    val = min;
    counter = 0;
  }

  ARDUINO_INLINE void update()
  {
    if (phase == ENV_PHASE_OUTSIDE)
    {
      val = min;
      return;
    }

    if (phase == ENV_PHASE_HOLD)
    {
      val = max;
      return;
    }

    if (phase == ENV_PHASE_ATTACK)
    {
      // zero ticks between advancements --> jump to decay
      if (attack_rate == 0)
      {
        if (oneshot) phase = ENV_PHASE_DECAY;
        else phase = ENV_PHASE_HOLD;
        val = max;
        counter = 0;
      }
      else if (counter >= attack_rate)
      {
        val++;
        if (val >= max)
        {
          if (oneshot) phase = ENV_PHASE_DECAY;
          else phase = ENV_PHASE_HOLD;
        }
        counter = 0;
      }
      else
      {
        counter++;
      }
    }

    else if (phase == ENV_PHASE_DECAY)
    {
      if (decay_rate == 0)
      {
        phase = ENV_PHASE_OUTSIDE;
        val = min;
        counter = 0;
      }
      else if (counter >= decay_rate)
      {
        val--;
        if (val <= min)
        {
          phase = ENV_PHASE_OUTSIDE;
        }
        counter = 0;
      }
      else
      {
        counter++;
      }
    }
  }
};


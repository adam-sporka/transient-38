struct LFO
{
  signed char min;
  signed char max;
  signed char speed; // How many ticks between updates
  signed char current_val = 0;
  signed char counter = 0; // How many ticks since the last update
  signed char direction = 1; // The next move

  void setup(signed char min_, signed char max_, signed char speed_)
  {
    min = min_;
    max = max_;
    speed = speed_;
    current_val = min_;
  }

  void update()
  {
    counter++;
    if (counter < speed)
    {
      return;
    }

    counter = 0;
    if (direction > 0)
    {
      if (current_val < max)
      {
        current_val++;
      }
      else
      {
        direction = -1;
        current_val--;
      }
    }
    else if (direction < 0)
    {
      if (current_val > min)
      {
        current_val--;
      }
      else
      {
        direction = 1;
        current_val++;
      }
    }
  }

  signed char getVal()
  {
    return current_val;
  }
};


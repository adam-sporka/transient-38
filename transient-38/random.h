#define RANDOM_BYTES 4

//const uint8_t random_bits[32] = { 145, 14, 76, 178, 19, 212, 214, 52, 124, 123, 93, 132, 141, 231, 140, 150, 40, 149, 187, 121, 191, 204, 138, 193, 85, 159, 66, 254, 161, 199, 2, 98 };
const uint8_t random_bits[4] = { 145, 14, 13, 178 };
uint8_t random_pos_byte = 0;
uint8_t random_pos_bit = 1;
uint8_t random_byte = 145;

ARDUINO_INLINE uint8_t getRandom8()
{
  if (random_pos_bit != 128) random_pos_bit = random_pos_bit << 1;
  else
  {
    random_pos_bit = 1;
    if (random_pos_byte < (RANDOM_BYTES - 1)) random_pos_byte++;
    else random_pos_byte = 0;
    random_byte = random_bits[random_pos_byte];
  }
  return random_byte & random_pos_bit;
}

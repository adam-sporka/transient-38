const PROGMEM byte bitmap[40] =
 { 0b00100, 0b01110, 0b00000, 0b01111, 0b10000, 0b10000, 0b01111, 0b00000,
   0b00100, 0b01110, 0b00000, 0b11110, 0b10001, 0b10001, 0b11110, 0b00000,
   0b00100, 0b01110, 0b00000, 0b11111, 0b10000, 0b11110, 0b10000, 0b00000,
   0b00100, 0b01110, 0b00000, 0b01111, 0b10000, 0b10001, 0b01111, 0b00000,
   0b00100, 0b01110, 0b00000, 0b01110, 0b10001, 0b11111, 0b10001, 0b00000 };

void init_charset()
{
  byte buffer[8];
  byte* b = bitmap;
  for (int c = 0; c < 5; c++)
  {
    for (int a = 0; a < 8; a++)
    {
      buffer[a] = pgm_read_byte(b++);
    }
    lcd.createChar(c, buffer); 
  }
}

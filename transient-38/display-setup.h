void init_charset()
{
  byte Csharp[8] = { 0b00100, 0b01110, 0b00000, 0b01111, 0b10000, 0b10000, 0b01111, 0b00000 };
  byte Dsharp[8] = { 0b00100, 0b01110, 0b00000, 0b11110, 0b10001, 0b10001, 0b11110, 0b00000 };
  byte Fsharp[8] = { 0b00100, 0b01110, 0b00000, 0b11111, 0b10000, 0b11110, 0b10000, 0b00000 };
  byte Gsharp[8] = { 0b00100, 0b01110, 0b00000, 0b01111, 0b10000, 0b10001, 0b01111, 0b00000 };
  byte Asharp[8] = { 0b00100, 0b01110, 0b00000, 0b01110, 0b10001, 0b11111, 0b10001, 0b00000 };
  
  lcd.createChar(0, Csharp); 
  lcd.createChar(1, Dsharp); 
  lcd.createChar(2, Fsharp); 
  lcd.createChar(3, Gsharp); 
  lcd.createChar(4, Asharp); 
}

void writeEeprom()
{
  uint8_t checksum = 0;


  for (uint8_t i = 0; i < MODULE_NUM; i++)
  {
    EEPROM.write(i * 4, highByte(MAX_RSSI[i]));
    EEPROM.write(i * 4 + 1, lowByte(MAX_RSSI[i]));
    EEPROM.write(i * 4 + 2, highByte(MIN_RSSI[i]));
    EEPROM.write(i * 4 + 3, lowByte(MIN_RSSI[i]));
  }

  EEPROM.write(32, highByte(ACTUAL_FREQ));
  EEPROM.write(33, lowByte(ACTUAL_FREQ));

  for (uint8_t i = 0; i <= 33; i++) checksum = checksum + EEPROM.read(i); //XOR CHECKSUM

  EEPROM.write(34, checksum);
}


//EEPROMから
uint8_t readEeprom()
{
  uint8_t temp[35];
  uint8_t checksum = 0;
  for (uint8_t i = 0; i <= 33; i++)
  {
    temp[i] = EEPROM.read(i);
    checksum = checksum + EEPROM.read(i);
  }
  if (checksum == EEPROM.read(34))
  {

    for (uint8_t i = 0; i < MODULE_NUM; i++)
    {
      MAX_RSSI[i] = (temp[i * 4] << 8) + temp[i * 4 + 1];
      MIN_RSSI[i] = (temp[i * 4 + 2] << 8) + temp[i * 4 + 3];
    }

    ACTUAL_FREQ = (temp[32] << 8) + temp[33];
  }
  else return (0);
}

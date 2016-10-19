void writeEeprom()
{
  uint8_t checksum = 0;

  for (uint8_t i = 0; i < 67; i++) EEPROM.write(i ,0);


  for (uint8_t i = 0; i < MODULE_NUM; i++)
  {
    EEPROM.write(i * 4, highByte(MAX_RSSI[i]));
    EEPROM.write(i * 4 + 1, lowByte(MAX_RSSI[i]));
    EEPROM.write(i * 4 + 2, highByte(MIN_RSSI[i]));
    EEPROM.write(i * 4 + 3, lowByte(MIN_RSSI[i]));
  }

  EEPROM.write(MODULE_NUM * 4, highByte(ACTUAL_CHANNEL));
  EEPROM.write(MODULE_NUM* 4 + 1, lowByte(ACTUAL_CHANNEL));

  for (uint8_t i = 0; i <= MODULE_NUM * 4 + 1; i++) checksum = checksum + EEPROM.read(i); //XOR CHECKSUM

  EEPROM.write(MODULE_NUM * 4 + 2, checksum);
}


//EEPROMから
uint8_t readEeprom()
{
  uint8_t temp[67];
  uint8_t checksum = 0;
  for (uint8_t i = 0; i <= MODULE_NUM * 4 + 1; i++)
  {
    temp[i] = EEPROM.read(i);
    checksum = checksum + EEPROM.read(i);
  }
  if (checksum == EEPROM.read(MODULE_NUM * 4 + 2))
  {

    for (uint8_t i = 0; i < MODULE_NUM; i++)
    {
      MAX_RSSI[i] = (temp[i * 4] << 8) + temp[i * 4 + 1];
      MIN_RSSI[i] = (temp[i * 4 + 2] << 8) + temp[i * 4 + 3];
    }

    ACTUAL_CHANNEL = (temp[MODULE_NUM * 4] << 8) + temp[MODULE_NUM * 4 + 1];
  }
  else return (0);
}

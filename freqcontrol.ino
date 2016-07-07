
void nextFreq()
{
  uint8_t channelOrder;
  channelOrder = getSortedChannelOrder();

  if (channelOrder == 39) channelOrder = 0;
  else channelOrder++;
  
  ACTUAL_FREQ = SORTED_CHANNEL_FREQ_TABLE[channelOrder];
  ACTUAL_CHANNEL = SORTED_BAND_CHANNEL_TABLE[channelOrder];
  printFreq();
  setChannel(); 
}

void prevFreq()
{
  uint8_t channelOrder;
  channelOrder = getSortedChannelOrder();

  if (channelOrder == 0) channelOrder = 39;
  else channelOrder--;
  
  ACTUAL_FREQ = SORTED_CHANNEL_FREQ_TABLE[channelOrder];
  ACTUAL_CHANNEL = SORTED_BAND_CHANNEL_TABLE[channelOrder];
  printFreq();
  setChannel(); 
}

void nextBand()
{
  uint8_t channelOrder;
  channelOrder = getChannelOrder();

  if (channelOrder > 31) channelOrder = channelOrder - 32;
  else channelOrder = channelOrder + 8;
  
  ACTUAL_FREQ = CHANNEL_FREQ_TABLE[channelOrder];
  ACTUAL_CHANNEL = BAND_CHANNEL_TABLE[channelOrder];
  printFreq();
  setChannel(); 
}

void nextChannel()
{
  uint8_t channelOrder;
  channelOrder = getChannelOrder();
  
  if ((channelOrder+1)%8 == 0) channelOrder = channelOrder - 7;
  else channelOrder++;
  ACTUAL_FREQ = CHANNEL_FREQ_TABLE[channelOrder];
  ACTUAL_CHANNEL = BAND_CHANNEL_TABLE[channelOrder];
  printFreq();
  setChannel();  
}

void setChannel()
{
  for (uint8_t i = 0; i < MODULE_NUM; i++)
  {
    setChannelModule(i, ACTUAL_FREQ);
    delay(10);
  }
}

uint8_t getChannelOrder()
{
  for (uint8_t i = 0; i < 40; i++)
  {
    if (ACTUAL_CHANNEL == BAND_CHANNEL_TABLE[i])
    {
      return (i);
      break;
    }
  }
}

uint8_t getSortedChannelOrder()
{
  for (uint8_t i = 0; i < 40; i++)
  {
    if (ACTUAL_CHANNEL == SORTED_BAND_CHANNEL_TABLE[i])
    {
      return (i);
      break;
    }
  }
}


uint16_t getFreq(uint16_t bandChannel)
{
  for (uint8_t i = 0; i < 40; i++)
  {
    if (BAND_CHANNEL_TABLE[i] == bandChannel)
    {
      return (CHANNEL_FREQ_TABLE[i]);
      break;
    }
  }
}

void sortChannel()
{
  uint16_t temp;
  for (uint8_t i = 0; i < 40 ; i++) 
  {
    SORTED_CHANNEL_FREQ_TABLE[i] = CHANNEL_FREQ_TABLE[i];
    SORTED_BAND_CHANNEL_TABLE[i] = BAND_CHANNEL_TABLE[i];
  }

  for (uint8_t i = 0; i < 39 ; i++)
  {
    for (uint8_t j = 0; j < 39 - i ; j++)
    {
      if (SORTED_CHANNEL_FREQ_TABLE[j] > SORTED_CHANNEL_FREQ_TABLE[j + 1])
      {
        temp =  SORTED_CHANNEL_FREQ_TABLE[j + 1];
        SORTED_CHANNEL_FREQ_TABLE[j + 1] = SORTED_CHANNEL_FREQ_TABLE[j];
        SORTED_CHANNEL_FREQ_TABLE[j] = temp;
        
        temp =  SORTED_BAND_CHANNEL_TABLE[j + 1];
        SORTED_BAND_CHANNEL_TABLE[j + 1] = SORTED_BAND_CHANNEL_TABLE[j];
        SORTED_BAND_CHANNEL_TABLE[j] = temp;
      }
    }
  }
}


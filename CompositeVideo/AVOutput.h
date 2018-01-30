#pragma once
#include "driver/i2s.h"


template<class Graphics>
class AVOutput
{
  static const int BLANK = 23;
  static const int BLACK = 26;
  static const int SYNC = 0;
  static const int FRAME_LINES = 200;//16;
  static const int PRE_BLANK_LINES = 15;
  static const int POST_BLANK_LINES = 0;
  static const int VISIBLE_LINES = 290;
  static const int PRE_FRAME_LINES = (VISIBLE_LINES - FRAME_LINES) / 2;
  static const int POST_FRAME_LINES = VISIBLE_LINES - FRAME_LINES - PRE_FRAME_LINES;
    
  unsigned short row[854];
  static const i2s_port_t I2S_PORT = (i2s_port_t)I2S_NUM_0;
  public:

  void fillValues(int &i, unsigned char value, int count)
  {
    for(int j = 0; j < count; j++)
    {
      row[i^1] = value << 8;
      i++;
    }
  }
  
  AVOutput()
  {
  }

  void init()
  {
    Serial.println("init");
    static i2s_config_t i2s_config = {
       .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),
       .sample_rate = 1000000,  //not really used
       .bits_per_sample = (i2s_bits_per_sample_t)I2S_BITS_PER_SAMPLE_16BIT, 
       .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
       .communication_format = I2S_COMM_FORMAT_I2S_MSB,
       .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
       .dma_buf_count = 3,
       .dma_buf_len = 854  //a buffer per line
    };
    
    i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);    //start i2s driver
    i2s_set_pin(I2S_PORT, NULL);                           //use internal DAC
    i2s_set_sample_rates(I2S_PORT, 1000000);               //dummy sample rate, since the function fails at high values
  
    //this is the hack that enables the highest sampling rate possible ~13MHz, have fun
    SET_PERI_REG_BITS(I2S_CLKM_CONF_REG(0), I2S_CLKM_DIV_A_V, 1, I2S_CLKM_DIV_A_S);
    SET_PERI_REG_BITS(I2S_CLKM_CONF_REG(0), I2S_CLKM_DIV_B_V, 1, I2S_CLKM_DIV_B_S);
    SET_PERI_REG_BITS(I2S_CLKM_CONF_REG(0), I2S_CLKM_DIV_NUM_V, 2, I2S_CLKM_DIV_NUM_S); 
    SET_PERI_REG_BITS(I2S_SAMPLE_RATE_CONF_REG(0), I2S_TX_BCK_DIV_NUM_V, 2, I2S_TX_BCK_DIV_NUM_S);     
  }

  void sendRow()
  {
    i2s_write_bytes(I2S_PORT, (char*)row, 854 * 2/*sizeof(row)*/, portMAX_DELAY);
  }

  void fillRow(char *pixels)
  {
    int i = 0;
    fillValues(i, SYNC, 53);
    fillValues(i, BLANK, 109);
    for(int x = 0; x < 640; x++)
    {
      row[i^1] = (BLACK + pixels[x >> 1]) << 8;
      i++;
    }
    fillValues(i, BLANK, 52);
  }

  void fillLong(int &i)
  {
    fillValues(i, SYNC, 854 / 2 - 27);
    fillValues(i, BLANK, 27);
  }
  
  void fillShort(int &i)
  {
    fillValues(i, SYNC, 27);
    fillValues(i, BLANK, 854 / 2 - 27);  
  }
  
  void fillBlank()
  {
    int i = 0;
    fillValues(i, SYNC, 53);
    fillValues(i, BLANK, 854 - 53);  
  }

  void fillHalfBlank()
  {
    int i = 0;
    fillValues(i, SYNC, 53);
    fillValues(i, BLANK, 854 / 2 - 53);  
  }
  
  void sendFrame(Graphics &graphics, bool odd)
  {
    int i = 0;
    fillLong(i); fillLong(i);
    sendRow(); sendRow();
    i = 0;
    fillLong(i); fillShort(i);
    sendRow();
    i = 0;
    fillShort(i); fillShort(i);
    sendRow(); sendRow();
    fillBlank();
    for(int y = 0; y < PRE_BLANK_LINES + PRE_FRAME_LINES; y++)
      sendRow();
    for(int y = 0; y < FRAME_LINES; y++)
    {
      char *pixels = graphics.frame[y];
      fillRow(pixels);
      sendRow();
    }
    fillBlank();
    for(int y = 0; y < POST_BLANK_LINES + POST_FRAME_LINES; y++)
      sendRow();
    i = 0;
    fillShort(i); fillShort(i);
    sendRow(); sendRow();
    i = 0;
    fillShort(i); fillLong(i);
    sendRow(); 
    i = 0;
    fillLong(i); fillLong(i);
    sendRow(); sendRow();
    i = 0;
    fillShort(i); fillShort(i);
    sendRow(); sendRow();
    fillBlank();
    for(int y = 0; y < PRE_BLANK_LINES + PRE_FRAME_LINES; y++)
      sendRow();
    for(int y = 0; y < FRAME_LINES; y++)
    {
      char *pixels = graphics.frame[y];
      fillRow(pixels);
      sendRow();
    }
    fillBlank();
    for(int y = 0; y < POST_BLANK_LINES + POST_FRAME_LINES; y++)
      sendRow();
    i = 0;
    fillHalfBlank(); fillShort(i);
    sendRow(); 
    i = 0;
    fillShort(i); fillShort(i);
    sendRow(); sendRow();
  }
};



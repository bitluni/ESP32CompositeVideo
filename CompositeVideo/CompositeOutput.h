#pragma once
#include "driver/i2s.h"

typedef struct
{
  float lineMicros;
  float hsyncMicros;
  float backPorchMicros;
  float frontPorchMicros;
  float shortSyncMicros;
  float broadSyncMicros;
  float overscanLeftMicros; 
  float overscanRightMicros; 
  double syncVolts; 
  double blankVolts; 
  double blackVolts;
  double whiteVolts;
  short lines;
  short linesFirstTop;
  short linesOverscanTop;
  short linesOverscanBottom;
  float imageAspect;
}TechProperties;

// Levels: see https://www.maximintegrated.com/en/design/technical-documents/tutorials/7/734.html
// Used for sync, blank, black, white levels.
// 1Vp-p = 140*IRE
const double IRE = 1.0 / 140.0;

// Timings: http://www.batsocks.co.uk/readme/video_timing.htm
// http://martin.hinner.info/vga/pal.html
// Also interesting: https://wiki.nesdev.com/w/index.php/NTSC_video

const TechProperties PALProperties = {
  .lineMicros = 64,
  .hsyncMicros = 4.7,
  .backPorchMicros = 10.4,
  .frontPorchMicros = 1.65,
  .shortSyncMicros = 2.35,
  .broadSyncMicros = (64 / 2) - 4.7,
  .overscanLeftMicros = 1.6875,
  .overscanRightMicros = 1.6875,
  .syncVolts = -0.3,
  .blankVolts = 0.0, 
  .blackVolts =  0.005,//specs 0.0,
  .whiteVolts = 0.7,
  .lines = 625,
  .linesFirstTop = 23,
  .linesOverscanTop = 9,
  .linesOverscanBottom = 9,
  .imageAspect = 4./3.
};

const TechProperties NTSCProperties = {
  // Duration of a line
  .lineMicros = 63.492,
  // HSync
  .hsyncMicros = 4.7,
  .backPorchMicros = 4.5,
  // Front porch
  .frontPorchMicros = 1.5,
  // Short sync pulse
  .shortSyncMicros = 2.35, // TO REMOVE
  // Broad sync pulse
  .broadSyncMicros = (63.492 / 2) - 4.7, // TO REMOVE
  .overscanLeftMicros = 0,//1.3, // TO REMOVE
  .overscanRightMicros = 0,//1, // TO REMOVE
  .syncVolts = -40.0 * IRE,
  .blankVolts = 0.0 * IRE,
  .blackVolts = 7.5 * IRE,
  .whiteVolts = 100.0 * IRE,
  .lines = 525,
  .linesFirstTop = 20, // TO REMOVE
  .linesOverscanTop = 6, // TO REMOVE
  .linesOverscanBottom = 9, // TO REMOVE
  .imageAspect = 4./3.
};

class CompositeOutput
{
  public:
  int samplesLine;
  int samplesHSync;
  int samplesBackPorch;
  int samplesFrontPorch;
  int samplesActive;
  int samplesBlackLeft;
  int samplesBlackRight;

  int samplesShortSync;
  int samplesBroadSync;

  char levelSync;
  char levelBlank;
  char levelBlack;
  char levelWhite;
  char grayValues;

  int targetXres;
  int targetYres;
  int targetYresEven;
  int targetYresOdd;

  int linesEven;
  int linesOdd;
  int linesEvenActive;
  int linesOddActive;
  int linesEvenVisible;
  int linesOddVisible;
  int linesEvenBlankTop;
  int linesEvenBlankBottom;
  int linesOddBlankTop;
  int linesOddBlankBottom;

  float pixelAspect;
    
  unsigned short *line;

  static const i2s_port_t I2S_PORT = (i2s_port_t)I2S_NUM_0;
    
  enum Mode
  {
    PAL,
    NTSC  
  };
  
  const TechProperties &properties;
  
  CompositeOutput(Mode mode, int xres, int yres, double Vcc = 3.3)
    :properties((mode==NTSC) ? NTSCProperties: PALProperties)
  {
    // TO REMOVE - start

    int linesSyncTop = 5;
    int linesSyncBottom = 3;

    linesOdd = properties.lines / 2;
    linesEven = properties.lines - linesOdd;
    
    linesEvenActive = linesEven - properties.linesFirstTop - linesSyncBottom;
    
    linesOddActive = linesOdd - properties.linesFirstTop - linesSyncBottom;
    
    linesEvenVisible = linesEvenActive - properties.linesOverscanTop - properties.linesOverscanBottom; 
    linesOddVisible = linesOddActive - properties.linesOverscanTop - properties.linesOverscanBottom;

    targetYresOdd = (yres / 2 < linesOddVisible) ? yres / 2 : linesOddVisible;
    targetYresEven = (yres - targetYresOdd < linesEvenVisible) ? yres - targetYresOdd : linesEvenVisible;
    // TO REMOVE
    targetYres = targetYresEven + targetYresOdd;
    
    // TO REMOVE
    linesEvenBlankTop = properties.linesFirstTop - linesSyncTop + properties.linesOverscanTop + (linesEvenVisible - targetYresEven) / 2;
    // TO REMOVE
    linesEvenBlankBottom = linesEven - linesEvenBlankTop - targetYresEven - linesSyncBottom;
    // TO REMOVE
    linesOddBlankTop = linesEvenBlankTop;
    // TO REMOVE
    linesOddBlankBottom = linesOdd - linesOddBlankTop - targetYresOdd - linesSyncBottom;
    // TO REMOVE - ends
    
    double samplesPerMicro = 160.0 / 3.0 / 2.0 / 2.0;
    samplesLine = (int)(samplesPerMicro * properties.lineMicros + 1.5) & ~1;
    //samplesLine = samplesPerMicro * properties.lineMicros + 0.5;
    // 4.7 µS HSync
    samplesHSync = samplesPerMicro * properties.hsyncMicros + 0.5;
    // Back Porch
    samplesBackPorch = samplesPerMicro * properties.backPorchMicros + 0.5;
    // Front Porch
    samplesFrontPorch = samplesPerMicro * properties.frontPorchMicros + 0.5;
    // Picture Data
    samplesActive = samplesLine - samplesHSync - samplesBackPorch - samplesFrontPorch;

    samplesShortSync = samplesPerMicro * properties.shortSyncMicros + 0.5;

    samplesBroadSync = samplesPerMicro * properties.broadSyncMicros + 0.5;

    targetXres = xres < samplesActive ? xres : samplesActive;
    samplesBlackLeft = (samplesActive - targetXres) / 2;
    samplesBlackRight = samplesActive - targetXres - samplesBlackLeft;

    double dacPerVolt = 255.0 / Vcc;
    levelSync = 0;
    levelBlank = (properties.blankVolts - properties.syncVolts) * dacPerVolt + 0.5;
    levelBlack = (properties.blackVolts - properties.syncVolts) * dacPerVolt + 0.5;
    levelWhite = (properties.whiteVolts - properties.syncVolts) * dacPerVolt + 0.5;
    grayValues = levelWhite - levelBlack + 1;

    pixelAspect = (float(samplesActive) / (linesEvenVisible + linesOddVisible)) / properties.imageAspect;
  }

  void init()
  {
    line = (unsigned short*)malloc(sizeof(unsigned short) * samplesLine);
    i2s_config_t i2s_config = {
       .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),
       .sample_rate = 1000000,  //not really used
       .bits_per_sample = (i2s_bits_per_sample_t)I2S_BITS_PER_SAMPLE_16BIT, 
       .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
       .communication_format = I2S_COMM_FORMAT_I2S_MSB,
       .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
       .dma_buf_count = 2,
       .dma_buf_len = samplesLine  //a buffer per line
    };
    
    i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);    //start i2s driver
    i2s_set_pin(I2S_PORT, NULL);                           //use internal DAC
    i2s_set_sample_rates(I2S_PORT, 1000000);               //dummy sample rate, since the function fails at high values
  
    //this is the hack that enables the highest sampling rate possible 13.333MHz, have fun
    SET_PERI_REG_BITS(I2S_CLKM_CONF_REG(I2S_PORT), I2S_CLKM_DIV_A_V, 1, I2S_CLKM_DIV_A_S);
    SET_PERI_REG_BITS(I2S_CLKM_CONF_REG(I2S_PORT), I2S_CLKM_DIV_B_V, 1, I2S_CLKM_DIV_B_S);
    SET_PERI_REG_BITS(I2S_CLKM_CONF_REG(I2S_PORT), I2S_CLKM_DIV_NUM_V, 2, I2S_CLKM_DIV_NUM_S); 
    SET_PERI_REG_BITS(I2S_SAMPLE_RATE_CONF_REG(I2S_PORT), I2S_TX_BCK_DIV_NUM_V, 2, I2S_TX_BCK_DIV_NUM_S);
  }

  void sendLine()
  {
    esp_err_t error = ESP_OK;
    size_t bytes_written = 0;
    size_t bytes_to_write = samplesLine * sizeof(unsigned short);
    size_t cursor = 0;
    while (error == ESP_OK && bytes_to_write > 0) {
      error = i2s_write(I2S_PORT, (const char *)line + cursor, bytes_to_write, &bytes_written, portMAX_DELAY);
      bytes_to_write -= bytes_written;
      cursor += bytes_written;
    }
  }

  inline void fillValues(int &i, unsigned char value, int count)
  {
    for(int j = 0; j < count; j++)
      line[i++^1] = value << 8;
  }

  // One scanLine
  void fillLine(char *pixels)
  {
    int i = 0;
    // HSync
    fillValues(i, levelSync, samplesHSync);
    // Back Porch
    fillValues(i, levelBlank, samplesBackPorch);
    // Black left (image centering)
    fillValues(i, levelBlack, samplesBlackLeft);
    // Picture Data
    for(int x = 0; x < targetXres / 2; x++)
    {
      short pix = (levelBlack + pixels[x]) << 8;
      line[i++^1] = pix;
      line[i++^1] = pix;
    }
    // Black right (image centering)
    fillValues(i, levelBlack, samplesBlackRight);
    // Front Porch
    fillValues(i, levelBlank, samplesFrontPorch);
  }

  // Half a line of BroadSync
  void fillBroadSync(int &i)
  {
    fillValues(i, levelSync, samplesBroadSync);
    fillValues(i, levelBlank, samplesLine / 2 - samplesBroadSync);
  }
  
  // Half a line of ShortSync
  void fillShortSync(int &i)
  {
    fillValues(i, levelSync, samplesShortSync);
    fillValues(i, levelBlank, samplesLine / 2 - samplesShortSync);  
  }
  
  // A full line of Black pixels
  void fillBlackLine()
  {
    int i = 0;
    fillValues(i, levelSync, samplesHSync);
    fillValues(i, levelBlank, samplesBackPorch);
    fillValues(i, levelBlack, samplesActive);
    fillValues(i, levelBlank, samplesFrontPorch);
  }

  void fillHalfBlank()
  {
    int i = 0;
    fillValues(i, levelSync, samplesHSync);
    fillValues(i, levelBlank, samplesLine / 2 - samplesHSync);  
  }
  
  void sendFrameHalfResolution(char ***frame)
  {
    //Even field
    // 4 long
    int i = 0;
    fillBroadSync(i); fillBroadSync(i);
    sendLine(); sendLine();
    i = 0;
    // 1 long, 1 short
    fillBroadSync(i); fillShortSync(i);
    sendLine();
    i = 0;
    // 4 short
    fillShortSync(i); fillShortSync(i);
    sendLine(); sendLine();

    // Black lines for vertical centering
    fillBlackLine();
    for(int y = 0; y < linesEvenBlankTop; y++)
      sendLine();
    // Lines (image)
    for(int y = 0; y < targetYresEven; y++)
    {
      char *pixels = (*frame)[y];
      fillLine(pixels);
      sendLine();
    }
    // Black lines for vertical centering
    fillBlackLine();
    for(int y = 0; y < linesEvenBlankBottom; y++)
      sendLine();

    // 4 short
    i = 0;
    fillShortSync(i); fillShortSync(i);
    sendLine(); sendLine();

    // 1 short, 1 long 
    //Odd field
    i = 0;
    fillShortSync(i); fillBroadSync(i);
    sendLine();

    // 4 long
    i = 0;
    fillBroadSync(i); fillBroadSync(i);
    sendLine(); sendLine();
    // 4 short
    i = 0;
    fillShortSync(i); fillShortSync(i);
    sendLine(); sendLine();
    // 1 short, half a line of blank
    i = 0;
    fillShortSync(i); fillValues(i, levelBlank, samplesLine / 2);
    sendLine();

    // Black lines for vertical centering
    fillBlackLine();
    for(int y = 0; y < linesOddBlankTop; y++)
      sendLine();
    // Lines (image)
    for(int y = 0; y < targetYresOdd; y++)
    {
      char *pixels = (*frame)[y];
      fillLine(pixels);
      sendLine();
    }
    // Black lines for vertical centering
    fillBlackLine();
    for(int y = 0; y < linesOddBlankBottom; y++)
      sendLine();
    // half a line of blank, 1 short
    i = 0;
    fillHalfBlank(); fillShortSync(i);
    sendLine(); 
    // 4 short
    i = 0;
    fillShortSync(i); fillShortSync(i);
    sendLine(); sendLine();
  }
};

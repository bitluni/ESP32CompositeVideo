//code by bitluni (send me a high five if you like the code)
#include "esp_pm.h"

//#define SKULL
//#define LOGO
#define VENUS
//#define DRAGON

#include "Matrix.h"
#include "CompositeGraphics.h"
#include "Mesh.h"
#include "Image.h"
#include "CompositeOutput.h"

#include "skull.h"
#include "venus.h"
#include "logo3d.h"
#include "dragon.h"
#include "lablogo.h"

#include "font6x8.h"

//PAL MAX, half: 324x268 full: 648x536
//NTSC MAX, half: 324x224 full: 648x448
const int XRES = 320;
const int YRES = 200;

//Graphics using the defined resolution for the backbuffer and fixed buffer for triangles
CompositeGraphics graphics(XRES, YRES, 1337);

//Composite output using the desired mode (PAL/NTSC) and twice the resolution. 
//It will center the displayed image automatically
CompositeOutput composite(CompositeOutput::NTSC, XRES * 2, YRES * 2);

#if defined(VENUS)
Mesh<CompositeGraphics> model(venus::vertexCount, venus::vertices, 0, 0, venus::triangleCount, venus::triangles, venus::triangleNormals);
#elif defined(DRAGON)
Mesh<CompositeGraphics> model(dragon::vertexCount, dragon::vertices, 0, 0, dragon::triangleCount, dragon::triangles, dragon::triangleNormals);
#elif defined(LOGO)
Mesh<CompositeGraphics> model(logo3d::vertexCount, logo3d::vertices, 0, 0, logo3d::triangleCount, logo3d::triangles, logo3d::triangleNormals);
#elif defined(SKULL)
Mesh<CompositeGraphics> model(skull::vertexCount, skull::vertices, 0, 0, skull::triangleCount, skull::triangles, skull::triangleNormals);
#endif
Image<CompositeGraphics> image(lablogo::xres, lablogo::yres, lablogo::pixels);

Font<CompositeGraphics> font(6, 8, font6x8::pixels);

#include <soc/rtc.h>

void setup()
{
  //highest clockspeed needed
  esp_pm_lock_handle_t powerManagementLock;
  esp_pm_lock_create(ESP_PM_CPU_FREQ_MAX, 0, "compositeCorePerformanceLock", &powerManagementLock);
  esp_pm_lock_acquire(powerManagementLock);
  
  //initializing DMA buffers and I2S
  composite.init();
  //initializing graphics double buffer
  graphics.init();
  //select font
  graphics.setFont(font);

  //running composite output pinned to first core
  xTaskCreatePinnedToCore(compositeCore, "compositeCoreTask", 1024, NULL, 1, NULL, 0);
  //rendering the actual graphics in the main loop is done on second core by default
}

void compositeCore(void *data)
{
  while (true)
  {
    //just send the graphics frontbuffer whithout any interruption 
    composite.sendFrameHalfResolution(&graphics.frame);
  }
}

void drawSkull()
{
  image.draw(graphics, 30, 70);
  static Matrix perspective = Matrix::translation(graphics.xres / 2, graphics.yres / 2, 0) * Matrix::scaling(100 * composite.pixelAspect, 100, 100) * Matrix::perspective(90, 1, 10);
  static float u = 0;
  static float v = 0;
  u += 0.05;
  v += 0.01;
  Matrix rotation = Matrix::rotation(u, 0, 1, 0) * Matrix::rotation(v, 1, 0, 0);
  Matrix m0 = perspective * Matrix::translation(-4, 1.7 * 0, 7) * rotation * Matrix::scaling(6);
  model.transform(m0, rotation);
  model.drawTriangles(graphics, 40);
  graphics.flush();
}

void drawVenus()
{
  static Matrix perspective = Matrix::translation(graphics.xres / 2, graphics.yres / 2, 0) * Matrix::scaling(100 * composite.pixelAspect, 100, 100) * Matrix::perspective(90, 1, 10);
  static float u = 0;
  u += 0.02;
  Matrix rotation = Matrix::rotation(-1.7, 1, 0, 0) * Matrix::rotation(u, 0, 0, 1);
  Matrix m0 = perspective * Matrix::translation(0, 1.7 * 0, 6) * rotation * Matrix::scaling(7);
  model.transform(m0, rotation);  
  model.drawTriangles(graphics, 40);
  graphics.flush();
}

void drawLogo()
{
  static Matrix perspective = Matrix::translation(graphics.xres / 2, graphics.yres / 2, 0) * Matrix::scaling(100 * composite.pixelAspect, 100, 100) * Matrix::perspective(90, 1, 10);
  static float u = 0;
  u += 0.02;
  Matrix rotation = Matrix::rotation(-0.2, 1, 0, 0) * Matrix::rotation(u, 0, 1, 0);
  Matrix m0 = perspective * Matrix::translation(0, 1.7 * 0, 7) * rotation * Matrix::scaling(7);
  model.transform(m0, rotation);  
  model.drawTriangles(graphics, 40);
  graphics.flush();
}

void drawDragon()
{
  static Matrix perspective = Matrix::translation(graphics.xres / 2, graphics.yres / 2, 0) * Matrix::scaling(100 * composite.pixelAspect, 100, 100) * Matrix::perspective(90, 1, 10);
  static float u = 0;
  u += 0.02;
  Matrix rotation = Matrix::rotation(-1.8 + sin(u * 0.3) * 0.2, 1, 0, 0) * Matrix::rotation(u, 0, 0, 1);
  Matrix m0 = perspective * Matrix::translation(0, 1.7 * 0, 6) * rotation * Matrix::scaling(8);
  model.transform(m0, rotation);  
  model.drawTriangles(graphics, 40);
  graphics.flush();
}

void draw()
{
  static int lastMillis = 0;
  int t = millis();
  int fps = 1000 / (t - lastMillis);
  lastMillis = t;

  graphics.begin(54);
  #if defined(LOGO)
    drawLogo();
  #elif defined(VENUS)
    drawVenus();
  #elif defined(DRAGON)
    drawDragon();
  #elif defined(SKULL)
    drawSkull();
  #endif
  graphics.setTextColor(5);
  graphics.setCursor(30, 5);
  graphics.print("free memory: ");
  graphics.print((int)heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
  graphics.print(" fps: ");
  graphics.print(fps, 10, 2);
  graphics.print(" triangles/s: ");
  graphics.print(fps * model.triangleCount);
  graphics.end();
}

void loop()
{
  draw();
}

//code by bitluni (send me a high five if you like the code)

#include "Matrix.h"
#include "AVGraphics.h"
#include "Mesh.h"
#include "Image.h"
#include "AVOutput.h"

namespace mesh0
{
//#include "bitlunislablogo.h"
#include "skull.h"
}

namespace image0
{
#include "luni.h"
}

namespace font6x8
{
#include "font6x8.h"
}

AVGraphics graphics(320, 200, 1337);
AVOutput<AVGraphics> av;

Mesh<AVGraphics> model(mesh0::vertexCount, mesh0::vertices, 0, 0, mesh0::triangleCount, mesh0::triangles, mesh0::triangleNormals);
Image<AVGraphics> luni(image0::xres, image0::yres, image0::pixels);
Font<AVGraphics> font(6, 8, font6x8::pixels);

#include <soc/rtc.h>

void setup()
{
  rtc_clk_cpu_freq_set(RTC_CPU_FREQ_240M);              //highest cpu frequency
  av.init();
  graphics.init();
  graphics.setFont(font);
  Serial.println(heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
  xTaskCreatePinnedToCore(graphicsCard, "g", 1024, NULL, 1, NULL, 0);
}

void graphicsCard(void *data)
{
  while (true)
  {
    av.sendFrame(graphics, false);
    av.sendFrame(graphics, true);
  }
}

void draw()
{
  static Matrix perspective = Matrix::translation(graphics.width / 2, graphics.height / 2, 0) * Matrix::scaling(120) * Matrix::perspective(90, 1, 10);
  static int lastMillis = 0;
  int t = millis();
  int fps = 1000 / (t - lastMillis);
  lastMillis = t;

  static float u = 0;
  static float v = 0;
  u += 0.05;
  v += 0.01;
  Matrix rotation = Matrix::rotation(u, 0, 1, 0) * Matrix::rotation(v, 1, 0, 0);
  Matrix m0 = perspective * Matrix::translation(4 * 0, 1.7 * 0, 6) * rotation * Matrix::scaling(6);
  model.transform(m0, rotation);
  graphics.begin(true);
  //luni.draw(graphics, 0, 0);
  model.drawTriangles(graphics, 40);
  graphics.setCursor(0, 0);
  graphics.print("hello!", 50);
  graphics.print(" free memory: ", 50);
  graphics.print((int)heap_caps_get_free_size(MALLOC_CAP_DEFAULT), 50);
  graphics.print(" fps: ", 50);
  graphics.print(fps, 50, 10, 3);
  graphics.print("\n triangles/s: ", 50);
  graphics.print(fps * model.triangleCount, 50);
  graphics.print("\n tree depth: ", 50);
  graphics.print(graphics.triangleRoot->depth, 50);
  graphics.end();
}

void loop()
{
  draw();
}



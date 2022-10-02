#ifndef BOARD_HAS_PSRAM
#error "Please enable PSRAM !!!"
#endif

#include <Arduino.h>
#include <esp_task_wdt.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "epd_driver.h"
#include "firasans.h"
#include <Wire.h>
#include <touch.h>
#include "datatypes.h"

#define TOUCH_INT   13
TouchClass touch;
uint8_t *framebuffer = NULL;

int cursor_x = 20;
int cursor_y = 60;

void setup()
{
  Serial.begin(115200);
  epd_init();

  pinMode(TOUCH_INT, INPUT_PULLUP);

  Wire.begin(15, 14);

  if (!touch.begin()) {
    Serial.println("start touchscreen failed");
    while (1);
  }
  Serial.println("Started Touchscreen poll...");


  framebuffer = (uint8_t *)ps_calloc(sizeof(uint8_t), EPD_WIDTH * EPD_HEIGHT / 2);
  if (!framebuffer) {
    Serial.println("alloc memory failed !!!");
    while (1);
  }
  memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);

  epd_poweron();
  epd_clear();
  //write_string((GFXfont *)&FiraSans, "Test", &cursor_x, &cursor_y, framebuffer);

  /*//Draw buttons
  touchutilAddButton(600, 450, 120, 60, "Prev", framebuffer);
  touchutilAddButton(740, 450, 120, 60, "Next", framebuffer);*/

  // Draw list box
  char elements[10][30];
  for (int i = 10; i < 20; i++) {
    sprintf(elements[i-10], "DL-111111111111%d", i);
  }
  //char *elements[10] = {"DL-11111111111111", "DL-11111111111112", "DL-11111111111113"};
  touchutilAddListBox(1, 100, 60, 600, 400, "Liste:", framebuffer, elements, 10);

  epd_draw_grayscale_image(epd_full_screen(), framebuffer);
  epd_poweroff();
}

void loop()
{
  uint16_t  x, y;
  if (digitalRead(TOUCH_INT)) {
    touchutilCheckTouch(framebuffer);
    ButtonData buttonData;
    if (touchutilGetPressedButton(&buttonData)) {
      // button found -> buttonData is filled
      Serial.print("Button pressed: ");
      Serial.print(buttonData.text);        
      Serial.print(" (ID = ");
      Serial.print(buttonData.id);
      Serial.println(")");
    }
  }
}

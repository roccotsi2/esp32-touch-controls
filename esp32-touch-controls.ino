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
#include "touchutil_datatypes.h"

#define TOUCH_INT   13
TouchClass touch;
uint8_t *frameBuffer = NULL;

int currentPageNo = 1;
int maxPageNo = 5;
int buttonIdNext;
int buttonIdPrev;

// variables for button + listbox
uint16_t  x, y;
char pressedListBoxItem[30];

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


  frameBuffer = (uint8_t *)ps_calloc(sizeof(uint8_t), EPD_WIDTH * EPD_HEIGHT / 2);
  if (!frameBuffer) {
    Serial.println("alloc memory failed !!!");
    while (1);
  }

  displayPage(1);
}

void displayPage(int pageNo) {
  if (pageNo > 0 && pageNo <= maxPageNo) {
    // display general elements
    memset(frameBuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2); // clear frame buffer
    touchutilInitialize();

    char pageText[10];
    sprintf(pageText, "Page %d:", pageNo);
    int cursor_x = 20;
    int cursor_y = 40;
    write_string((GFXfont *)&FiraSans, pageText, &cursor_x, &cursor_y, frameBuffer);
  
    //Draw buttons
    buttonIdPrev = touchutilGetButtonIdByIndex(touchutilAddButton(600, 470, 120, 60, "Prev", true, frameBuffer));
    /*Serial.print("buttonIdPrev: ");
    Serial.println(buttonIdPrev);*/
    buttonIdNext = touchutilGetButtonIdByIndex(touchutilAddButton(740, 470, 120, 60, "Next", true, frameBuffer));

    // display specific page elements
    if (pageNo == 1) {
      touchutilAddButton(740, 370, 120, 60, "", true, frameBuffer); // without text but with border
      touchutilAddButton(740, 270, 120, 60, "Test", false, frameBuffer); // without border
    
      // Draw list box
      char elements[10][30];
      int count = 10;
      for (int i = 10; i < 10 + count; i++) {
        sprintf(elements[i-10], "DL-111111111111%d", i);
      }
      touchutilAddListBox(1, 100, 60, 600, 400, "Liste:", frameBuffer, elements, count);
    } else if (pageNo == 2) {
      touchutilAddNumberEntry(1, 100, 60, 600, 400, frameBuffer);
    } else {
      // all other pages
    }

    //draw content
    epd_poweron();
    epd_clear();
    epd_draw_grayscale_image(epd_full_screen(), frameBuffer);
    epd_poweroff();
    currentPageNo = pageNo; 
  }
}

void loop()
{ 
  if (digitalRead(TOUCH_INT)) {
    touchutilCheckTouch(frameBuffer);
    ButtonData buttonData;
    if (touchutilGetPressedButton(&buttonData)) {
      // button found -> buttonData is filled
      Serial.print("Button pressed: ");
      Serial.print(buttonData.text);        
      Serial.print(" (ID = ");
      Serial.print(buttonData.id);
      Serial.println(")");
      if (buttonData.id == buttonIdNext) {
        if (currentPageNo < maxPageNo) {
          displayPage(++currentPageNo);
        }
      } else if (buttonData.id == buttonIdPrev) {
        if (currentPageNo > 1) {
          displayPage(--currentPageNo);
        }
      } else if (touchutilIsNumberEntryOkButtonPressed()) {
        int value = touchutilNumberEntryGetValue();
        Serial.print("Ok Button pressed, entered value: ");
        Serial.println(String(value));
      }
    } else if (touchutilGetPressedListBoxItem(pressedListBoxItem, sizeof(pressedListBoxItem))) {
      Serial.print("ListBoxItem pressed: ");
      Serial.println(pressedListBoxItem);        
    }

    Serial.print("Free heap: ");
    Serial.println(ESP.getFreeHeap());
  }
}

ButtonData buttons[10];
ListBoxData listBoxes[10];
int countButtons = 0;
int countListBoxes = 0;

const byte WIDTH_LIST_BOX_UP_DOWN_BUTTON = 50;
const byte HEIGHT_LIST_BOX_UP_DOWN_BUTTON = 50;
const int HEIGHT_LIST_BOX_ELEMENT = 60;

void touchutilAddListBox(int id, int x, int y, int width, int height, char *text, uint8_t *framebuffer, char *elements[10], int elementCount) {
  // add 1 button per displayed element
  int elementsPerPage = min(height / HEIGHT_LIST_BOX_ELEMENT, elementCount);
  for (int i = 0; i < elementsPerPage; i++) {
    touchutilAddButton(100 + i, x, y + i*HEIGHT_LIST_BOX_ELEMENT, width - WIDTH_LIST_BOX_UP_DOWN_BUTTON, HEIGHT_LIST_BOX_ELEMENT, elements[i], framebuffer);
  }

  // add border, up/down buttons
  epd_draw_rect(x, y, width, height, 0, framebuffer);
  epd_draw_rect(x + width - WIDTH_LIST_BOX_UP_DOWN_BUTTON, y, WIDTH_LIST_BOX_UP_DOWN_BUTTON, height, 0, framebuffer);
  touchutilAddButton(200, x + width - WIDTH_LIST_BOX_UP_DOWN_BUTTON, y, WIDTH_LIST_BOX_UP_DOWN_BUTTON, HEIGHT_LIST_BOX_UP_DOWN_BUTTON, "^", framebuffer);
  touchutilAddButton(201, x + width - WIDTH_LIST_BOX_UP_DOWN_BUTTON, y + height - HEIGHT_LIST_BOX_UP_DOWN_BUTTON, WIDTH_LIST_BOX_UP_DOWN_BUTTON, HEIGHT_LIST_BOX_UP_DOWN_BUTTON, "v", framebuffer);
}

void touchutilAddButton(int id, int x, int y, int width, int height, char *text, uint8_t *framebuffer) {
  epd_draw_rect(x, y, width, height, 0, framebuffer);
  int text_x = x + 15;
  int text_y = y + 40;
  write_string((GFXfont *)&FiraSans, text, &text_x, &text_y, framebuffer);

  // register button
  touchutilRegisterButton(id, x, y, width, height, text);
}

bool touchutilRegisterButton(int id, int x, int y, int width, int height, char *text) {
  if (countButtons >= sizeof(buttons)) {
    Serial.println("Could not register button, max size exceeded");
    return false;
  }

  Rect_t rect = {
    .x = x,
    .y = y,
    .width = width,
    .height = height
  };
  ButtonData buttonData = {
    .id = id,
    .area = rect,
    .text = text
  };
  buttons[countButtons] = buttonData;
  countButtons++;
  return true;
}

bool touchutilGetPressedButton(ButtonData *pressedButtonData) {
  if (touch.scanPoint()) {
    uint16_t  x, y;
    touch.getPoint(x, y, 0);
    y = EPD_HEIGHT - y;
    for (int i = 0; i < countButtons; i++) {
      ButtonData buttonData = buttons[i];
      if (x >= buttonData.area.x && x <= buttonData.area.x + buttonData.area.width && y >= buttonData.area.y && y <= buttonData.area.y + buttonData.area.height) {
        // button found, wait until it is released
        touchutilWaitUntilNoPress();

        // fill the found pressed button
        pressedButtonData->id = buttonData.id;
        pressedButtonData->area = buttonData.area;
        pressedButtonData->text = buttonData.text;
        return true;
      }
    }
  }
  return false; // no button found
}

void touchutilWaitUntilNoPress() {
  int count = 0;
  while (count < 10) {
    if (touch.scanPoint()) {
      count = 0;
    } else {
      count++;
    }
    delay(10);
  }
}

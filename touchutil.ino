ButtonData buttons[10];
ListBoxData listBoxes[10];
int countButtons = 0;
int countListBoxes = 0;
ButtonData lastPressedButton;
bool buttonPressed = false;

const byte WIDTH_LIST_BOX_UP_DOWN_BUTTON = 50;
const byte HEIGHT_LIST_BOX_UP_DOWN_BUTTON = 50;
const int HEIGHT_LIST_BOX_ELEMENT = 60;
const int ID_LIST_BOX_BUTTON_UP = 200;
const int ID_LIST_BOX_BUTTON_DOWN = 201;
const int ID_LIST_BOX_FIRST_ITEM_BUTTON = 100;

void touchutilAddListBox(int id, int x, int y, int width, int height, char *text, uint8_t *framebuffer, char *elements[10], int elementCount) {
  // add 1 button per displayed element
  int elementsPerPage = min(height / HEIGHT_LIST_BOX_ELEMENT, elementCount);
  int elementHeight = height / elementsPerPage;
  int adaptedHeight = elementsPerPage * elementHeight; // adapt the height to the number of elements
  for (int i = 0; i < elementsPerPage; i++) {
    touchutilAddButton(ID_LIST_BOX_FIRST_ITEM_BUTTON + i, x, y + i*elementHeight, width - WIDTH_LIST_BOX_UP_DOWN_BUTTON, elementHeight, elements[i], framebuffer);
  }

  // add border, up/down buttons
  epd_draw_rect(x, y, width, adaptedHeight, 0, framebuffer);
  epd_draw_rect(x + width - WIDTH_LIST_BOX_UP_DOWN_BUTTON, y, WIDTH_LIST_BOX_UP_DOWN_BUTTON, adaptedHeight, 0, framebuffer);
  touchutilAddButton(ID_LIST_BOX_BUTTON_UP, x + width - WIDTH_LIST_BOX_UP_DOWN_BUTTON, y, WIDTH_LIST_BOX_UP_DOWN_BUTTON, HEIGHT_LIST_BOX_UP_DOWN_BUTTON, "^", framebuffer);
  touchutilAddButton(ID_LIST_BOX_BUTTON_DOWN, x + width - WIDTH_LIST_BOX_UP_DOWN_BUTTON, y + adaptedHeight - HEIGHT_LIST_BOX_UP_DOWN_BUTTON, WIDTH_LIST_BOX_UP_DOWN_BUTTON, HEIGHT_LIST_BOX_UP_DOWN_BUTTON, "v", framebuffer);
}

void checkListBoxButtons() {
  if (buttonPressed) {
    if (lastPressedButton.id == ID_LIST_BOX_BUTTON_UP) {
      // button UP pressed
      Serial.println("ListBox button pressed: UP");
      buttonPressed = false;
    } else if (lastPressedButton.id == ID_LIST_BOX_BUTTON_DOWN) {
      // button DOWN pressed
      Serial.println("ListBox button pressed: DOWN");
      buttonPressed = false;
    } else if (lastPressedButton.id >= ID_LIST_BOX_FIRST_ITEM_BUTTON) {
      // item button pressed
      Serial.print("ListBox item pressed: ");
      Serial.println(lastPressedButton.text);
      buttonPressed = false;
    }
  }
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

void touchutilCheckTouch() {
  buttonPressed = false; // reset

  // check for pressed button
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
        lastPressedButton.id = buttonData.id;
        lastPressedButton.area = buttonData.area;
        lastPressedButton.text = buttonData.text;
        buttonPressed = true;
      }
    }
  }

  checkListBoxButtons();
}

bool touchutilGetPressedButton(ButtonData *pressedButtonData) {
  if (buttonPressed) {
    pressedButtonData->id = lastPressedButton.id;
    pressedButtonData->area = lastPressedButton.area;
    pressedButtonData->text = lastPressedButton.text;
    buttonPressed = false; // reset
    return true; // button found
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

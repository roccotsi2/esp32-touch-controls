ButtonData buttons[20];
ListBoxData listBox;
int countButtons = 0;
ButtonData lastPressedButton;
bool buttonPressed = false;

const byte WIDTH_LIST_BOX_UP_DOWN_BUTTON = 50;
const byte HEIGHT_LIST_BOX_UP_DOWN_BUTTON = 50;
const int HEIGHT_LIST_BOX_ELEMENT = 60;

boolean touchutilAddListBox(int id, int x, int y, int width, int height, char *text, uint8_t *framebuffer, char elements[10][30], int elementCount) {
  int indexNewListBox = touchutilRegisterListBox(id, x, y, width, height, text, elements, elementCount);
  if (indexNewListBox == -1) {
    return false;
  }
  
  // add 1 button per displayed element
  for (int i = 0; i < listBox.elementsPerPage; i++) {
    listBox.buttonIdsElements[i] = touchutilAddButton(x, y + i*listBox.elementHeight, width - WIDTH_LIST_BOX_UP_DOWN_BUTTON, listBox.elementHeight, listBox.elements[i], framebuffer);
  }

  int adaptedHeight = listBox.area.height;

  // add border, up/down buttons
  epd_draw_rect(x, y, width, adaptedHeight, 0, framebuffer);
  epd_draw_rect(x + width - WIDTH_LIST_BOX_UP_DOWN_BUTTON, y, WIDTH_LIST_BOX_UP_DOWN_BUTTON, adaptedHeight, 0, framebuffer);
  listBox.buttonIdUp = touchutilAddButton(x + width - WIDTH_LIST_BOX_UP_DOWN_BUTTON, y, WIDTH_LIST_BOX_UP_DOWN_BUTTON, HEIGHT_LIST_BOX_UP_DOWN_BUTTON, "^", framebuffer);
  listBox.buttonIdDown = touchutilAddButton(x + width - WIDTH_LIST_BOX_UP_DOWN_BUTTON, y + adaptedHeight - HEIGHT_LIST_BOX_UP_DOWN_BUTTON, WIDTH_LIST_BOX_UP_DOWN_BUTTON, HEIGHT_LIST_BOX_UP_DOWN_BUTTON, "v", framebuffer);

  return true; // ListBox successfully created
}

bool touchutilRegisterListBox(int id, int x, int y, int width, int height, char *text, char elements[10][30], int elementCount) {
  int elementsPerPage = min(height / HEIGHT_LIST_BOX_ELEMENT, elementCount);
  int elementHeight = height / elementsPerPage;
  int adpatedHeight = elementsPerPage * elementHeight;
  
  Rect_t rect = {
    .x = x,
    .y = y,
    .width = width,
    .height = adpatedHeight // adapt the height to the number of elements
  };
  listBox.id = id;
  listBox.area = rect;
  listBox.text = text;
  for (int i = 0; i < 10; i++) {
    strncpy(listBox.elements[i], elements[i], 30);
  }
  listBox.elementCount = elementCount;
  listBox.elementsPerPage = elementsPerPage;
  listBox.elementHeight = elementHeight;
  
  return true;
}

int findListBoxElementButtonIndexById(uint8_t  id) {
  for (int i = 0; i < 10; i++) {
    if (listBox.buttonIdsElements[i] == id) {
      return i;
    }
  }
  return -1;
}

void checkListBoxButtons() {
  if (buttonPressed) {
    if (lastPressedButton.id == listBox.buttonIdUp) {
      // button UP pressed
      Serial.println("ListBox button pressed: UP");
      buttonPressed = false;
    } else if (lastPressedButton.id == listBox.buttonIdDown) {
      // button DOWN pressed
      Serial.println("ListBox button pressed: DOWN");
      buttonPressed = false;
    } else {
      int listBoxElementButtonIndex = findListBoxElementButtonIndexById(lastPressedButton.id);
      if (listBoxElementButtonIndex > -1) {
        // item button pressed
        Serial.print("ListBox item pressed: ");
        Serial.println(lastPressedButton.text);
        buttonPressed = false;
      }
    }
  }
}

int touchutilAddButton(int x, int y, int width, int height, char *text, uint8_t *framebuffer) {
  epd_draw_rect(x, y, width, height, 0, framebuffer);
  int text_x = x + 15;
  int text_y = y + 40;
  write_string((GFXfont *)&FiraSans, text, &text_x, &text_y, framebuffer);

  // register button
  return touchutilRegisterButton(x, y, width, height, text);
}

int touchutilRegisterButton(int x, int y, int width, int height, char *text) {
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
    .id = countButtons,
    .area = rect,
    .text = text
  };
  buttons[countButtons] = buttonData;
  countButtons++;
  return buttonData.id;
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

#define COLOR_WHITE 0xF0

ButtonData buttons[20];
ListBoxData listBox;
int countButtons = 0;
ButtonData lastPressedButton;
char lastPressedListBoxItem[30];
bool listBoxItemPressed = false;
bool buttonPressed = false;
int lastPressedButtonIndex = -1;

const byte WIDTH_LIST_BOX_UP_DOWN_BUTTON = 50;
const byte HEIGHT_LIST_BOX_UP_DOWN_BUTTON = 50;
const int HEIGHT_LIST_BOX_ELEMENT = 60;

boolean touchutilAddListBox(int id, int x, int y, int width, int height, char *text, uint8_t *framebuffer, char elements[10][30], int elementCount) {
  bool success = touchutilRegisterListBox(id, x, y, width, height, text, elements, elementCount);
  //Serial.println("ListBox registered");
  if (!success) {
    return false;
  }

  touchutilDrawListBox(true, framebuffer);
  //Serial.println("ListBox drawed");
  
  return true; // ListBox successfully created
}

bool touchutilRegisterListBox(int id, int x, int y, int width, int height, char *text, char elements[10][30], int elementCount) {
  //int elementsPerPage = min(height / HEIGHT_LIST_BOX_ELEMENT, elementCount);
  int elementsPerPage = height / HEIGHT_LIST_BOX_ELEMENT;
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
    strcpy(listBox.elements[i], elements[i]);
  }
  
  listBox.elementCount = elementCount;
  listBox.elementsPerPage = elementsPerPage;
  listBox.elementHeight = elementHeight;
  listBox.currentPageNo = 1; // first page = 1
  if (elementCount % elementsPerPage == 0) {
    listBox.pageCount = elementCount / elementsPerPage;
  } else {
    listBox.pageCount = elementCount / elementsPerPage + 1;
  }
  memset(listBox.buttonIndexElements, -1, 10); // initialize buttonIndexElements with -1
  
  return true;
}

bool touchutilIsButtonIndexFromListBoxItem(uint8_t buttonIndex) {
  for (int i = 0; i < 10; i++) {
    if (listBox.buttonIndexElements[i] == buttonIndex) {
      return true;
    }
  }
  return false;
}

void touchutilCheckListBoxButtons(uint8_t *framebuffer) {
  listBoxItemPressed = false;
  if (buttonPressed) {
    if (lastPressedButtonIndex == listBox.buttonIndexUp) {
      // button UP pressed
      Serial.println("ListBox button pressed: UP");
      buttonPressed = false;
      touchutilChangePageList(listBox.currentPageNo - 1, framebuffer);
    } else if (lastPressedButtonIndex == listBox.buttonIndexDown) {
      // button DOWN pressed
      Serial.println("ListBox button pressed: DOWN");
      buttonPressed = false;
      touchutilChangePageList(listBox.currentPageNo + 1, framebuffer);
    } else {
      bool listBoxItemButtonPressed = touchutilIsButtonIndexFromListBoxItem(lastPressedButtonIndex);
      if (listBoxItemButtonPressed) {
        // item button pressed
        //Serial.print("ListBox item pressed: ");
        //Serial.println(lastPressedButton.text);
        buttonPressed = false;
        listBoxItemPressed = true;
        strcpy(lastPressedListBoxItem, lastPressedButton.text);
      }
    }
  }
}

void touchutilChangePageList(int newPageNo, uint8_t *framebuffer) {
  if (newPageNo < 1 || listBox.pageCount < newPageNo) {
    // start or end reached -> nothing to do
    return;
  }
  // from here newPageNo is valid
  listBox.currentPageNo = newPageNo;

  // replace the texts of the buttons
  int startIndex = (listBox.currentPageNo - 1) * listBox.elementsPerPage; // 6
  int endIndex = min(startIndex + listBox.elementsPerPage - 1, listBox.elementCount - 1); // 9
  /*Serial.print("startIndex: ");
  Serial.println(startIndex);
  Serial.print("endIndex: ");
  Serial.println(endIndex);*/
  
  for (int i = startIndex; i <= endIndex; i++) {
    int buttonIndex = listBox.buttonIndexElements[i - startIndex];
    strncpy(buttons[buttonIndex].text, listBox.elements[i], 30);
  }
  for (int i = endIndex - startIndex + 1; i < listBox.elementsPerPage; i++) {
    // set empty text for last listbox items
    int buttonIndex = listBox.buttonIndexElements[i];
    memset(buttons[buttonIndex].text, 0, 30);
  }

  touchutilDrawListBox(false, framebuffer);
}

void touchutilDrawListBox(bool initialDraw, uint8_t *framebuffer) { 
  if (!initialDraw) {
    // first clear the area of the list box if this is not the first time it gets drawed
    epd_fill_rect(listBox.area.x, listBox.area.y, listBox.area.width, listBox.area.height, COLOR_WHITE, framebuffer);
  }
  
  // add border, up/down buttons
  epd_draw_rect(listBox.area.x, listBox.area.y, listBox.area.width, listBox.area.height, 0, framebuffer); // border of list box
  //Serial.println("ListBox border drawed");
  epd_draw_rect(listBox.area.x + listBox.area.width - WIDTH_LIST_BOX_UP_DOWN_BUTTON, listBox.area.y, WIDTH_LIST_BOX_UP_DOWN_BUTTON, listBox.area.height, 0, framebuffer); // border of scroll
  //Serial.println("ListBox scroll border drawed");
  
  if (initialDraw) {
    // add 1 button per displayed element
    int count = min(listBox.elementsPerPage, listBox.elementCount);
    for (int i = 0; i < count; i++) {
      listBox.buttonIndexElements[i] = touchutilAddButton(listBox.area.x, listBox.area.y + i*listBox.elementHeight, listBox.area.width - WIDTH_LIST_BOX_UP_DOWN_BUTTON, listBox.elementHeight, listBox.elements[i], true, framebuffer);
    }
    //Serial.println("ListBox buttons added");
    // add UP / DOWN buttons
    listBox.buttonIndexUp = touchutilAddButton(listBox.area.x + listBox.area.width - WIDTH_LIST_BOX_UP_DOWN_BUTTON, listBox.area.y, WIDTH_LIST_BOX_UP_DOWN_BUTTON, HEIGHT_LIST_BOX_UP_DOWN_BUTTON, "^", true, framebuffer);
    listBox.buttonIndexDown = touchutilAddButton(listBox.area.x + listBox.area.width - WIDTH_LIST_BOX_UP_DOWN_BUTTON, listBox.area.y + listBox.area.height - HEIGHT_LIST_BOX_UP_DOWN_BUTTON, WIDTH_LIST_BOX_UP_DOWN_BUTTON, HEIGHT_LIST_BOX_UP_DOWN_BUTTON, "v", true, framebuffer);
    //Serial.println("Up/Down buttons added");
  } else {    
    // draw only the buttons
    for (int i = 0; i < listBox.elementsPerPage; i++) {
      ButtonData button = buttons[listBox.buttonIndexElements[i]];
      if (button.text[0] != 0) {
        // draw only button with text
        touchutilDrawButton(button, framebuffer);
      }
    }
    touchutilDrawButton(buttons[listBox.buttonIndexUp], framebuffer);
    touchutilDrawButton(buttons[listBox.buttonIndexDown], framebuffer);
    touchutilDrawScreen();
  }
}

void touchutilDrawButton(ButtonData button, uint8_t *framebuffer) {
  if (button.drawBorder) {
    epd_draw_rect(button.area.x, button.area.y, button.area.width, button.area.height, 0, framebuffer);
  }
  if (strlen(button.text) > 0) {
    int text_x = button.area.x + 15;
    int text_y = button.area.y + 40;
    write_string((GFXfont *)&FiraSans, button.text, &text_x, &text_y, framebuffer);
  }
  //Serial.println(button.text);
}

int touchutilAddButton(int x, int y, int width, int height, char *text, bool drawBorder, uint8_t *framebuffer) {
  // register button
  int buttonIndex = touchutilRegisterButton(x, y, width, height, text, drawBorder);

  // draw button
  touchutilDrawButton(buttons[buttonIndex], framebuffer);
  
  return buttonIndex;
}

int touchutilRegisterButton(int x, int y, int width, int height, char *text, bool drawBorder) {
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
    .drawBorder = drawBorder
  };
  strcpy(buttonData.text, text);
  int buttonIndex = countButtons;
  buttons[buttonIndex] = buttonData;
  countButtons++;
  return buttonIndex;
}

void touchutilCheckTouch(uint8_t *framebuffer) {
  buttonPressed = false; // reset

  // check for pressed button
  if (touch.scanPoint()) {
    uint16_t  x, y;    //pressedButtonData->text = lastPressedButton.text;
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
        strcpy(lastPressedButton.text, buttonData.text);
        lastPressedButtonIndex = i;
        buttonPressed = true;
        break;
      }
    }
  }

  touchutilCheckListBoxButtons(framebuffer);
}

bool touchutilGetPressedButton(ButtonData *pressedButtonData) {
  if (buttonPressed) {
    pressedButtonData->id = lastPressedButton.id;
    pressedButtonData->area = lastPressedButton.area;
    strcpy(pressedButtonData->text, lastPressedButton.text);
    buttonPressed = false; // reset
    return true; // button found
  }
  return false; // no button found
}

bool touchutilGetPressedListBoxItem(char *buf, int count) {
  if (!listBoxItemPressed) {
    return false;
  }
  if (count < sizeof(lastPressedListBoxItem)) {
    Serial.print("Size of buf too small, needed size: ");
    Serial.println(sizeof(lastPressedListBoxItem));
    return false;
  }
  strcpy(buf, lastPressedListBoxItem);
  return true;
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

void touchutilDrawScreen() {
  epd_poweron();
  epd_clear();
  epd_draw_grayscale_image(epd_full_screen(), framebuffer);
  epd_poweroff();
}

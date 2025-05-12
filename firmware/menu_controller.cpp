/**
 * @file menu_controller.cpp
 * @brief Implementation of menu system for badge UI
 * @author Francisco Torres - Electronic Cats - electroniccats.com
 * @date May 2025
 */

#include "menu_controller.h"
#include "display_controller.h"
#include "input_controller.h"
#include "nfc_controller.h"
#include "nfc_display.h"

// Global menu controller instance
MenuController menuController;

// Menu definitions
enum {
  MENU_MAIN = 0,
  MENU_APPS,
  MENU_NFC,
  MENU_COUNT  // Always keep this last
};

// Define menus
Menu menus[MENU_COUNT] = {
    // Main Menu
    {"Main Menu",
     2,
     {{"Apps", MENU_TYPE_SUBMENU, {.submenuId = MENU_APPS}},
      {"About", MENU_TYPE_FUNCTION, {.function = showAbout}}}},

    // Apps Menu
    {"Apps",
     2,
     {{"NFC", MENU_TYPE_SUBMENU, {.submenuId = MENU_NFC}},
      {"Magspoof", MENU_TYPE_FUNCTION, {.function = runMagspoof}}}},

    // NFC Menu
    {"NFC",
     4,
     {{"Detect Tags", MENU_TYPE_FUNCTION, {.function = runDetectTags}},
      {"Detect Readers", MENU_TYPE_FUNCTION, {.function = runDetectReaders}},
      {"NDEF Send", MENU_TYPE_FUNCTION, {.function = runNdefSend}},
      {"NDEF Read", MENU_TYPE_FUNCTION, {.function = runNdefRead}}}}};

void MenuController::initialize() {
  _currentMenuId = MENU_MAIN;
  _currentIndex = 0;
  _scrollOffset = 0;
  _menuStackPos = 0;

  // Reset menu navigation stack
  memset(_menuStackIds, 0, sizeof(_menuStackIds));
}

void MenuController::update() {
  // Update buttons
  inputController.update();

  // Handle button presses
  if (inputController.isUpPressed()) {
    navigateUp();
  } else if (inputController.isDownPressed()) {
    navigateDown();
  } else if (inputController.isSelectPressed()) {
    navigateSelect();
  } else if (inputController.isBackPressed()) {
    navigateBack();
  }
}

void MenuController::render() {
  Adafruit_SSD1306* display = displayController.getDisplay();
  Menu* currentMenu = &menus[_currentMenuId];

  // Clear display
  display->clearDisplay();

  // Draw title
  display->setTextSize(1);
  display->setTextColor(SSD1306_WHITE);
  display->setCursor(0, 0);
  display->println(currentMenu->name);

  // Draw separator line
  display->drawLine(0, 8, display->width(), 8, SSD1306_WHITE);

  // Draw menu items
  for (uint8_t i = 0;
       i < DISPLAY_ROWS && i + _scrollOffset < currentMenu->itemCount; i++) {
    uint8_t yPos = 10 + i * 8;

    // Highlight selected item
    if (i + _scrollOffset == _currentIndex) {
      display->fillRect(0, yPos - 1, display->width(), 8, SSD1306_WHITE);
      display->setTextColor(SSD1306_BLACK);
    } else {
      display->setTextColor(SSD1306_WHITE);
    }

    display->setCursor(2, yPos);
    display->println(currentMenu->items[i + _scrollOffset].name);

    // Draw submenu indicator
    if (currentMenu->items[i + _scrollOffset].type == MENU_TYPE_SUBMENU) {
      display->setCursor(display->width() - 6, yPos);
      display->print(">");
    }
  }

  // Draw scroll indicators if necessary
  if (_scrollOffset > 0) {
    display->fillTriangle(display->width() - 5, 9, display->width() - 8, 12,
                          display->width() - 2, 12, SSD1306_WHITE);
  }

  if (_scrollOffset + DISPLAY_ROWS < currentMenu->itemCount) {
    display->fillTriangle(display->width() - 5, 31, display->width() - 8, 28,
                          display->width() - 2, 28, SSD1306_WHITE);
  }

  display->display();
}

void MenuController::navigateUp() {
  if (_currentIndex > 0) {
    _currentIndex--;
    adjustScroll();
  }
}

void MenuController::navigateDown() {
  if (_currentIndex < menus[_currentMenuId].itemCount - 1) {
    _currentIndex++;
    adjustScroll();
  }
}

void MenuController::navigateSelect() {
  Menu* currentMenu = &menus[_currentMenuId];
  MenuItem* selectedItem = &currentMenu->items[_currentIndex];

  if (selectedItem->type == MENU_TYPE_SUBMENU) {
    // Push current menu to stack
    _menuStackIds[_menuStackPos++] = _currentMenuId;

    // Navigate to submenu
    _currentMenuId = selectedItem->submenuId;
    _currentIndex = 0;
    _scrollOffset = 0;
  } else if (selectedItem->type == MENU_TYPE_FUNCTION) {
    // Call the function
    selectedItem->function();
  }
}

void MenuController::navigateBack() {
  if (_menuStackPos > 0) {
    // Pop menu from stack
    _currentMenuId = _menuStackIds[--_menuStackPos];
    _currentIndex = 0;
    _scrollOffset = 0;
  }
}

void MenuController::adjustScroll() {
  if (_currentIndex < _scrollOffset) {
    _scrollOffset = _currentIndex;
  } else if (_currentIndex >= _scrollOffset + DISPLAY_ROWS) {
    _scrollOffset = _currentIndex - DISPLAY_ROWS + 1;
  }
}

// Menu action functions implementations
void runDetectTags() {
  Adafruit_SSD1306* display = displayController.getDisplay();
  display->clearDisplay();
  display->setTextColor(SSD1306_WHITE);
  display->setCursor(0, 0);
  display->println(F("Detecting tags..."));
  display->println(F("Place tag near"));
  display->println(F("the antenna"));
  display->display();

  // Get global NFC instance - declared in firmware.ino
  extern Electroniccats_PN7150 nfc;

  // Use existing tag detection function
  if (handleTagDetection(nfc)) {
    // Show tag info on display
    String tagInfo = getTagInfoForDisplay(nfc);

    // Add instructions to the tag info
    tagInfo += "\n\nPress BACK button";

    displayController.showTagInfo(tagInfo);

    // Wait for back button instead of using a fixed delay
    while (!inputController.isBackPressed()) {
      inputController.update();
      delay(10);
    }
  } else {
    // No tag detected
    display->clearDisplay();
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(0, 0);
    display->println(F("No tag detected"));
    display->println(F("Press BACK to"));
    display->println(F("return to menu"));
    display->display();
  }

  resetNfcController(nfc);

  // Wait for back button press to return to menu
  while (!inputController.isBackPressed()) {
    inputController.update();
    delay(10);
  }
}

// Placeholder functions for other menu items
void runDetectReaders() {
  Adafruit_SSD1306* display = displayController.getDisplay();
  display->clearDisplay();
  display->setTextColor(SSD1306_WHITE);
  display->setCursor(0, 0);
  display->println(F("Detect Readers"));
  display->println(F("Not implemented"));
  display->display();

  // Wait for back button press
  while (!inputController.isBackPressed()) {
    inputController.update();
    delay(10);
  }
}

void runNdefSend() {
  Adafruit_SSD1306* display = displayController.getDisplay();
  display->clearDisplay();
  display->setTextColor(SSD1306_WHITE);
  display->setCursor(0, 0);
  display->println(F("NDEF Send"));
  display->println(F("Not implemented"));
  display->display();

  // Wait for back button press
  while (!inputController.isBackPressed()) {
    inputController.update();
    delay(10);
  }
}

void runNdefRead() {
  Adafruit_SSD1306* display = displayController.getDisplay();
  display->clearDisplay();
  display->setTextColor(SSD1306_WHITE);
  display->setCursor(0, 0);
  display->println(F("NDEF Read"));
  display->println(F("Not implemented"));
  display->display();

  // Wait for back button press
  while (!inputController.isBackPressed()) {
    inputController.update();
    delay(10);
  }
}

void runMagspoof() {
  Adafruit_SSD1306* display = displayController.getDisplay();
  display->clearDisplay();
  display->setTextColor(SSD1306_WHITE);
  display->setCursor(0, 0);
  display->println(F("Magspoof"));
  display->println(F("Not implemented"));
  display->display();

  // Wait for back button press
  while (!inputController.isBackPressed()) {
    inputController.update();
    delay(10);
  }
}

void showAbout() {
  Adafruit_SSD1306* display = displayController.getDisplay();
  display->clearDisplay();
  display->setTextColor(SSD1306_WHITE);
  display->setCursor(0, 0);
  display->println(F("Recon Badge 2025"));
  display->println(F("by Electronic Cats"));
  display->display();

  // Wait for back button press
  while (!inputController.isBackPressed()) {
    inputController.update();
    delay(10);
  }
}
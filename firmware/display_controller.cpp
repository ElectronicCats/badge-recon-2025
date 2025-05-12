/**
 * @file display_controller.cpp
 * @brief Implementation of OLED display controller
 * @author Francisco Torres - Electronic Cats - electroniccats.com
 * @date May 2025
 */

#include "display_controller.h"
#include <Wire.h>
#include "display_assets.h"

DisplayController displayController;
static Adafruit_SSD1306 display(128, 32, &Wire, -1);

bool DisplayController::initialize(uint16_t width,
                                   uint16_t height,
                                   uint8_t address) {
  _display = &display;

  // Initialize OLED display
  if (!_display->begin(SSD1306_SWITCHCAPVCC, address)) {
    return false;
  }

  return true;
}

void DisplayController::showWelcomeScreen() {
  _display->clearDisplay();
  _display->drawBitmap(0, 0, epd_bitmap_recon_logo, 128, 32, SSD1306_WHITE);
  _display->display();
}

void DisplayController::showTagInfo(const String& tagInfo) {
  _display->clearDisplay();
  _display->setTextColor(SSD1306_WHITE);
  _display->setCursor(0, 0);
  _display->println(tagInfo);
  _display->display();
}

Adafruit_SSD1306* DisplayController::getDisplay() {
  return _display;
}
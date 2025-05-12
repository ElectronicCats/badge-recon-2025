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
  _display->setTextSize(1);
  _display->setTextColor(SSD1306_WHITE);
  _display->setCursor(0, 0);
  
  // Display each line with proper wrapping
  int startPos = 0;
  int lineHeight = 8; // Font height for size 1
  int yPos = 0;
  int maxCharsPerLine = 21; // Approximate max characters for 128px width
  
  // Parse and display each line
  while (startPos < tagInfo.length() && yPos < _display->height()) {
    int endPos = tagInfo.indexOf('\n', startPos);
    if (endPos == -1) {
      endPos = tagInfo.length();
    }
    
    String line = tagInfo.substring(startPos, endPos);
    
    // Handle line wrapping if needed
    if (line.length() > maxCharsPerLine) {
      _display->setCursor(0, yPos);
      _display->println(line.substring(0, maxCharsPerLine));
      yPos += lineHeight;
      
      if (yPos < _display->height()) {
        _display->setCursor(0, yPos);
        _display->println(line.substring(maxCharsPerLine));
        yPos += lineHeight;
      }
    } else {
      _display->setCursor(0, yPos);
      _display->println(line);
      yPos += lineHeight;
    }
    
    startPos = endPos + 1;
  }
  
  _display->display();
}

Adafruit_SSD1306* DisplayController::getDisplay() {
  return _display;
}
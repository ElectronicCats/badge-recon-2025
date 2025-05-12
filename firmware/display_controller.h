/**
 * @file display_controller.h
 * @brief OLED display controller for the badge
 * @author Francisco Torres - Electronic Cats - electroniccats.com
 * @date May 2025
 */

#ifndef DISPLAY_CONTROLLER_H
#define DISPLAY_CONTROLLER_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class DisplayController {
 public:
  /**
   * @brief Initialize the display
   * 
   * @param width Display width in pixels
   * @param height Display height in pixels
   * @param address I2C address of display
   * @return bool true if initialization successful
   */
  bool initialize(uint16_t width, uint16_t height, uint8_t address);
  
  /**
   * @brief Show welcome screen
   */
  void showWelcomeScreen();
  
  /**
   * @brief Show NFC tag information
   * 
   * @param tagInfo String containing tag information
   */
  void showTagInfo(const String& tagInfo);
  
  /**
   * @brief Get reference to the display object
   */
  Adafruit_SSD1306* getDisplay();

 private:
  Adafruit_SSD1306* _display;
};

extern DisplayController displayController;

#endif // DISPLAY_CONTROLLER_H
/**
 * @file menu_controller.h
 * @brief Menu system for badge UI
 * @author Francisco Torres - Electronic Cats - electroniccats.com
 * @date May 2025
 */

#ifndef MENU_CONTROLLER_H
#define MENU_CONTROLLER_H

#include <Arduino.h>
#include "Electroniccats_PN7150.h"
#include "display_controller.h"
#include "input_controller.h"

// Maximum items per menu level
#define MAX_MENU_ITEMS 10
// Maximum displayed items on screen
#define DISPLAY_ROWS 3

// Menu item type
typedef enum {
  MENU_TYPE_SUBMENU,  // Has submenu
  MENU_TYPE_FUNCTION  // Runs a function
} MenuItemType;

// Menu item structure
typedef struct {
  const char* name;   // Display name
  MenuItemType type;  // Type of menu item
  union {
    uint8_t submenuId;   // ID of submenu if type is MENU_TYPE_SUBMENU
    void (*function)();  // Function to call if type is MENU_TYPE_FUNCTION
  };
} MenuItem;

// Menu structure
typedef struct {
  const char* name;                // Menu name
  uint8_t itemCount;               // Number of items
  MenuItem items[MAX_MENU_ITEMS];  // Menu items
} Menu;

class MenuController {
 public:
  /**
   * @brief Initialize the menu system
   */
  void initialize();

  /**
   * @brief Update menu state based on inputs
   * Should be called in main loop
   */
  void update();

  /**
   * @brief Render current menu to display
   */
  void render();

 private:
  uint8_t _currentMenuId;    // Current menu level
  uint8_t _currentIndex;     // Currently selected item
  uint8_t _scrollOffset;     // Scroll offset for displaying items
  uint8_t _menuStackIds[5];  // Menu navigation history
  uint8_t _menuStackPos;     // Position in menu history

  // Navigation functions
  void navigateUp();
  void navigateDown();
  void navigateSelect();
  void navigateBack();

  // Helper functions
  void adjustScroll();
};

extern MenuController menuController;

// Function prototypes for menu actions
void runDetectTags();
void runDetectReaders();
void runNdefSend();
void runNdefRead();
void runMagspoof();
void showAbout();
void messageReceivedCallback();
void countNdefRecords(NdefMessage& ndefMessage);
void showNdefRecordWithNavigation(NdefMessage& ndefMessage, Adafruit_SSD1306* display, int recordIndex);
void displayNdefRecord(NdefRecord record, Adafruit_SSD1306* display);

#endif  // MENU_CONTROLLER_H
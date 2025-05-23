/**
 * @file badge-recon-2025.ino
 * @brief Main sketch for NFC tag detection system
 * @author Francisco Torres - Electronic Cats - electroniccats.com
 * @date May 2025
 *
 * This code is beerware; if you see me (or any other collaborator
 * member) at the local, and you've found our code helpful,
 * please buy us a round!
 * Distributed as-is; no warranty is given.
 */

#include <SPI.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>
#include <ezButton.h>
#include "Electroniccats_PN7150.h"

#include "display_controller.h"
#include "input_controller.h"
#include "magspoof.h"
#include "nfc_config.h"
#include "nfc_controller.h"
#include "nfc_display.h"

// Display configuration
#define SCREEN_WIDTH   128   // OLED display width in pixels
#define SCREEN_HEIGHT  32    // OLED display height in pixels
#define OLED_RESET     -1    // Reset pin (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  // I2C address of the SSD1306 display
#define IC2_SDA_PIN    12
#define IC2_SCL_PIN    13

// Buttons configuration
#define BUTTON_UP_PIN      5
#define BUTTON_DOWN_PIN    4
#define BUTTON_SELECT_PIN  2
#define BUTTON_BACK_PIN    1
#define BUTTON_DEBOUNCE_MS 50

// Menu system configuration
#define MAX_MENU_ITEMS 10  // Maximum items per menu level
#define DISPLAY_ROWS   3   // Maximum displayed items on screen

// Read/Write block configuration
// Block to be read
#define BLK_NB_MFC 4
// Default Mifare Classic key
#define KEY_MFC 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF

// Data to be written in the Mifare Classic block
#define DATA_WRITE_MFC                                                    \
  0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, \
      0xcc, 0xdd, 0xee, 0xff

/**
 * @brief Global NFC device interface object
 *
 * Creates a global NFC device interface object, attached to pins defined
 * in nfc_config.h and using the specified I2C address
 */
Electroniccats_PN7150 nfc(PN7150_IRQ, PN7150_VEN, PN7150_ADDR, PN7150, &Wire);

/**
 * @brief Display object for SSD1306 OLED
 */
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/**
 * @brief Preferences object for storing settings
 */
Preferences preferences;

// Menu item type
typedef enum {
  MENU_TYPE_SUBMENU,  // Has submenu
  MENU_TYPE_FUNCTION  // Runs a function
} MenuItemType;

// Forward declarations of menu action functions
void runDetectTags();
void runDetectReaders();
void runReadBlock();
void runWriteBlock();
void runMagspoof();
void runMagspoofSetup();
void setupTracks(String newTrack1 = "", String newTrack2 = "");
void showAbout();
void showMagspoofHelp();

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

// Menu definitions
enum { MENU_MAIN = 0, MENU_APPS, MENU_NFC, MENU_MAGSPOOF, MENU_COUNT };

/**
 * @brief Menu controller class
 *
 * Handles menu navigation, rendering, and user interaction
 */
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
      {"Magspoof", MENU_TYPE_SUBMENU, {.submenuId = MENU_MAGSPOOF}}}},

    // NFC Menu
    {"NFC",
     4,
     {{"Detect Tags", MENU_TYPE_FUNCTION, {.function = runDetectTags}},
      {"Detect Readers", MENU_TYPE_FUNCTION, {.function = runDetectReaders}},
      {"Read block", MENU_TYPE_FUNCTION, {.function = runReadBlock}},
      {"Write block", MENU_TYPE_FUNCTION, {.function = runWriteBlock}}}},

    // Magspoof Menu
    {"Magspoof",
     3,
     {{"Emulate", MENU_TYPE_FUNCTION, {.function = runMagspoof}},
      {"Setup", MENU_TYPE_FUNCTION, {.function = runMagspoofSetup}},
      {"Help", MENU_TYPE_FUNCTION, {.function = showMagspoofHelp}}}}};

MenuController menuController;

// MenuController implementation
void MenuController::initialize() {
  _currentMenuId = MENU_MAIN;
  _currentIndex = 0;
  _scrollOffset = 0;
  _menuStackPos = 0;

  // Reset menu navigation stack
  memset(_menuStackIds, 0, sizeof(_menuStackIds));
}

void MenuController::update() {
  inputController.update();

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

void runDetectTags() {
  Adafruit_SSD1306* display = displayController.getDisplay();
  display->clearDisplay();
  display->setTextColor(SSD1306_WHITE);
  display->setCursor(0, 0);
  display->println(F("Detecting tags..."));
  display->println(F("Place tag near"));
  display->println(F("the antenna"));
  display->display();

  // Set card reader/writer mode - required for tag detection
  if (!nfc.setReaderWriterMode()) {
    display->clearDisplay();
    display->setCursor(0, 0);
    display->println(F("Error setting"));
    display->println(F("reader/writer mode"));
    display->display();
    delay(2000);
    // return;
  }

  bool tagDetected = false;
  String tagInfo;
  while (!inputController.isBackPressed()) {
    inputController.update();
    if (nfc.isTagDetected()) {
      tagDetected = true;
      tagInfo = getTagInfoForDisplay(nfc);

      display->clearDisplay();
      display->setCursor(0, 0);
      display->println(F("Tag detected!"));
      display->println(F("Please remove it"));
      display->println(F("from the antenna"));
      display->display();

      // It can detect multiple cards at the same time if they use the same
      // protocol
      if (nfc.remoteDevice.hasMoreTags()) {
        nfc.activateNextTagDiscovery();
        Serial.println("Multiple cards are detected!");
      }

      Serial.println("Remove the Card");
      nfc.waitForTagRemoval();
      Serial.println("Card removed!");
    }

    Serial.println("Restarting...");
    nfc.reset();
    Serial.println("Waiting for a Card...");

    if (tagDetected) {
      break;
    }

    delay(10);
  }

  if (tagDetected) {
    Serial.println("Tag detected!");
    // Add instructions to the tag info
    tagInfo += "\n\nPress BACK button";

    displayController.showTagInfo(tagInfo);

    // Wait for back button instead of using a fixed delay
    while (!inputController.isBackPressed()) {
      inputController.update();
      delay(10);
    }
  } else {
    Serial.println("No tag detected!");
    display->clearDisplay();
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(0, 0);
    display->println(F("No tag detected"));
    display->println(F("Press BACK to"));
    display->println(F("return to menu"));
    display->display();
  }

  nfc.reset();

  // Wait for back button press to return to menu
  while (!inputController.isBackPressed()) {
    inputController.update();
    delay(10);
  }
}

void runDetectReaders() {
  Adafruit_SSD1306* display = displayController.getDisplay();
  display->clearDisplay();
  display->setTextColor(SSD1306_WHITE);
  display->setCursor(0, 0);
  display->println(F("Detect Readers"));
  display->println(F("Please wait..."));
  display->display();

  while (!initializeNfcController(nfc)) {
    delay(1000);
  }

  // Set card emulation mode - required for reader detection
  if (!nfc.setEmulationMode()) {
    Serial.println("Error setting emulation mode!");
    display->clearDisplay();
    display->setCursor(0, 0);
    display->println(F("Error setting"));
    display->println(F("emulation mode"));
    display->display();
    delay(2000);
    // return;
  }

  display->clearDisplay();
  display->setCursor(0, 0);
  display->println(F("Waiting for reader"));
  display->println(F("Hold near a phone"));
  display->println(F("or card reader"));
  display->println(F("BACK to cancel"));
  display->display();

  Serial.println("Emulation mode set!");
  Serial.print("Waiting for a reader...");

  // Animation dots for waiting
  uint8_t animDots = 0;
  uint8_t animFrame = 0;
  unsigned long lastAnimUpdate = 0;
  boolean readerFound = false;

  // Wait for reader detection or back button
  while (!inputController.isBackPressed()) {
    inputController.update();
    // Serial.print(".");

    if (nfc.isReaderDetected()) {
      readerFound = true;
      nfc.handleCardEmulation();
      nfc.closeCommunication();
      Serial.println("\nReader detected!");
      break;
    }
  }

  if (readerFound) {
    display->clearDisplay();
    display->setCursor(0, 0);
    display->println(F("Reader detected!"));
    display->println(F("Emulation complete"));
    display->println(F("Press BACK button"));
    display->display();

    // Wait for back button to return to menu
    while (!inputController.isBackPressed()) {
      inputController.update();
      delay(10);
    }
  } else {
    Serial.println("No reader detected!");
  }

  nfc.reset();
}

bool mifare_read_block(void) {
  Adafruit_SSD1306* display = displayController.getDisplay();
  display->setTextColor(SSD1306_WHITE);

  Serial.println("Start reading process...");
  bool status;
  unsigned char Resp[256];
  unsigned char RespSize;
  /* Authenticate sector 1 with generic keys */
  unsigned char Auth[] = {0x40, BLK_NB_MFC / 4, 0x10, KEY_MFC};
  /* Read block 4 */
  unsigned char Read[] = {0x10, 0x30, BLK_NB_MFC};

  /* Authenticate */
  status = nfc.readerTagCmd(Auth, sizeof(Auth), Resp, &RespSize);
  if ((status == NFC_ERROR) || (Resp[RespSize - 1] != 0)) {
    Serial.println("Auth error!");
    display->clearDisplay();
    display->setCursor(0, 0);
    display->println(F("Auth error!"));
    display->display();
    return false;
  }

  /* Read block */
  status = nfc.readerTagCmd(Read, sizeof(Read), Resp, &RespSize);
  if ((status == NFC_ERROR) || (Resp[RespSize - 1] != 0)) {
    Serial.print("Error reading sector!");
    display->clearDisplay();
    display->setCursor(0, 0);
    display->println(F("Error reading sector!"));
    display->display();
    return false;
  }

  return true;
}

bool mifare_read_write_block(void) {
  Adafruit_SSD1306* display = displayController.getDisplay();
  display->setTextColor(SSD1306_WHITE);

  Serial.println("Start reading process...");
  bool status;
  unsigned char Resp[256];
  unsigned char RespSize;
  /* Authenticate sector 1 with generic keys */
  unsigned char Auth[] = {0x40, BLK_NB_MFC / 4, 0x10, KEY_MFC};
  /* Read block 4 */
  unsigned char Read[] = {0x10, 0x30, BLK_NB_MFC};
  /* Write block 4 */
  unsigned char WritePart1[] = {0x10, 0xA0, BLK_NB_MFC};
  unsigned char WritePart2[] = {0x10, DATA_WRITE_MFC};

  /* Authenticate */
  status = nfc.readerTagCmd(Auth, sizeof(Auth), Resp, &RespSize);
  // if ((status == NFC_ERROR) || (Resp[RespSize - 1] != 0)) {
  if (false) {
    Serial.println("Auth error!");
    display->clearDisplay();
    display->setCursor(0, 0);
    display->println(F("Auth error!"));
    display->display();
    return false;
  }

  /* Read block */
  status = nfc.readerTagCmd(Read, sizeof(Read), Resp, &RespSize);
  // if ((status == NFC_ERROR) || (Resp[RespSize - 1] != 0)) {
  if (false) {
    Serial.print("Error reading sector!");
    display->clearDisplay();
    display->setCursor(0, 0);
    display->println(F("Error reading sector!"));
    display->display();
    return false;
  }

  /* Write block */
  status = nfc.readerTagCmd(WritePart1, sizeof(WritePart1), Resp, &RespSize);
  // if ((status == NFC_ERROR) || (Resp[RespSize - 1] != ChipWriteAck)) {
  if (false) {
    Serial.print("Error writing block!");
    display->clearDisplay();
    display->setCursor(0, 0);
    display->println(F("Error writing block!"));
    display->display();
    return false;
  }

  status = nfc.readerTagCmd(WritePart2, sizeof(WritePart2), Resp, &RespSize);
  // if ((status == NFC_ERROR) || (Resp[RespSize - 1] != ChipWriteAck)) {
  if (false) {
    Serial.print("Error writing block!");
    display->clearDisplay();
    display->setCursor(0, 0);
    display->println(F("Error writing block!"));
    display->display();
    return false;
  }

  /* Read block again to see te changes*/
  status = nfc.readerTagCmd(Read, sizeof(Read), Resp, &RespSize);
  // if ((status == NFC_ERROR) || (Resp[RespSize - 1] != 0)) {
  if (false) {
    Serial.print("Error reading block!");
    display->clearDisplay();
    display->setCursor(0, 0);
    display->println(F("Error reading block!"));
    display->println(F("After writing"));
    display->display();
    return false;
  }

  return true;
}

void runReadBlock() {
  Adafruit_SSD1306* display = displayController.getDisplay();
  display->clearDisplay();
  display->setTextColor(SSD1306_WHITE);
  display->setCursor(0, 0);
  display->println(F("Detecting tags..."));
  display->println(F("Place tag near"));
  display->println(F("the antenna"));
  display->display();

  // Set card reader/writer mode - required for tag detection
  if (!nfc.setReaderWriterMode()) {
    display->clearDisplay();
    display->setCursor(0, 0);
    display->println(F("Error setting"));
    display->println(F("reader/writer mode"));
    display->display();
    delay(2000);
    // return;
  }

  bool tagDetected = false;
  while (!inputController.isBackPressed()) {
    inputController.update();

    if (nfc.isTagDetected()) {
      tagDetected = true;

      display->clearDisplay();
      display->setCursor(0, 0);
      display->println(F("Tag detected!"));

      switch (nfc.remoteDevice.getProtocol()) {
        case nfc.protocol.MIFARE:
          Serial.println(" - Found MIFARE card");
          display->println(F("Starting reading"));
          display->println(F("process..."));
          display->display();
          if (mifare_read_block()) {
            display->clearDisplay();
            display->setCursor(0, 0);
            display->println(F("Successful read!"));
            display->display();
          }
          break;

        default:
          Serial.println(" - Found a card, but it is not Mifare");
          display->println(F("but it is not Mifare"));
          display->display();
          break;
      }

      // It can detect multiple cards at the same time if they use the same
      // protocol
      if (nfc.remoteDevice.hasMoreTags()) {
        nfc.activateNextTagDiscovery();
        Serial.println("Multiple cards are detected!");
      }

      display->println(F("Please remove the tag"));
      display->println(F("from the antenna"));
      display->display();

      Serial.println("Remove the Card");
      nfc.waitForTagRemoval();
      Serial.println("Card removed!");
    }

    Serial.println("Restarting...");
    nfc.reset();
    Serial.println("Waiting for a Card...");
    delay(10);

    if (tagDetected) {
      break;
    }
  }

  if (tagDetected) {
    display->println(F("Press BACK button"));
    display->display();

    while (!inputController.isBackPressed()) {
      inputController.update();
      delay(10);
    }
  } else {
    Serial.println("No tag detected!");
    display->clearDisplay();
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(0, 0);
    display->println(F("No tag detected"));
    display->println(F("Press BACK to"));
    display->println(F("return to menu"));
    display->display();
  }

  nfc.reset();

  while (!inputController.isBackPressed()) {
    inputController.update();
    delay(10);
  }
}

void runWriteBlock() {
  Adafruit_SSD1306* display = displayController.getDisplay();
  display->clearDisplay();
  display->setTextColor(SSD1306_WHITE);
  display->setCursor(0, 0);
  display->println(F("Detecting tags..."));
  display->println(F("Place tag near"));
  display->println(F("the antenna"));
  display->display();

  // Set card reader/writer mode - required for tag detection
  if (!nfc.setReaderWriterMode()) {
    display->clearDisplay();
    display->setCursor(0, 0);
    display->println(F("Error setting"));
    display->println(F("reader/writer mode"));
    display->display();
    delay(2000);
    // return;
  }

  bool tagDetected = false;
  while (!inputController.isBackPressed()) {
    inputController.update();

    if (nfc.isTagDetected()) {
      tagDetected = true;

      display->clearDisplay();
      display->setCursor(0, 0);
      display->println(F("Tag detected!"));

      switch (nfc.remoteDevice.getProtocol()) {
        case nfc.protocol.MIFARE:
          Serial.println(" - Found MIFARE card");
          display->println(F("Starting writing"));
          display->println(F("process..."));
          display->display();
          if (mifare_read_write_block()) {
            display->clearDisplay();
            display->setCursor(0, 0);
            display->println(F("Successful write!"));
            display->display();
          }
          break;

        default:
          Serial.println(" - Found a card, but it is not Mifare");
          display->println(F("but it is not Mifare"));
          display->display();
          break;
      }

      // It can detect multiple cards at the same time if they use the same
      // protocol
      if (nfc.remoteDevice.hasMoreTags()) {
        nfc.activateNextTagDiscovery();
        Serial.println("Multiple cards are detected!");
      }

      display->println(F("Please remove the tag"));
      display->println(F("from the antenna"));
      display->display();

      Serial.println("Remove the Card");
      nfc.waitForTagRemoval();
      Serial.println("Card removed!");
    }

    Serial.println("Restarting...");
    nfc.reset();
    Serial.println("Waiting for a Card...");
    delay(10);

    if (tagDetected) {
      break;
    }
  }

  if (tagDetected) {
    display->println(F("Press BACK button"));
    display->display();

    while (!inputController.isBackPressed()) {
      inputController.update();
      delay(10);
    }
  } else {
    Serial.println("No tag detected!");
    display->clearDisplay();
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(0, 0);
    display->println(F("No tag detected"));
    display->println(F("Press BACK to"));
    display->println(F("return to menu"));
    display->display();
  }

  nfc.reset();

  while (!inputController.isBackPressed()) {
    inputController.update();
    delay(10);
  }
}

void runMagspoofSetup() {
  Adafruit_SSD1306* display = displayController.getDisplay();
  display->clearDisplay();
  display->setTextColor(SSD1306_WHITE);
  display->setCursor(0, 0);
  display->println(F("Connect to a PC"));
  display->println(F("to update the tracks"));
  display->println(F("Press BACK to"));
  display->println(F("return to menu"));
  display->display();

  enum SetupState { WAITING_TRACK1, WAITING_TRACK2, SETUP_COMPLETE };

  SetupState currentState = WAITING_TRACK1;
  String track1 = "";
  String track2 = "";

  // Time tracking for periodic serial messages
  unsigned long lastSerialPrompt = 0;
  const unsigned long serialPromptInterval = 5000;  // 5 seconds

  // Send initial prompt
  Serial.println("Insert track 1:");
  lastSerialPrompt = millis();

  while (!inputController.isBackPressed()) {
    inputController.update();

    if (currentState != SETUP_COMPLETE) {
      unsigned long currentTime = millis();
      if (currentTime - lastSerialPrompt >= serialPromptInterval) {
        if (currentState == WAITING_TRACK1) {
          Serial.println("Insert track 1:");
        } else if (currentState == WAITING_TRACK2) {
          Serial.println("Insert track 2:");
        }
        lastSerialPrompt = currentTime;
      }

      if (Serial.available() > 0) {
        String input = Serial.readStringUntil('\n');
        input.trim();  // Remove any whitespace

        if (input.length() > 0) {
          if (currentState == WAITING_TRACK1) {
            // Save track 1 and move to track 2
            track1 = input;
            Serial.println("Track 1 received: " + track1);
            Serial.println("Insert track 2:");
            currentState = WAITING_TRACK2;
            lastSerialPrompt = millis();
          } else if (currentState == WAITING_TRACK2) {
            // Save track 2 and complete setup
            track2 = input;
            Serial.println("Track 2 received: " + track2);

            setupTracks(track1, track2);

            display->clearDisplay();
            display->setCursor(0, 0);
            display->println(F("Tracks updated!"));
            display->println(F("Press BACK to return"));
            display->display();

            currentState = SETUP_COMPLETE;
          }
        }
      }
    }

    delay(10);
  }
}

void showAbout() {
  Adafruit_SSD1306* display = displayController.getDisplay();
  display->clearDisplay();
  display->setTextColor(SSD1306_WHITE);
  display->setCursor(0, 0);
  display->println(F("Recon Badge 2025"));
  display->println(F(""));
  display->println(F("With love from Mexico"));
  display->println(F("by Electronic Cats"));
  display->display();

  // Wait for back button press
  while (!inputController.isBackPressed()) {
    inputController.update();
    delay(10);
  }
}

void showMagspoofHelp() {
  Adafruit_SSD1306* display = displayController.getDisplay();
  display->clearDisplay();
  display->setTextColor(SSD1306_WHITE);
  display->setCursor(0, 0);
  display->println(F("Emulate magnetic"));
  display->println(F("stripe or credit"));
  display->println(F("card"));
  display->display();

  // Wait for back button press
  while (!inputController.isBackPressed()) {
    inputController.update();
    delay(10);
  }
}

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  Wire.setSDA(12);
  Wire.setSCL(13);

  if (!displayController.initialize(SCREEN_WIDTH, SCREEN_HEIGHT,
                                    SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true) {
      // Don't proceed, loop forever
      delay(1000);
    }
  }
  Serial.println("Display initialized");

  inputController.initialize(BUTTON_UP_PIN, BUTTON_DOWN_PIN, BUTTON_SELECT_PIN,
                             BUTTON_BACK_PIN, BUTTON_DEBOUNCE_MS);

  menuController.initialize();

  displayController.showWelcomeScreen();
  while (true) {
    inputController.update();
    if (inputController.isUpPressed() || inputController.isDownPressed() ||
        inputController.isSelectPressed() || inputController.isBackPressed()) {
      break;
    }
    delay(10);
  }
  setupMagspoof();

  while (!initializeNfcController(nfc)) {
    Adafruit_SSD1306* display = displayController.getDisplay();
    display->clearDisplay();
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(0, 0);
    display->println(F("NFC controller"));
    display->println(F("initialization"));
    display->println(F("failed!"));
    display->display();
    delay(1000);
  }
  Serial.println("NFC controller initialized");

  if (preferences.begin("my-app")) {
    Serial.println("Preferences initialized");
  } else {
    Serial.println("Failed to initialize preferences");
  }

  int counter = preferences.getInt("counter", 1);  // default to 1
  Serial.print("Reboot count: ");
  Serial.println(counter);
  counter++;
  preferences.putInt("counter", counter);
}

void loop() {
  menuController.update();  // Update menu state based on button inputs
  menuController.render();
  delay(10);
}

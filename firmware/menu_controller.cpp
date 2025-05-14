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

bool ndefMessageReceived = false;
int currentRecordIndex = 0;
int totalRecords = 0;

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

  // Reset NFC controller first
  resetNfcController(nfc);

  // Set card reader/writer mode - required for tag detection
  if (nfc.setReaderWriterMode()) {
    display->clearDisplay();
    display->setCursor(0, 0);
    display->println(F("Error setting"));
    display->println(F("reader/writer mode"));
    display->display();
    delay(2000);
    return;
  }

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

void runDetectReaders() {
  Adafruit_SSD1306* display = displayController.getDisplay();
  display->clearDisplay();
  display->setTextColor(SSD1306_WHITE);
  display->setCursor(0, 0);
  display->println(F("Detect Readers"));
  display->println(F("Please wait..."));
  display->display();

  // Get global NFC instance - declared in firmware.ino
  extern Electroniccats_PN7150 nfc;

  // Reset NFC controller first
  resetNfcController(nfc);

  // Set card emulation mode - required for reader detection
  if (nfc.setEmulationMode()) {
    display->clearDisplay();
    display->setCursor(0, 0);
    display->println(F("Error setting"));
    display->println(F("emulation mode"));
    display->display();
    delay(2000);
    return;
  }

  display->clearDisplay();
  display->setCursor(0, 0);
  display->println(F("Waiting for reader"));
  display->println(F("Hold near a phone"));
  display->println(F("or card reader"));
  display->println(F("BACK to cancel"));
  display->display();

  // Animation dots for waiting
  uint8_t animDots = 0;
  uint8_t animFrame = 0;
  unsigned long lastAnimUpdate = 0;
  boolean readerFound = false;

  // Wait for reader detection or back button
  while (!inputController.isBackPressed()) {
    inputController.update();

    // Update animation every 500ms
    if (millis() - lastAnimUpdate > 500) {
      lastAnimUpdate = millis();
      animDots = (animDots + 1) % 4;

      // Update animation on last line
      display->fillRect(0, 24, display->width(), 8, SSD1306_BLACK);
      display->setCursor(0, 24);
      display->print(F("Scanning"));
      for (uint8_t i = 0; i < animDots; i++) {
        display->print(F("."));
      }
      display->display();

      // Check for reader detection
      if (nfc.isReaderDetected()) {
        readerFound = true;
        break;
      }
    }

    // Small delay to prevent CPU hogging
    delay(10);
  }

  // If a reader was found
  if (readerFound) {
    display->clearDisplay();
    display->setCursor(0, 0);
    display->println(F("Reader detected!"));
    display->println(F("Handling emulation"));
    display->display();

    // Handle card emulation
    nfc.handleCardEmulation();

    // Close communication
    nfc.closeCommunication();

    // Show completion message
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
  }

  // Reset NFC controller back to normal mode
  resetNfcController(nfc);
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

/**
 * @brief Callback function called when an NDEF message is received
 * This function will be called by the NFC library when a message is read
 */
void messageReceivedCallback() {
  Serial.println("NDEF message received!");
  ndefMessageReceived = true;
}

// Replace the runNdefRead function
void runNdefRead() {
  Adafruit_SSD1306* display = displayController.getDisplay();
  display->clearDisplay();
  display->setTextColor(SSD1306_WHITE);
  display->setCursor(0, 0);
  display->println(F("NDEF Read"));
  display->println(F("Initializing..."));
  display->display();

  // Get global NFC and NDEF message instances
  extern Electroniccats_PN7150 nfc;
  extern NdefMessage ndefMessage;

  // Reset the state
  ndefMessageReceived = false;
  currentRecordIndex = 0;

  // Reset NFC controller first
  resetNfcController(nfc);

  // Initialize NDEF message buffer
  ndefMessage.begin();

  // Register the callback - THIS IS THE KEY CHANGE
  nfc.setReadMsgCallback(messageReceivedCallback);

  // Set card reader/writer mode
  if (nfc.setReaderWriterMode()) {
    display->clearDisplay();
    display->setCursor(0, 0);
    display->println(F("Error setting"));
    display->println(F("reader/writer mode"));
    display->display();
    delay(2000);
    return;
  }

  display->clearDisplay();
  display->setCursor(0, 0);
  display->println(F("Waiting for tag"));
  display->println(F("with NDEF message"));
  display->println(F("BACK to cancel"));
  display->display();

  // Animation dots for waiting
  uint8_t animDots = 0;
  unsigned long lastAnimUpdate = 0;
  boolean tagFound = false;

  // Wait for tag detection or back button
  while (!inputController.isBackPressed()) {
    inputController.update();

    // Update animation every 500ms
    if (millis() - lastAnimUpdate > 500) {
      lastAnimUpdate = millis();
      animDots = (animDots + 1) % 4;

      // Update animation on last line
      display->fillRect(0, 24, display->width(), 8, SSD1306_BLACK);
      display->setCursor(0, 24);
      display->print(F("Scanning"));
      for (uint8_t i = 0; i < animDots; i++) {
        display->print(F("."));
      }
      display->display();
    }

    // Check for tag detection
    if (nfc.isTagDetected()) {
      tagFound = true;

      display->clearDisplay();
      display->setCursor(0, 0);
      display->println(F("Tag detected!"));
      display->println(F("Reading NDEF..."));
      display->display();

      // Check protocol to ensure it supports NDEF
      switch (nfc.remoteDevice.getProtocol()) {
        case nfc.protocol.T1T:
        case nfc.protocol.T2T:
        case nfc.protocol.T3T:
        case nfc.protocol.ISODEP:
        case nfc.protocol.MIFARE:
          // Reset the received flag
          ndefMessageReceived = false;

          // Just call readNdefMessage - callback will be triggered if
          // successful
          Serial.println("Waiting for callback");
          nfc.readNdefMessage();
          
          // Wait a moment to see if callback happens
          delay(500);
          Serial.println("Timeout!");

          if (ndefMessageReceived) {
            // Count the total records for navigation
            countNdefRecords(ndefMessage);

            // Show the first record
            showNdefRecordWithNavigation(ndefMessage, display,
                                         currentRecordIndex);
          } else {
            display->clearDisplay();
            display->setCursor(0, 0);
            display->println(F("No valid NDEF"));
            display->println(F("message found"));
            display->display();
          }
          break;

        default:
          display->clearDisplay();
          display->setCursor(0, 0);
          display->println(F("Tag doesn't"));
          display->println(F("support NDEF"));
          display->display();
          break;
      }

      // If we received a message, let the user navigate through records
      if (ndefMessageReceived) {
        while (!inputController.isBackPressed()) {
          inputController.update();

          // Use UP/DOWN buttons to scroll through records
          if (inputController.isUpPressed() && currentRecordIndex > 0) {
            currentRecordIndex--;
            showNdefRecordWithNavigation(ndefMessage, display,
                                         currentRecordIndex);
            delay(200);  // Debounce
          } else if (inputController.isDownPressed() &&
                     currentRecordIndex < totalRecords - 1) {
            currentRecordIndex++;
            showNdefRecordWithNavigation(ndefMessage, display,
                                         currentRecordIndex);
            delay(200);  // Debounce
          }

          delay(10);
        }
      } else {
        // Wait for back button if no message received
        while (!inputController.isBackPressed()) {
          inputController.update();
          delay(10);
        }
      }

      break;  // Exit the waiting loop
    }

    delay(10);
  }

  // If no tag was found, show appropriate message
  if (!tagFound) {
    display->clearDisplay();
    display->setCursor(0, 0);
    display->println(F("Operation canceled"));
    display->display();
    delay(1000);
  }

  // Reset NFC controller
  resetNfcController(nfc);
}

void displayRecordInfo(NdefRecord record) {
  if (record.isEmpty()) {
    Serial.println("No more records, exiting...");
    return;
  }

  uint8_t *payload = record.getPayload();
  Serial.println("--- NDEF record received:");

  switch (record.getType()) {
    case record.type.MEDIA_VCARD:
      Serial.println("vCard:");
      Serial.println(record.getVCardContent());
      break;

    case record.type.WELL_KNOWN_SIMPLE_TEXT:
      Serial.println("\tWell known simple text");
      Serial.println("\t- Text record: " + record.getText());
      break;

    case record.type.WELL_KNOWN_SIMPLE_URI:
      Serial.println("\tWell known simple URI");
      Serial.print("\t- URI record: ");
      Serial.println(record.getUri());
      break;

    case record.type.MEDIA_HANDOVER_WIFI:
      Serial.println("\tReceived WIFI credentials:");
      Serial.println("\t- SSID: " + record.getWiFiSSID());
      Serial.println("\t- Network key: " + record.getWiFiPassword());
      Serial.println("\t- Authentication type: " + record.getWiFiAuthenticationType());
      Serial.println("\t- Encryption type: " + record.getWiFiEncryptionType());
      break;

    case record.type.WELL_KNOWN_HANDOVER_SELECT:
      Serial.print("\tHandover select version: ");
      Serial.print(*payload >> 4);
      Serial.print(".");
      Serial.println(*payload & 0xF);
      break;

    case record.type.WELL_KNOWN_HANDOVER_REQUEST:
      Serial.print("\tHandover request version: ");
      Serial.print(*payload >> 4);
      Serial.print(".");
      Serial.println(*payload & 0xF);
      break;

    case record.type.MEDIA_HANDOVER_BT:
      Serial.println("\tBluetooth handover");
      Serial.println("\t- Bluetooth name: " + record.getBluetoothName());
      Serial.println("\t- Bluetooth address: " + record.getBluetoothAddress());
      break;

    case record.type.MEDIA_HANDOVER_BLE:
      Serial.print("\tBLE Handover");
      Serial.println("\t- Payload size: " + String(record.getPayloadLength()) + " bytes");
      Serial.print("\t- Payload = ");
      Serial.println(getHexRepresentation(record.getPayload(), record.getPayloadLength()));
      break;

    case record.type.MEDIA_HANDOVER_BLE_SECURE:
      Serial.print("\tBLE secure Handover");
      Serial.println("\t- Payload size: " + String(record.getPayloadLength()) + " bytes");
      Serial.print("\t- Payload = ");
      Serial.println(getHexRepresentation(record.getPayload(), record.getPayloadLength()));
      break;

    default:
      Serial.println("\tUnsupported NDEF record, cannot parse");
      break;
  }

  Serial.println("");
}

/**
 * @brief Count total NDEF records in the message
 *
 * @param ndefMessage Message to count records in
 */
void countNdefRecords(NdefMessage& ndefMessage) {
  totalRecords = 0;
  NdefRecord record;
  Serial.println("Counting records...");

  // message is automatically updated when a new NDEF message is received
  // only if we call message.begin() in setup()
  if (ndefMessage.isEmpty()) {
    Serial.println("--- Provisioned buffer size too small or NDEF message empty");
    return;
  }

  Serial.print("NDEF message: ");
  Serial.println(getHexRepresentation(ndefMessage.getContent(), ndefMessage.getContentLength()));

  // Count records
  do {
    record.create(ndefMessage.getRecord());
    // if (record.isNotEmpty()) {
      displayRecordInfo(record);
      totalRecords++;
    // }
  } while (record.isNotEmpty());

  Serial.print("Total records: ");
  // totalRecords = 3;
  Serial.println(totalRecords);
}

/**
 * @brief Show NDEF record with navigation UI
 *
 * @param ndefMessage NDEF message to display
 * @param display Pointer to display object
 * @param recordIndex Index of record to show
 */
void showNdefRecordWithNavigation(NdefMessage& ndefMessage,
                                  Adafruit_SSD1306* display,
                                  int recordIndex) {
  display->clearDisplay();
  display->setTextColor(SSD1306_WHITE);

  // Record navigation header
  display->setCursor(0, 0);
  display->print(F("Record "));
  display->print(recordIndex + 1);
  display->print(F("/"));
  display->print(totalRecords);

  // Draw separator line
  display->drawLine(0, 8, display->width(), 8, SSD1306_WHITE);

  // Skip to the desired record
  NdefRecord record;
  for (int i = 0; i <= recordIndex; i++) {
    record.create(ndefMessage.getRecord());
  }

  // Display record content
  display->setCursor(0, 10);
  displayNdefRecord(record, display);

  // Draw navigation instruction at bottom
  display->setCursor(0, 24);
  if (totalRecords > 1) {
    display->println(F("UP/DOWN:Nav BACK:Exit"));
  } else {
    display->println(F("BACK: Return to menu"));
  }

  display->display();
}

/**
 * @brief Display NDEF record information on the OLED
 *
 * @param record NdefRecord to display
 * @param display Pointer to the display object
 */
void displayNdefRecord(NdefRecord record, Adafruit_SSD1306* display) {
  if (record.isEmpty()) {
    display->println(F("Empty record"));
    return;
  }

  switch (record.getType()) {
    case record.type.WELL_KNOWN_SIMPLE_TEXT:
      display->println(F("Text:"));
      display->println(record.getText().substring(0, 20));
      break;

    case record.type.WELL_KNOWN_SIMPLE_URI:
      display->println(F("URI:"));
      display->println(record.getUri().substring(0, 20));
      break;

    case record.type.MEDIA_VCARD:
      display->println(F("vCard"));
      break;

    case record.type.MEDIA_HANDOVER_WIFI:
      display->println(F("WiFi:"));
      display->println(record.getWiFiSSID().substring(0, 20));
      break;

    case record.type.MEDIA_HANDOVER_BT:
      display->println(F("BT:"));
      display->println(record.getBluetoothName().substring(0, 20));
      break;

    default:
      display->println(F("Unsupported type"));
      break;
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

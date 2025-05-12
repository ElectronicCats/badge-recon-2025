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
#include <ezButton.h>
#include "Electroniccats_PN7150.h"

#include "nfc_config.h"
#include "nfc_controller.h"
#include "nfc_display.h"
#include "display_controller.h"
#include "input_controller.h"
#include "menu_controller.h"

// Display configuration
#define SCREEN_WIDTH   128   // OLED display width in pixels
#define SCREEN_HEIGHT  32    // OLED display height in pixels
#define OLED_RESET     -1    // Reset pin (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  // I2C address of the SSD1306 display
#define IC2_SDA_PIN    12
#define IC2_SCL_PIN    13

// Buttons configuration
#define BUTTON_UP_PIN 5
#define BUTTON_DOWN_PIN 4
#define BUTTON_SELECT_PIN 2
#define BUTTON_BACK_PIN 1
#define BUTTON_DEBOUNCE_MS 50

/**
 * @brief Global NFC device interface object
 *
 * Creates a global NFC device interface object, attached to pins defined
 * in nfc_config.h and using the specified I2C address
 */
Electroniccats_PN7150 nfc(PN7150_IRQ, PN7150_VEN, PN7150_ADDR, PN7150);

/**
 * @brief Display object for SSD1306 OLED
 */
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/**
 * @brief Button objects for user input
 */
ezButton buttonUp(BUTTON_UP_PIN);
ezButton buttonDown(BUTTON_DOWN_PIN);
ezButton buttonSelect(BUTTON_SELECT_PIN);
ezButton buttonBack(BUTTON_BACK_PIN);

/**
 * @brief Arduino setup function
 *
 * Initialize serial communication and NFC controller
 */
void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  while (!Serial) {
    ;  // Wait for serial port to connect
  }

  Serial.println("Recon Badge 2025");

  Wire.setSDA(12);
  Wire.setSCL(13);

  // Initialize NFC controller - retry if fails
  while (!initializeNfcController(nfc)) {
    delay(1000);
  }

  // Initialize display controller
  if (!displayController.initialize(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true) {
      // Don't proceed, loop forever
      delay(1000);
    }
  }

  // Initialize input controller with buttons
  inputController.initialize(
    BUTTON_UP_PIN,
    BUTTON_DOWN_PIN,
    BUTTON_SELECT_PIN,
    BUTTON_BACK_PIN,
    BUTTON_DEBOUNCE_MS
  );

  // Initialize menu system
  menuController.initialize();
  
  // Show welcome screen
  displayController.showWelcomeScreen();
  delay(2000); // Show welcome screen for 2 seconds
}

/**
 * @brief Arduino main loop
 *
 * Handle menu navigation and selected functions
 */
void loop() {
  // Update menu state based on button inputs
  menuController.update();
  
  // Render current menu
  menuController.render();
  
  // Short delay for stability
  // delay(10);
}
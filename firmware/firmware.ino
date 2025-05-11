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

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true) {
      // Don't proceed, loop forever
      delay(1000);
    }
  }

  buttonUp.setDebounceTime(BUTTON_DEBOUNCE_MS);
  buttonDown.setDebounceTime(BUTTON_DEBOUNCE_MS);
  buttonSelect.setDebounceTime(BUTTON_DEBOUNCE_MS);
  buttonBack.setDebounceTime(BUTTON_DEBOUNCE_MS);

  // Clear the display buffer
  display.clearDisplay();

  // Set text properties
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  // Display welcome message
  display.println(F("NFC Tag"));
  display.println(F("Detector"));

  // Show the display buffer on the screen
  display.display();
}

/**
 * @brief Arduino main loop
 *
 * Continuously check for NFC tags and handle them when detected
 */
void loop() {
  // Process any detected tags
  handleTagDetection(nfc);

  // Reset controller and prepare for next detection
  resetNfcController(nfc);

  // Short delay before next detection attempt
  delay(DETECTION_DELAY_MS);
}

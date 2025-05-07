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

 #include "Electroniccats_PN7150.h"
 #include "nfc_config.h"
 #include "nfc_controller.h"
 #include "nfc_display.h"
 
 /**
  * @brief Global NFC device interface object
  * 
  * Creates a global NFC device interface object, attached to pins defined
  * in nfc_config.h and using the specified I2C address
  */
 Electroniccats_PN7150 nfc(PN7150_IRQ, PN7150_VEN, PN7150_ADDR, PN7150);
 
 /**
  * @brief Arduino setup function
  * 
  * Initialize serial communication and NFC controller
  */
 void setup() {
   Serial.begin(SERIAL_BAUD_RATE);
   while (!Serial) {
     ; // Wait for serial port to connect
   }
   
   Serial.println("Detect NFC tags with PN7150/60");
   
   // Initialize NFC controller - retry if fails
   while (!initializeNfcController(nfc)) {
     delay(1000);
   }
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
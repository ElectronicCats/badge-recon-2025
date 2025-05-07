/**
 * @file nfc_controller.h
 * @brief NFC controller management functions
 * @author Francisco Torres - Electronic Cats
 * @date May 2025
 * 
 * This file contains functions for initializing and managing the NFC controller.
 */

 #ifndef NFC_CONTROLLER_H
 #define NFC_CONTROLLER_H
 
 #include <Arduino.h>
 #include "Electroniccats_PN7150.h"
 
 /**
  * @brief Initialize the NFC controller
  * 
  * Sets up the NFC controller with proper configuration
  * 
  * @param nfc Reference to NFC controller object
  * @return bool true if initialization is successful, false otherwise
  */
 bool initializeNfcController(Electroniccats_PN7150 &nfc);
 
 /**
  * @brief Handle tag detection process
  * 
  * @param nfc Reference to NFC controller object
  * @return bool true if a tag was detected and handled, false otherwise
  */
 bool handleTagDetection(Electroniccats_PN7150 &nfc);
 
 /**
  * @brief Reset the NFC controller for next detection cycle
  * 
  * @param nfc Reference to NFC controller object
  */
 void resetNfcController(Electroniccats_PN7150 &nfc);
 
 #endif // NFC_CONTROLLER_H
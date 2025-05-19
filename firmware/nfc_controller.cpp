/**
 * @file nfc_controller.cpp
 * @brief Implementation of NFC controller management functions
 * @author Francisco Torres - Electronic Cats - electroniccats.com
 * @date May 2025
 */

#include "nfc_controller.h"
#include "nfc_config.h"
#include "nfc_display.h"

bool initializeNfcController(Electroniccats_PN7150& nfc) {
  Serial.println("Initializing...");

  // Wake up the NFC board
  if (nfc.connectNCI()) {
    Serial.println("Error while setting up the mode, check connections!");
    return false;
  }

  // Configure NFC settings
  if (nfc.configureSettings()) {
    Serial.println("The Configure Settings has failed!");
    return false;
  }

  // Set Read/Write mode as default
  if (nfc.configMode()) {
    Serial.println("The Configure Mode has failed!!");
    return false;
  }

  // Start NCI Discovery mode
  nfc.startDiscovery();
  return true;
}

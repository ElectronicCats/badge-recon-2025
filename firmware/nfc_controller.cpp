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

bool handleTagDetection(Electroniccats_PN7150& nfc) {
  if (!nfc.isTagDetected()) {
    return false;
  }

  // Display information about detected tag(s)
  displayCardInfo(nfc);

  // Handle multiple cards with same protocol
  if (nfc.remoteDevice.hasMoreTags()) {
    nfc.activateNextTagDiscovery();
    Serial.println("Multiple cards are detected!");
  }

  // Wait for card removal
  Serial.println("Remove the Card");
  nfc.waitForTagRemoval();
  Serial.println("Card removed!");

  return true;
}

void resetNfcController(Electroniccats_PN7150& nfc) {
  Serial.println("Restarting...");
  nfc.reset();
  Serial.println("Waiting for a Card...");
}
/**
 * @file nfc_display.cpp
 * @brief Implementation of NFC tag display functions
 * @author Francisco Torres - Electronic Cats - electroniccats.com
 * @date May 2025
 */

#include "nfc_display.h"

String getHexRepresentation(const byte* data, const uint32_t numBytes) {
  String hexString;

  if (numBytes == 0) {
    hexString = "null";
    return hexString;
  }

  for (uint32_t szPos = 0; szPos < numBytes; szPos++) {
    hexString += "0x";
    // Add leading zero for values less than 0x10
    if (data[szPos] <= 0xF)
      hexString += "0";
    hexString += String(data[szPos] & 0xFF, HEX);
    // Add space between bytes except after the last byte
    if ((numBytes > 1) && (szPos != numBytes - 1)) {
      hexString += " ";
    }
  }
  return hexString;
}

/**
 * @brief Display NFC-A specific tag information
 *
 * @param nfc Reference to NFC controller object
 */
void displayNfcAInfo(Electroniccats_PN7150& nfc) {
  Serial.println("\tTechnology: NFC-A");
  Serial.print("\tSENS RES = ");
  Serial.println(getHexRepresentation(nfc.remoteDevice.getSensRes(),
                                      nfc.remoteDevice.getSensResLen()));

  Serial.print("\tNFC ID = ");
  Serial.println(getHexRepresentation(nfc.remoteDevice.getNFCID(),
                                      nfc.remoteDevice.getNFCIDLen()));

  Serial.print("\tSEL RES = ");
  Serial.println(getHexRepresentation(nfc.remoteDevice.getSelRes(),
                                      nfc.remoteDevice.getSelResLen()));
}

/**
 * @brief Display NFC-B specific tag information
 *
 * @param nfc Reference to NFC controller object
 */
void displayNfcBInfo(Electroniccats_PN7150& nfc) {
  Serial.println("\tTechnology: NFC-B");
  Serial.print("\tSENS RES = ");
  Serial.println(getHexRepresentation(nfc.remoteDevice.getSensRes(),
                                      nfc.remoteDevice.getSensResLen()));

  Serial.println("\tAttrib RES = ");
  Serial.println(getHexRepresentation(nfc.remoteDevice.getAttribRes(),
                                      nfc.remoteDevice.getAttribResLen()));
}

/**
 * @brief Display NFC-F specific tag information
 *
 * @param nfc Reference to NFC controller object
 */
void displayNfcFInfo(Electroniccats_PN7150& nfc) {
  Serial.println("\tTechnology: NFC-F");
  Serial.print("\tSENS RES = ");
  Serial.println(getHexRepresentation(nfc.remoteDevice.getSensRes(),
                                      nfc.remoteDevice.getSensResLen()));

  Serial.print("\tBitrate = ");
  Serial.println((nfc.remoteDevice.getBitRate() == 1) ? "212" : "424");
}

/**
 * @brief Display NFC-V specific tag information
 *
 * @param nfc Reference to NFC controller object
 */
void displayNfcVInfo(Electroniccats_PN7150& nfc) {
  Serial.println("\tTechnology: NFC-V");
  Serial.print("\tID = ");
  Serial.println(getHexRepresentation(nfc.remoteDevice.getID(),
                                      sizeof(nfc.remoteDevice.getID())));

  Serial.print("\tAFI = ");
  Serial.println(nfc.remoteDevice.getAFI());

  Serial.print("\tDSF ID = ");
  Serial.println(nfc.remoteDevice.getDSFID(), HEX);
}

void displayCardInfo(Electroniccats_PN7150& nfc) {
  // Loop to handle multiple cards if present
  while (true) {
    // Display protocol information
    switch (nfc.remoteDevice.getProtocol()) {
      case nfc.protocol.T1T:
      case nfc.protocol.T2T:
      case nfc.protocol.T3T:
      case nfc.protocol.ISODEP:
        Serial.print(" - POLL MODE: Remote activated tag type: ");
        Serial.println(nfc.remoteDevice.getProtocol());
        break;
      case nfc.protocol.ISO15693:
        Serial.println(" - POLL MODE: Remote ISO15693 card activated");
        break;
      case nfc.protocol.MIFARE:
        Serial.println(" - POLL MODE: Remote MIFARE card activated");
        break;
      default:
        Serial.println(" - POLL MODE: Undetermined target");
        return;
    }

    // Display technology-specific information
    switch (nfc.remoteDevice.getModeTech()) {
      case (nfc.tech.PASSIVE_NFCA):
        displayNfcAInfo(nfc);
        break;

      case (nfc.tech.PASSIVE_NFCB):
        displayNfcBInfo(nfc);
        break;

      case (nfc.tech.PASSIVE_NFCF):
        displayNfcFInfo(nfc);
        break;

      case (nfc.tech.PASSIVE_NFCV):
        displayNfcVInfo(nfc);
        break;

      default:
        break;
    }

    // Handle multiple cards
    if (nfc.remoteDevice.hasMoreTags()) {
      Serial.println("Multiple cards are detected!");
      if (!nfc.activateNextTagDiscovery()) {
        break;  // Can't activate next tag
      }
    } else {
      break;
    }
  }
}

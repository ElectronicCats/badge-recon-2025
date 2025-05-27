/**
 * @file nfc_display.h
 * @brief Functions for displaying NFC tag information
 * @author Francisco Torres - Electronic Cats - electroniccats.com
 * @date May 2025
 *
 * This file contains utility functions for formatting and displaying
 * information about detected NFC tags.
 */

#ifndef NFC_DISPLAY_H
#define NFC_DISPLAY_H

#include <Arduino.h>
#include "Electroniccats_PN7150.h"

/**
 * @brief Convert byte array to formatted hexadecimal string
 *
 * @param data Pointer to byte array
 * @param numBytes Number of bytes in the array
 * @return String Formatted hexadecimal representation
 */
String getHexRepresentation(const byte* data, const uint32_t numBytes);

/**
 * @brief Display detailed information about detected NFC card(s) via Serial
 *
 * @param nfc Reference to NFC controller object
 */
void displayCardInfo(Electroniccats_PN7150& nfc);

/**
 * @brief Get NFC tag information as a display-friendly string
 * 
 * @param nfc Reference to NFC controller object
 * @return String Tag information formatted for display
 */
String getTagInfoForDisplay(Electroniccats_PN7150& nfc);

#endif  // NFC_DISPLAY_H
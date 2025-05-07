/**
 * @file nfc_display.h
 * @brief Functions for displaying NFC tag information
 * @author Francisco Torres - Electronic Cats
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
 * @brief Display detailed information about detected NFC card(s)
 *
 * @param nfc Reference to NFC controller object
 */
void displayCardInfo(Electroniccats_PN7150& nfc);

#endif  // NFC_DISPLAY_H
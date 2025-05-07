/**
 * @file nfc_config.h
 * @brief Configuration constants for the NFC reader
 * @author Francisco Torres - Electronic Cats
 * @date May 2025
 * 
 * This file contains all hardware-specific configuration parameters
 * for the NFC reader.
 */

 #ifndef NFC_CONFIG_H
 #define NFC_CONFIG_H
 
 /**
  * @brief PIN definitions for PN7150/PN7160
  */
 #define PN7150_IRQ (11)  ///< IRQ pin connection
 #define PN7150_VEN (13)  ///< VEN pin connection 
 #define PN7150_ADDR (0x28)  ///< I2C address of the device
 
 /**
  * @brief Serial monitor configuration
  */
 #define SERIAL_BAUD_RATE (9600)  ///< Baud rate for serial communication
 
 /**
  * @brief Timing configurations
  */
 #define DETECTION_DELAY_MS (500)  ///< Delay between detection attempts
 
 #endif // NFC_CONFIG_H
/**
 * @file input_controller.h
 * @brief Button input controller for the badge
 * @author Francisco Torres - Electronic Cats - electroniccats.com
 * @date May 2025
 */

#ifndef INPUT_CONTROLLER_H
#define INPUT_CONTROLLER_H

#include <ezButton.h>

class InputController {
 public:
  /**
   * @brief Constructor with default pin values
   * These will be overridden in initialize()
   */
  InputController();
  
  /**
   * @brief Initialize the buttons
   * 
   * @param upPin Pin for up button
   * @param downPin Pin for down button
   * @param selectPin Pin for select button
   * @param backPin Pin for back button
   * @param debounceTime Debounce time in ms
   */
  void initialize(uint8_t upPin, uint8_t downPin, uint8_t selectPin, 
                 uint8_t backPin, uint32_t debounceTime);
  
  /**
   * @brief Update button states, call in main loop
   */
  void update();
  
  /**
   * @brief Check if up button was pressed
   * 
   * @return bool true if up button was pressed
   */
  bool isUpPressed();
  
  /**
   * @brief Check if down button was pressed
   * 
   * @return bool true if down button was pressed
   */
  bool isDownPressed();
  
  /**
   * @brief Check if select button was pressed
   * 
   * @return bool true if select button was pressed
   */
  bool isSelectPressed();
  
  /**
   * @brief Check if back button was pressed
   * 
   * @return bool true if back button was pressed
   */
  bool isBackPressed();

 private:
  ezButton _upButton;
  ezButton _downButton;
  ezButton _selectButton;
  ezButton _backButton;
};

extern InputController inputController;

#endif // INPUT_CONTROLLER_H
/**
 * @file input_controller.cpp
 * @brief Implementation of button input controller
 * @author Francisco Torres - Electronic Cats - electroniccats.com
 * @date May 2025
 */

#include "input_controller.h"

// Create global instance
InputController inputController;

// Constructor uses temporary pin values that will be replaced in initialize()
InputController::InputController() : 
  _upButton(0),
  _downButton(0),
  _selectButton(0),
  _backButton(0)
{
  // Pins will be properly initialized in initialize() method
}

void InputController::initialize(uint8_t upPin, uint8_t downPin, 
                                uint8_t selectPin, uint8_t backPin,
                                uint32_t debounceTime) {
  // Re-initialize buttons with the correct pins
  _upButton = ezButton(upPin);
  _downButton = ezButton(downPin);
  _selectButton = ezButton(selectPin);
  _backButton = ezButton(backPin);
  
  _upButton.setDebounceTime(debounceTime);
  _downButton.setDebounceTime(debounceTime);
  _selectButton.setDebounceTime(debounceTime);
  _backButton.setDebounceTime(debounceTime);
}

void InputController::update() {
  _upButton.loop();
  _downButton.loop();
  _selectButton.loop();
  _backButton.loop();
}

bool InputController::isUpPressed() {
  return _upButton.isPressed();
}

bool InputController::isDownPressed() {
  return _downButton.isPressed();
}

bool InputController::isSelectPressed() {
  return _selectButton.isPressed();
}

bool InputController::isBackPressed() {
  return _backButton.isPressed();
}
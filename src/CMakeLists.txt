set(SRCS "main.c")

set(REQUIRES "bluepad32")

# See: https://gitlab.com/ricardoquesada/bluepad32/-/issues/9
if(DEFINED ENV{BLUEPAD32_ARDUINO})
    list(APPEND SRCS "arduino_main.cpp")
    list(APPEND REQUIRES "bluepad32_arduino" "framework-arduinoespressif32")
endif()

idf_component_register(
    SRCS "wifi_manager.cpp" "motor_control.cpp" "state_manager.cpp" "${SRCS}" 
    INCLUDE_DIRS "../include"
    REQUIRES "${REQUIRES}")



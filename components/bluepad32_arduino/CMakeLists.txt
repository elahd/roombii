# See: https://gitlab.com/ricardoquesada/bluepad32/-/issues/9
if(DEFINED ENV{BLUEPAD32_ARDUINO})
    set(srcs "ArduinoBluepad32.cpp"
            "ArduinoConsole.cpp"
            "ArduinoGamepad.cpp")

    set(requires "bluepad32" "framework-arduinoespressif32")

    idf_component_register(SRCS ${srcs}
                        INCLUDE_DIRS "include"
                        REQUIRES ${requires})
endif()

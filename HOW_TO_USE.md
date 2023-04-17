Posted to [Unijoysticle\bluepad32](https://discord.com/channels/775177861665521725/775177925938642945) on Discord to explain how to use this as a base for a blank bluepad32/platformio project.

@HiperDoo#3502: Take a look at https://github.com/elahd/roombii. This is a project that lets my 3 year old control a Roomba using a Wiimote as a controller and an Adafruit Feather ESP32v2 as the Roomba's new brains.

**Here's the rundown:**

1. This is set up as a PlatformIO Arduino + ESP-IDF project, so you can use libraries from both platforms and tap into both Arduino and FreeRTOS APIs.

2. Get my code to build to confirm that your environment is set up correctly before you make changes.

3. Do not install ESP-IDF or PlatformIO outside of VS Code. Let VS Code's PlatformIO extension manage this for you.

4. You should run `pio pkg update` and `pio run` from the project root before trying to build. You may get build errors otherwise. PIO will also fail to build if your folder is not a git repository.

**When merging in your own code:**

1. Don't mess with the `components` folder.

2. Wipe the `includes` folder and replace with your own header files. You'll need, at a minimum, an `arduino_main.h` that keeps these includes: `<Arduino.h>`, `<Bluepad32.h>`, `<esp_adc_cal.h>`, and `<esp_log.h>`.

3. In `src/CMakeLists.txt`, remove the roombi-specific .cpp files in `idf_component_register` and replace with your own (as needed). Don't mess with the rest of the file!

4. Change the project name in the *root* folder's `CMakeLists.txt`. Don't mess with the rest of the file.

5. Edit `platformio.ini` as needed but don't change `platform`, `board_build.partitions`, `framework`, `lib_ldf_mode`, or `build_flags` parameters.

6. You will probably need to edit `partitions.csv` and `sdkconfig.defaults` to match your application and device's storage capacity. You need to have a `partitions.csv` file with the same partitions as mine for this to build correctly. You'll need to change the sizes of the `app0` and `app1` partitions. You may be able to get away with just one application partition if you don't need OTA updates, but you may need to make other changes.

I have plans to release a blank PlatformIO/bluepad32 template project, but I had another kid in September and got distracted. I haven't touched this code base since (other than clean up / pushing to a new repo to answer your request). I can confirm that it still builds on both MacOS and Windows, but my memory is hazy on exactly how I got this working in the first place.

From what I remember, I referenced PlatformIO's dual-platform example (https://github.com/platformio/platform-espressif32/tree/master/examples/espidf-arduino-blink) and bluepad32's configuration Option C (https://gitlab.com/ricardoquesada/bluepad32/-/blob/main/docs/plat_arduino.md), along with weeks of trial and error.

Also, I *think* the magic that gets this working lives in `sdkconfigs.defaults` and the `CMakeLists.txt` files... but don't hold me to this.

With that in mind, happy to answer questions you have getting this running, but I may respond on a delay.
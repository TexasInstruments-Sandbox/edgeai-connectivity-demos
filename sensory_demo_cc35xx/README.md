# Example Summary

This example showcases the capabilities of Sensory's TrulyHandsfree-Micro library by implementing its voice wake word and command functionality.
Specifically, this integrates the TrulyHandsfree-Micro library to enable voice-activated control over the Red, Blue, and Green LEDs on the 
LP-EM-CC35X1 LaunchPad. By using voice commands, users can toggle the state of these LEDs, effectively creating an interactive experience
that combines voice interaction with physical output. This example serves as an example for you to expand beyond blinking the LEDs for 
applications such as controlling an electronic door lock, appliances, or even in automotive commands.

# Board Resources

This example requires the following hardware

- [LP-EM-CC35X1](https://www.ti.com/tool/LP-EM-CC35X1)
- [Adafruit I2S MEMS microphone Breakout Borad](https://www.adafruit.com/product/3421)
- Connect the following between the microphone breakout board to the LaunchPad

  - SEL - Connect to GND
  - LRCL/WS - Board Pin 37
  - DOUT/SDO - Board Pin 26
  - BCLK/SCK - Board Pin 38
  - GND - GND
  - 3V  - 3.3V
 


## Getting Started

- Download [Code Composer Studio 20.4.X](https://www.ti.com/tool/download/CCSTUDIO)
- Download gcc_arm_none_eabi_10_3_1 compiler or newer and install in c:\ti folder
- Refresh the Compiler list within CCS - Window - Preferences - Code Composer Studio - Build - Compilers
- Get [simplelink_wifi_sdk v9_21_00_15](https://www.ti.com/tool/download/SIMPLELINK-WIFI-SDK) SDK from Texas Instruments
- Clone the [edgeai-connectivity-demo](https://github.com/TexasInstruments-Sandbox/edgeai-connectivity-demos) project GIT repository
- Go to wakeword_demo_cc35xx project folder
- Import project to Code Composer Studio
- Build and download the project

## Example Usage
- Start speaking naturally with the wake-word which is "voice genie"
- You have approximately 3-seconds to issue a voice command before it timeout and return requiring the wake-word again.
- The commands are "toggle green led", "toggle red led", "toggle blue led", or "toggle all led"
- You can toggle the green, red, and blue LEDs individually, or all of them at once.

## Licensing and Usage Limits
*** IMPORTANT ***
- The included libraries enforce event/usage limits and are intended for development purposes only. 
- The SDK returns error code ERR_LICENSE (0xFF) when a limit is reached. 
- Restarting the device will reset the event limits. 
- Event limits include:
  - Duration limit: 11.43 hours
  - Number of Recognitions limit: 107
    
## How to customize for your application

Reach out to Sensory at [sales@sensory.com](mailto:sales@sensory.com) to find out more on getting access to Voicehub to tailor your commands.



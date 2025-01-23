# Example Summary

This example showcases the capabilities of Sensory's TrulyHandsfree-Micro library by implementing its voice wake word and command functionality.
Specifically, this integrates the TrulyHandsfree-Micro library to enable voice-activated control over the Red and Green LEDs on the 
LP-EM-CC2745R10-Q1 LaunchPad. By using voice commands, users can toggle the state of these LEDs, effectively creating an interactive experience
that combines voice interaction with physical output. This example serves as an example for you to expand beyond blinking the LEDs for 
applications such as controlling an electronic door lock, appliances, or even in automotive commands.

# Board Resources

This example requires the following hardware

- [LP-EM-CC2745R10-Q1](https://www.ti.com/tool/LP-EM-CC2745R10-Q1)
- [Adafruit I2S MEMS microphone Breakout Borad](https://www.adafruit.com/product/3421)
- Connect the following between the microphone breakout board to the LaunchPad

  - SEL - Connect to GND
  - LRCL/WS - DIO18
  - DOUT/SDO - DIO19
  - BCLK/SCK - DIO17
  - GND - GND
  - 3V  - 3.3V
 


## Getting Started

- Download [Code Composer Studio 12.8.1.0](https://www.ti.com/tool/download/CCSTUDIO/12.8.1)
- Get [simplelink_lowpower_f3_sdk_8.40.00.61](https://www.ti.com/tool/download/SIMPLELINK-LOWPOWER-F3-SDK) SDK from Texas Instruments
- Clone the [edgeai-connectivity-demo](https://github.com/TexasInstruments-Sandbox/edgeai-connectivity-demos) project GIT repository
- Go to sensory_demo_cc27xx project folder
- Import project to Code Composer Studio
- Build and download the project

## Example Usage
- Start speaking naturally with the wake-word which is "voice genie"
- You have approximately 3-seconds to issue a voice command before it timeout and return requiring the wake-word again.
- The commands are "toggle green led" or "toggle red led"
- The green or the red LEDs shall then toggle.

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



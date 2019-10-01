# note-stm32f1

This is an example of how to use the Notecard and the [note-c][note-c] library
with the native STM32 SDK.

The board that I used for testing is the [Nucleo-F103RB][board] kit with STM32F103RB MCU, the MCU powering
the popular "blue pill" development board.

As a proof-of-concept, it implements the same functions as the [note-arduino][note-arduino] library's JSON
example.

## Hardware Setup
Before you begin using the software, wire your Nucleo development board to the Notecarrier containing
the Notecard.  You may wire it for Serial, I2C, or both.  In order to do so, you'll need some standard female-to-female
header wires to jumper between the boards.  For reference, the Nucleo's user manual is [here][reference-manual]
- Connect the Notecarrier's GND pin to any of the Nucleo's GND pins
- For Serial
  - Connect the Notecarrier's RX to the Nucleo's D8 pin (USART1_TX PA9)
  - Connect the Notecarrier's TX to the Nucleo's D2 pin (USART1_RX PA10)
- For I2C
  - Connect the Notecarrier's SDA pin to the Nucleo's D14 pin (I2C1_SDA PB9)
  - Connect the Notecarrier's SCL pin to the Nucleo's D15 pin (I2C1_SCL PB8)
- Connect both the Notecarrier and Nucleo to power by using their USB connectors to connect them to your development machine.

## Installation of the STMicroelectronics Integrated Development Environment

In order to do development with this example, you will need to download and install STMicroelectronics's free IDE
called [STM32CubeIDE][ide].  During installation it will ask you to specify a directory to contain your projects.
Remember this directory path, because that's where you'll download this example.

## Installation of this example

Clone this [note-stm32f1][note-stm32f1] repo into the projects folder that you selected during IDE
installation, as follows.  You'll note that the latest copy of the [note-c][note-c] C library is already
loaded by default, as a subdirectory of the [note-stm32f1][note-stm32f1] directory.

```
STM32CubeIDE projects folder
├ note-stm32f1
  ├ note-c  
```

In STM32CubeIDE, open the [note-stm32f1][note-stm32f1] project.  Make sure that you edit the "my" definitions
at the top of example.c so that this example will send data to your notehub.io project, and so that it uses
serial or I2C as you wish.  By using the standard Debug build configuration, you should be able to build and run the project.

This example has been tested with both UART and with I2C.

[note-stm32f1]: https://github.com/blues/note-stm32f1
[note-c]: https://github.com/blues/note-c
[note-arduino]: https://github.com/blues/note-arduino
[board]: https://www.st.com/en/evaluation-tools/nucleo-f103rb.html
[reference-manual]: https://www.st.com/resource/en/user_manual/dm00105823.pdf
[ide]: https://www.st.com/en/development-tools/stm32cubeide.html

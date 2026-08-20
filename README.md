# IMU Mouse

IMU Mouse is an experimental Arduino project that turns the tilt of an
ADXL335 three-axis accelerometer into USB mouse movement. A push button provides
left-click and click-and-drag input, while startup calibration adapts movement
to the user's neutral position and comfortable range of motion.

## Features

- Moves the pointer horizontally, vertically, and diagonally from roll and pitch
- Calibrates the neutral position and maximum tilt in four directions
- Smooths accelerometer readings with a 10-sample moving average
- Uses an 8-degree dead zone to reduce unintended pointer movement
- Supports left-click and click-and-drag
- Pauses pointer movement after the device has remained still for a while
- Prints roll, pitch, and calibration values to the serial monitor for debugging

## Hardware

- An Arduino-compatible board with native USB HID support, such as an Arduino
  Leonardo or Micro
- ADXL335 analog accelerometer
- Momentary push button
- LED and a suitable current-limiting resistor, unless using an onboard LED
- Jumper wires and a breadboard or equivalent wiring

> [!WARNING]
> The sketch uses Arduino's `Mouse` library and can take control of the pointer
> as soon as calibration finishes. Keep a way to disconnect or reset the board
> while testing. Boards without native USB support, such as a standard Uno,
> cannot use `Mouse` directly.

## Wiring

The pin assignments are defined near the top of `IMUmouse1_1.ino`.

| Component | Arduino pin |
| --- | --- |
| ADXL335 X output | `A3` |
| ADXL335 Y output | `A2` |
| ADXL335 Z output | `A1` |
| Left-click button | `14` |
| Status LED | `16` |

The button input is configured as `INPUT`, not `INPUT_PULLUP`. The existing
logic expects the pin to read `HIGH` while the button is pressed, so wire it
with an appropriate pull-down resistor. Connect the accelerometer power and
ground according to the sensor module's requirements. The sketch configures
the ADXL335 library with a 3.3 V reference.

Pin `8` is named `resetButton` in the sketch but is not currently read or
configured.

## Software requirements

- Arduino IDE or Arduino CLI
- A board package for the selected native-USB Arduino board
- Arduino `Mouse` library (included with supported Arduino cores)
- An `ADXL335` library that provides `ADXL335.h`, `update()`, `getX()`,
  `getY()`, and `getZ()`

## Uploading

1. Open `IMUmouse1_1.ino` in the Arduino IDE.
2. Install the required ADXL335 library through Library Manager or from its
   source repository.
3. Select the correct native-USB board and serial port.
4. Verify and upload the sketch.
5. If desired, open the Serial Monitor at **9600 baud** to observe calibration
   and angle values.

If the compiler cannot find `Mouse.h`, confirm that the selected board supports
native USB HID. If it cannot find `ADXL335.h`, install a compatible ADXL335
library or adapt the sensor calls to the API of the library you use.

## Calibration and use

Keep the accelerometer still in its neutral position immediately after startup.
The sketch discards the first 10 loop iterations, averages the next 50 samples,
and then turns the status LED on for one second. After that, use the click
button to save four comfortable maximum-tilt positions in this order:

1. Tilt right, then press and release the button.
2. Tilt left, then press and release the button.
3. Tilt forward, then press and release the button.
4. Tilt backward, then press and release the button.

Mouse control begins after the fourth position is captured. Tilt beyond the
dead zone to move the pointer. Press and release the button for a click; hold it
while moving for a drag.

Calibration values live only in memory and are lost when the board resets or
loses power. Repeat the full sequence after every restart.

## How it works

Each loop reads the three analog accelerometer channels and applies a moving
average. The sketch subtracts the neutral offsets, calculates roll and pitch
from the gravity vector, and maps those angles against the four calibrated
limits. `Mouse.move()` sends relative USB HID motion rather than absolute screen
coordinates.

The main tuning constants are near the top of the sketch:

| Constant | Default | Purpose |
| --- | ---: | --- |
| `numReadings` | `10` | Samples used by the moving average |
| `degreeOffset` | `8` | Tilt dead zone in degrees |
| `MOUSERATE` | `25` | Base cursor movement rate |
| `Scale` | `2` | Divides the generated movement speed |
| `RESTCOUNT` | `250` | Stable loops before rest mode activates |
| `debugDelay` | `10 ms` | Delay at the end of each main loop |
| `responseDelay` | `5 ms` | Extra delay after pointer movement |

Increase `degreeOffset` if the cursor drifts around the neutral position.
Increase `MOUSERATE` or decrease `Scale` for faster movement. Change one value
at a time and recalibrate after uploading.

## Current limitations

- Calibration is not saved to EEPROM or other persistent storage.
- There is no implemented hardware control to disable USB mouse output.
- The analog conversion includes a fixed `5 / 3.3` adjustment and may need to
  be changed for boards with a different ADC reference or input range.
- Direction and axis signs depend on how the accelerometer is mounted.
- The sketch has no scroll-wheel or right-click input.
- Serial debug output is always enabled and may affect loop timing.

## Project files

- `IMUmouse1_1.ino` — Arduino firmware containing sensor processing,
  calibration, and mouse control
- `README.md` — setup and usage documentation

## License

No license has been specified. Add a license file before redistributing or
reusing the project outside the terms allowed by copyright law.

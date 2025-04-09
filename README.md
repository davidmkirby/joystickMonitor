# Joystick Mirror Controller

A Qt-based application for controlling fast steering mirrors through Advantech D/A cards using joystick input with comprehensive testing and analysis tools.

## Overview

The Joystick Mirror Controller provides an interface for controlling fast steering mirrors via Advantech Digital-to-Analog (D/A) converter cards using joystick inputs. It features real-time visualization, signal generation capabilities, and data logging for characterizing system responses.

## Features

### Joystick Input
- Real-time detection of joystick buttons, axes, and hat inputs
- Joystick calibration to correct for drift and center offsets
- Configurable axis mapping and deadzone settings
- Support for multiple joystick types via SDL2

### D/A Card Control
- Support for Advantech D/A cards (PCIE-1816, PCIE-1824, etc.)
- XML profile support for device configuration
- Real-time voltage feedback
- Configurable axis mapping and inversion

### Sine Wave Testing
- Generate precise sine waves with configurable parameters:
  - Frequency control (1-1000 Hz)
  - Amplitude control (0.01-1.0)
  - Phase offset between X and Y axes (0-359°)
- Real-time visualization with oscilloscope-style display
- Independent X/Y axis control
- Create circular/elliptical patterns with phase offsets

### Data Logging
- CSV-based data logging for analysis
- High-precision timing information
- Captures commanded positions and actual feedback voltages
- Suitable for frequency response and latency characterization

## Requirements

- Qt6
- SDL2 for joystick support
- Advantech DAQNavi SDK (for hardware support)
- C++17 compatible compiler

## Building the Application

### Using CMake

```bash
mkdir build && cd build
cmake ..
make
```

### Using Qt Creator

1. Open the `CMakeLists.txt` file in Qt Creator
2. Configure the project for your preferred build configuration
3. Build the project using the build button or shortcut (Ctrl+B)

## Hardware Setup

### Supported Hardware

- Joysticks: Any SDL2-compatible joystick or gamepad
- D/A Cards: Advantech PCIE-1816, PCIE-1824, or other compatible analog output cards
- Fast Steering Mirrors: Any model compatible with analog voltage input

### Connection Diagram

```
+----------+      +-------------+      +------------+      +----------------+
| Joystick |----->| Application |----->| D/A Card   |----->| Fast Steering  |
+----------+      +-------------+      +------------+      | Mirror         |
                         |                                 +----------------+
                         v
                  +-------------+
                  | Data Logger |
                  +-------------+
```

## Using the Application

### Joystick Tab

1. Select your joystick from the dropdown menu
2. Press "Calibrate" to set the current position as center
3. Test button and axis inputs using the visual interface
4. Use button 3 to quickly toggle D/A output

### Mirror Control Tab

1. Select your Advantech D/A card from the dropdown
2. Optionally load an XML profile for device-specific settings
3. Map joystick axes to D/A output channels
4. Configure deadzone and inversion settings
5. Enable D/A output when ready to control the mirror

### Sine Wave Test Tab

1. Set the desired frequency, amplitude, and phase offset
2. Select which axes to output (X, Y, or both)
3. Press "Start Sine Wave" to begin generation
4. Observe the real-time waveform display
5. Optionally enable data logging to capture response data

## Data Analysis

The CSV log files contain the following columns:
- Time(s): Elapsed time in seconds
- Frequency(Hz): Current frequency setting
- Amplitude: Current amplitude setting (0.0-1.0)
- X-Command: Commanded X position (-1.0 to 1.0)
- Y-Command: Commanded Y position (-1.0 to 1.0)
- X-Feedback(V): Actual X voltage from D/A card
- Y-Feedback(V): Actual Y voltage from D/A card

This data can be analyzed to:
- Calculate system latency
- Determine frequency response characteristics
- Identify resonance issues in the mirror
- Assess positioning accuracy and repeatability

## Common Use Cases

### Frequency Response Testing
1. Set sine wave to desired amplitude (typically 0.5)
2. Perform frequency sweep from low to high values
3. Log data throughout the sweep
4. Analyze amplitude and phase response of the mirror

### Latency Testing
1. Generate square waves at low frequency (1-5 Hz)
2. Log data at high sampling rate
3. Measure time difference between command and feedback changes

### Circular Testing
1. Enable both X and Y axes
2. Set phase offset to 90°
3. Adjust frequency to test circular motion capability of the mirror
4. Observe if circle becomes elliptical at higher frequencies

## Troubleshooting

### Joystick Not Detected
- Ensure SDL2 is properly installed
- Disconnect and reconnect the joystick
- Try a different USB port
- Press the "Refresh" button to scan for newly connected devices

### D/A Card Not Found
- Check device connections
- Verify Advantech drivers are installed
- Make sure the correct DAQNavi SDK version is installed
- Restart the application after connecting the device

### No Mirror Movement
- Verify the D/A card is selected and enabled
- Check XML profile compatibility
- Ensure joystick mapping is configured correctly
- Verify voltage ranges are appropriate for your mirror
- Check physical connections between the D/A card and mirror

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Advantech for DAQNavi SDK
- SDL2 team for joystick support
- Qt for the application framework
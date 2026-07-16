# Meter - 2nd version

## Sim card

Vodafone offers free prepaid SIM cards with free delivery. They stay active for 6 months without any outgoing activity.

## Installation

- Install the PlatformIO extension in VSC.
- Everything should setup automatically.
- If intelhex is missing, install like `.platformio\penv\Scripts\python.exe -m pip install intelhex`.
- In `audio-tools/src/AudioTools/VolumeStream.h`, `line 19`, set ``volumeConfig.allow_boost`` to `true`.
- To upload, start holding the power button already before uploading starts, and keep holding it until is finished.

## Commands

Commands can be found in the Excel file and can be sent via:

- Serial: install the Serial Monitor extension in VSC, set baudrate to `9600`, set line ending to `None`. Send for example  `com Now`.
- SMS: `Now`

## Available components

SIM card / antenna, LED strip, speaker / bone inducer, GPS module, battery, solar panel and SD card.

## Current functionality

Games, audio playback, TTS (Text To Speech), FFT (Fast Fourier Transform)-based visuals, battery monitoring, telephone number management, testing.

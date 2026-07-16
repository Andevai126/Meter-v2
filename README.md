# Meter - 2nd version

## Sim card

Vodafone has free prepaid, free delivery sim cards.

## Installation

- Install platformio extension in VSC.
- Everything should setup automatically.
- If intelhex is missing, install like `.platformio\penv\Scripts\python.exe -m pip install intelhex`.
- In `audio-tools/src/AudioTools/VolumeStream.h`, `line 19`, set ``volumeConfig.allow_boost`` to `true`.
- To upload, start holding the power button already before uploading starts, and keep holding it until finished.

## Commands

Commands can be found in the Excel file and can be sent via serial (baud 9600, no line ending, for example: `com Now`) or SMS.

## Available components

SIM card / antenna, LED strip, speaker / bone inducer, GPS module, battery, solar panel and SD card.

## Current functionality

Games, audio playback, TTS (Text To Speech), FFT (Fast Fourier Transform)-based visuals, battery monitoring, telephone number management, testing.

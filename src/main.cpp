#define VERBOSE 1 // 0

#if VERBOSE==1
    #define DEBUG(x) Serial.print(x)
    #define DEBUGLN(x) Serial.println(x)
    #define DEBUGF(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
    #define DEBUGWRITE(x) Serial.write(x)
#else
    #define DEBUG(x)
    #define DEBUGLN(x)
    #define DEBUGF(fmt, ...)
    #define DEBUGWRITE(x)
#endif

#include "simSetup.hpp"
#include "pref.hpp"
#include "music.hpp"
#include "leds.hpp"
#include "commands.hpp"
#include "sim.hpp"
#include "test.hpp"

void setup() {
    // Wait five seconds
    // delay(5000);

    // Open Serial
    if (VERBOSE) {
        Serial.begin(9600);
    }

    DEBUGLN("\n\n--- Starting setup! ---");
    
    // Setup SIM7070G and start communication 
    simSetup();

    // Setup preferences database
    prefSetup();

    // Setup DAC Amp and start communication
    musicSetup();

    // Setup leds
    ledsSetup();

    DEBUGLN("\n--- Ready! ---\n");

    // Show setup is complete
    patterns[SPIRAL].input = 1;
    patterns[SPIRAL].running = true;
}

void loop() {
    // Manage music data streams
    musicHandler();

    // Manage led patterns
    ledsHandler();

    // Manage incomming sms messages
    simHandler();

    // Manager test input from Serial Monitor
    testHandler();
}

// --- TODO (options/ideas) ---
// Text to speach (tts) - upgrade
// autoNext loops?
// GPS implementation
// Shuffle !

// Power saving and sleep modes

// -Hardware-
// Casing
// Solarpanel
// Wireless charger
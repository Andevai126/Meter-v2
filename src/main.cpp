#define VERBOSE 1 // 0 or 1

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
#include "random.hpp"
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

    DEBUGLN(F("\n\n--- Starting setup! ---"));
    
    // Setup SIM7070G and start communication 
    simSetup();

    // Setup preferences database
    prefSetup();

    // Setup RNG
    randomSetup();

    // Setup DAC Amp and start communication
    musicSetup();

    // Setup leds
    ledsSetup();

    DEBUGLN(F("\n--- Ready! ---\n\n"));

    // Show setup is complete
    patterns[SPIRAL].input = 1;
    patterns[SPIRAL].running = true;

    playTTS("setup complete");
    // Set volume to 20% after reboot (as starting value)
    setVolume(0.20);
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
// Power saving and sleep modes


// -Hardware-
// Casing
// Solarpanel
// Wireless charger



// Very very optional
// 3 q files waar je alleen hoeft te appenden/poppen (als in een stack)
    // 1: q_history (deze muziek is al afgespeeld/geskipped)
    // 2: q_play (deze lijst wordt gemaakt met de Play <filter> command (wordt wellicht reverse opgeslagen)
    // 3: q_long (deze lijst wordt geappend met de Q <filter> functie

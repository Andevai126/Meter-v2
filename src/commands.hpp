// Define command content
typedef struct {
    char COMMANDtemplate[256];
    void (*func) (char* message);
    byte permissionLevel;
} commandType;

// Function prototypes
void testNow(char* message);
void testLocked(char* message);
void testAT(char* message);

void runAdd(char* message);
void runDel(char* message);
void runReset(char* message);
void runShow(char* message);
void runAddD(char* message);

void runRainbow(char* message);
void runSpiral(char* message);
void runRoulette(char* message);
void runBattery(char* message);
void runDisco(char* message);
void runMorse(char* message);

void runCountdown(char* message);
void runDanger(char* message);

void runIndex(char* message);
void runStop(char* message);
void runPlay(char* message);
void runQ(char* message);
void runPause(char* message);
void runNext(char* message);
void runAutonext(char* message);
void runPrevious(char* message);
void runEmpty(char* message);
void runVolume(char* message);
void runShuffle(char* message);

void runNL(char* message);
void runEN(char* message);
void runFFT(char* message);

void runClear(char* message);

void printPos(char* message);

// Array of all commands and specifications
static commandType commands[] = {
    // Tests
    {"Now", &testNow,           ALLOW_ALL}, //
    {"Lock", &testLocked,       ALLOW_WHITELIST},
    {"AT", &testAT,             ALLOW_ADMIN},
    // Telephone number management
    {"Add", &runAdd,            ALLOW_WHITELIST},
    {"Del", &runDel,            ALLOW_ADMIN},
    {"Reset", &runReset,        ALLOW_ADMIN},
    {"Show", &runShow,          ALLOW_WHITELIST},
    {"AddD", &runAddD,          ALLOW_ALL}, //
    // Games and LEDS
    {"Rainbow", &runRainbow,    ALLOW_WHITELIST},
    {"Spiral", &runSpiral,      ALLOW_WHITELIST},
    {"Roulette", &runRoulette,  ALLOW_WHITELIST},
    {"Battery", &runBattery,    ALLOW_ADMIN},
    {"Disco", &runDisco,        ALLOW_WHITELIST},
    {"Morse", &runMorse,        ALLOW_ALL},

    {"Countdown", &runCountdown,ALLOW_WHITELIST},
    {"Danger", &runDanger,      ALLOW_WHITELIST},

    // Audio player functions    
    {"Index", &runIndex,        ALLOW_WHITELIST}, 
    {"Stop", &runStop,          ALLOW_WHITELIST},
    {"Play", &runPlay,          ALLOW_WHITELIST},
    {"Q", &runQ,                ALLOW_WHITELIST},
    {"Pause", &runPause,        ALLOW_WHITELIST},
    {"Next", &runNext,          ALLOW_WHITELIST},
    {"Autonext", &runAutonext,  ALLOW_WHITELIST},
    {"Previous", &runPrevious,  ALLOW_WHITELIST},
    {"Empty", &runEmpty,        ALLOW_WHITELIST},
    {"Volume", &runVolume,      ALLOW_WHITELIST},
    {"Shuffle", &runShuffle,    ALLOW_WHITELIST},

    {"NL", &runNL,              ALLOW_WHITELIST},
    {"EN", &runEN,              ALLOW_WHITELIST},
    {"FFT", &runFFT,            ALLOW_WHITELIST}, 
    
    // Forced resets
    {"Clear", &runClear,        ALLOW_WHITELIST}, // Stop leds and audio

    // GPS
    {"Print", &printPos}
};
static int nCommands = sizeof(commands) / sizeof(commandType);


// Helper function
String ones[] = {"zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"};
String teens[] = {"ten", "eleven", "twelve", "thirteen", "fourteen", "fifteen", "sixteen", "seventeen", "eighteen", "nineteen"};
String tens[] = {"", "", "twenty", "thirty", "forty", "fifty", "sixty", "seventy", "eighty", "ninety"};
String nameForNumber(int number) {
    if (number < 10) {
        return ones[number];
    } else if (number < 20) {
        return teens[number - 10];
    } else if (number < 100) {
        return tens[number / 10] + ((number % 10 != 0) ? " " + nameForNumber(number % 10) : "");
    } else if (number < 1000) {
        return nameForNumber(number / 100) + " hundred" + ((number % 100 != 0) ? " " + nameForNumber(number % 100) : "");
    }
    return "";
}

// Functions Yay!

//--- Tests ---
// Test if SMS messages gets recieved and processed to this function
void testNow(char* message) {
    DEBUGLN(F("Detonated!!!"));
}

// Test if whitelist works
void testLocked(char* message) {
    DEBUGLN(F("Number accepted and testLocked function entered!"));
}

// Admins can SMS AT commands which will be caried out
void testAT(char* message) { 
    String command = message+strlen("AT ");
    DEBUGLN(sendAT(command));
}


//--- Telephone number management ---
// Whitelist members can add telephone number to whitelist (number of form '+316xxxxxxxx')
void runAdd(char* message) {
    if (strlen(message) != strlen("Add ")+12) {
        DEBUGLN(F("Invalid length!"));
    } else {
        addTelNumber((char*)(message+strlen("Add ")));
    }
}

// Admin can delete telephone number from whitelist (number of form '+316xxxxxxxx')
void runDel(char* message) {
    if (strlen(message) != strlen("Del ")+12) {
        DEBUGLN(F("Invalid length!"));
    } else {
        delTelNumber((char*)(message+strlen("Del ")));
    }
}

// Admin clears whole phone memory; admin phone numbers get restored, whitelist becomes empty
void runReset(char* message) {
    resetTelNumbers();
}

// Whitelist members can show admin numbers + whitelist numbers on serial
void runShow(char* message) {
    printTelNumbers();
}

// A way for everybody to add telephone numbers to whitelist
void runAddD(char* message) {
    DEBUGLN(F("Entered unsecured add function"));
    // Change password here...
    const char* pass = "AddD H3y0! ";
    if (strlen(message) != strlen(pass)+12) {
        DEBUGLN(F("Invalid length!"));
    } else if (strncmp(message, pass, strlen(pass)) != 0) {
        DEBUGLN(F("Incorrect password!"));
    } else {
        addTelNumber((char*)(message+strlen(pass)));
    }
}

//--- Games and LEDS ---

// Helper function
void startPattern(char* numberString, int index) {
    // Cap the length of the number on 8 chars
    if (strlen(numberString) > 8) {
        numberString[8] = '\0';
    }
    int number;
    if (sscanf(numberString, "%d", &number) != 1) {
        DEBUGLN(F("Could not parse number!"));
    } else {
        resetPatterns();
        patterns[index].input = number;
        patterns[index].running = true;
    }
}

void runRainbow(char* message) {
    startPattern(message+strlen("Rainbow "), RAINBOW);
}

void runSpiral(char* message) {
    startPattern(message+strlen("Spiral "), SPIRAL);
}

void runRoulette(char* message) {
    startPattern(message+strlen("Roulette "), ROULETTE);
}

void runBattery(char* message) {
    int percentage = getATarg<int>(sendAT("AT+CBC"), 1);
    resetPatterns();
    patterns[BATTERY].input = percentage;
    patterns[BATTERY].running = true;
    String txt = "the battery level is ";
    txt += nameForNumber(percentage);
    txt += " percent";
    setTtsLanguage();
    playTTS((char*)(txt.c_str()));
}

void runDisco(char* message) {
    startPattern(message+strlen("Disco "), DISCO);
}

void runMorse(char* message) {
    resetPatterns();
    patterns[MORSECODE].running = true;
    morseCodeSentence = message+strlen("Morse ");
}

void runCountdown(char* message) {
    // Cap the length of the first argument after 'Countdown ' at 8 chars
    if (strlen(message+strlen("Countdown ")) > 8) {
        message[strlen("Countdown ")+8] = '\0';
    }
    int number;
    if (sscanf((char*)(message+strlen("Countdown ")), "%d", &number) != 1) {
        DEBUGLN(F("Could not parse number!"));
    } else {
        // Leds
        startPattern(message+strlen("Countdown "), CHARGE);
        // Audio
        if (number == 60 || number == 80 || number == 90 || number == 120 || number == 300) {
            std::string filter = "Rocket_Launch_Countdown_" + std::to_string(number) + "s";
            playAudio(filter.c_str());
            DEBUGF("Playing %s\n", filter.c_str());
        } else {
            DEBUGLN(F("No compatible audio found for given time duration"));
        }
    }   
}

void runDanger(char* message) {
    startPattern(message+strlen("Danger "), DANGER);
}

//--- Audio player ---

// Print index files to serial and stops playing audio
void runIndex(char* message) { 
    printQueue();
}

// Stops playing audio and clear index files
void runStop(char* message) {
    setPaused(true);
}

//Plays audio. Either all mp3 or specified by 'filter'
void runPlay(char* message) {
    if (strlen(message) <= strlen("Play ")) {
        DEBUGLN(F("No filter specified"));
        // Start playing song at playerIndex
        moveAudio(true, 0);
    } else {
        DEBUGF("Filter specified: <%s>\n", message+strlen("Play "));
        playAudio((char*)(message+strlen("Play ")));
    }
}

//Adds audio specified by filter to end of queue
void runQ(char* message) {
    if (strlen(message) <= strlen("Q ")) { // If there is no argument, do nothing
        DEBUGLN(F("No filter specified"));
    } else {
        DEBUGF("Filter specified: <%s>\n", message+strlen("Q "));
        queueAudio((char*)(message+strlen("Q ")));
    }
}

// Play or Pause in between (does not pause the leds / nonblocking)
void runPause(char* message) {
    togglePaused();
}

// Go to next audio file
void runNext(char* message) {
    if (strlen(message) <= strlen("Next ")) {
        DEBUGLN(F("No argument specified"));
        moveAudio(true);
    } else {
        DEBUGF("Number specified: <%s>\n", message+strlen("Next "));
        // Cap the length of the number on 8 chars
        if (strlen(message+strlen("Next ")) > 8) {
            message[strlen("Next ")+8] = '\0';
        }
        int number;
        if (sscanf((char*)(message+strlen("Next ")), "%d", &number) != 1) {
            DEBUGLN(F("Could not parse number!"));
        } else {
            moveAudio(true, number);
        }
    }
}

// Activate autonext
void runAutonext(char* message) {  
    if (strlen(message) <= strlen("Autonext ")) {
        DEBUGLN(F("No argument specified"));
        toggleAutoNext();
    } else {
        // Get argument and convert to String for ease of comparison
        String arg = message+strlen("Autonext ");
        // Multiple options are allowed
        if (arg.indexOf("y") != -1 || arg.indexOf("Y") != -1 || arg.indexOf("t") != -1 || arg.indexOf("T") != -1) {
            setAutoNext(true);
            DEBUGLN(F("Changed Autonext to: true"));
        } else if (arg.indexOf("n") != -1 || arg.indexOf("N") != -1 || arg.indexOf("f") != -1 || arg.indexOf("F") != -1) {
            setAutoNext(false);
            DEBUGLN(F("Changed Autonext to: false"));
        }
    }
}

// Go to previous audio file
void runPrevious(char* message) {
    if (strlen(message) <= strlen("Previous ")) {
        DEBUGLN(F("No argument specified"));
        moveAudio(false);
    } else {
        DEBUGF("Number specified: <%s>\n", message+strlen("Previous "));
        // Cap the length of the number on 8 chars
        if (strlen(message+strlen("Previous ")) > 8) {
            message[strlen("Previous ")+8] = '\0';
        }
        int number;
        if (sscanf((char*)(message+strlen("Previous ")), "%d", &number) != 1) {
            DEBUGLN(F("Could not parse number!"));
        } else {
            moveAudio(false, number);
        }
    }
}

// Clear and reset the Queue
void runEmpty(char* message) { 
    emptyQueue();
}

// Edit the volume
void runVolume(char* message) {
    DEBUGF("Volume specified: <%s>\n", message+strlen("Volume "));
    // Cap the length of the volume on 4 chars
    if (strlen(message+strlen("Volume ")) > 4) {
        message[strlen("Volume ")+4] = '\0';
    }
    int volume;
    if (sscanf((char*)(message+strlen("Volume ")), "%d", &volume) != 1) {
        DEBUGLN(F("Could not parse number!"));
    } else {
        setVolume(((float)volume)/100);
    }
}

// Shuffle titles
void runShuffle(char* message) {
    shuffleQueue();
}


void runNL(char* message) {
    if (strlen(message) <= strlen("NL ")) {
        DEBUGLN(F("No words specified"));
    } else {
        setTtsLanguage("Dutch");
        DEBUGF("Words specified: <%s>\n", message+strlen("NL "));
        playTTS((char*)(message+strlen("NL ")));
    }
}

void runEN(char* message) {
    if (strlen(message) <= strlen("EN ")) {
        DEBUGLN(F("No words specified"));
    } else {
        setTtsLanguage("English");
        DEBUGF("Words specified: <%s>\n", message+strlen("EN "));
        playTTS((char*)(message+strlen("EN ")));
    }
}

// Run play but with FFT lights
void runFFT(char* message) {
    // Get argument and convert to String for ease of comparison
    String arg = message+strlen("FFT ");
    if (strlen(message) <= strlen("FFT ")) { // No filter, just turn FFT on
        DEBUGLN(F("No filter specified"));
        patterns[VUMETER].running = true;
    } else if (arg.indexOf("off") != -1 || arg.indexOf("Off") != -1) { // Off filter, turn FFT off
        DEBUGLN(F("Off filter detected: reset patterns"));
        resetPatterns();
    } else {
        DEBUGF("Filter specified: <%s>\n", message+strlen("FFT ")); // Filter present, turn FFT on and play Audio
        playAudio((char*)(message+strlen("FFT ")));
        resetPatterns();
        patterns[VUMETER].running = true;
    }
}

// Clear both audio and leds
void runClear(char* message) {
    resetPatterns();
    setPaused(true);
}

// --- GPS ---

void printPos(char* message) {
    // Turn on GNSS receiver
    DEBUGLN(sendAT(F("AT+CGNSPWR=1")));

    // Get data, aka try to get 'GNSS run status' to turn on (for max 30sec)
    unsigned long currentTime = millis();
    String response = "";
    bool turnedOn = false;
    do {
        DEBUGLN(F("Searching GPS connection..."));
        response = sendAT(F("AT+CGNSINF"));
        turnedOn = *(char*)(response.c_str()+response.indexOf(":")+2) != '0';
        delay(3000);
    } while (!turnedOn && millis() - currentTime < 30000);

    // Info about connection
    DEBUGF("Time passed: %d\n", millis() - currentTime);
    DEBUGF("Connection: %s\n", (turnedOn ? "Success!" : "Failed") );
    DEBUGLN(F("Please wait for reset"));

    // Reset
    DEBUGLN(sendAT(F("AT+CGNSPWR=0")));
    DEBUGLN(sendAT(F("AT+CFUN=1,1")));
    readySim();

    // Show results
    if (turnedOn) {
        // Print info
        DEBUGLN(response);
        float latitude = getATarg<float>(response, 3);
        float longitude = getATarg<float>(response, 4);
        DEBUGF("lat:   %f\n", latitude);
        DEBUGF("lon:   %f\n", longitude);

        // Prepare TTS
        setTtsLanguage();
        String text = "the coordinates are ";

        // Parse latitude
        text += "latitude ";
        if (latitude < 0) {
            text += "minus ";
            latitude *= -1;
        }
        text += nameForNumber((int)latitude);
        text += " dot ";
        int latitudeDec = (latitude-(int)latitude)*100;
        text += nameForNumber(latitudeDec);

        text +=" and ";

        // Parse longitude
        text += "longitude ";
        if (longitude < 0) {
            text += "minus ";
            longitude *= -1;
        }
        text += nameForNumber((int)longitude);
        text += " dot ";
        int longitudeDec = (longitude-(int)longitude)*100;
        text += nameForNumber(longitudeDec);

        // Play combined text
        playTTS((char*)(text.c_str()));
    } else {
        DEBUGLN(F("Could not get GPS connection"));
    }
}

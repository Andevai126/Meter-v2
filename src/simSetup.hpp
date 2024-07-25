#include <HardwareSerial.h>
#include <sstream>

#define SIM_RX 26
#define SIM_TX 27
#define SIM_PWR 4

HardwareSerial *ATSerial = new HardwareSerial(2);

String sendAT(String command, int tries = 5, unsigned long timeOut = 1200) {
    String response = "";
    unsigned long currentTime = millis();
    // Try command max number of times
    for (int i = 0; i < tries; i++) {
        ATSerial->print(command);
        ATSerial->print("\r\n");
        // Check for response for some time
        while (millis() - currentTime < timeOut) {
            if (ATSerial->available()) {
                response = ATSerial->readString();
                if (response.indexOf(F("OK")) != -1) {
                    // Good response
                    return response;
                } else {
                    // Bad response
                    break;
                }            
            }
        }
        currentTime = millis();
    }
    DEBUGLN(command);
    DEBUGLN(response);
    DEBUGLN(F("Error: Bad or no response received"));
    return "";
}

void readySim() {
    // Sometimes CPIN lies the first time
    DEBUGLN(sendAT(F("AT+CPIN?")));
    // Check if anything is needed
    String cpinCheck = sendAT(F("AT+CPIN?"));
    // Never happens, send PUK
    if (cpinCheck.indexOf(F("SIM PUK")) != -1) {
        DEBUGLN(F("Puk requested"));
        DEBUGLN(sendAT(F("AT+CPIN=8931440301472074259")));
        cpinCheck = sendAT(F("AT+CPIN?"));
        DEBUGLN(cpinCheck);
    }
    // Send PIN
    if (cpinCheck.indexOf(F("SIM PIN")) != -1) {
        DEBUGLN(F("Pin requested"));
        DEBUGLN(sendAT(F("AT+CPIN=0000")));
    }
    // Set SMS message format to text mode (instead of PDU mode)
    DEBUGLN(sendAT(F("AT+CMGF=1")));
}

void simSetup() {
    // Set default for PWR_PIN
    pinMode(SIM_PWR,OUTPUT);
    digitalWrite(SIM_PWR,LOW);
    delay(1000);

    // Open ATSerial
    ATSerial->begin(9600, SERIAL_8N1, SIM_RX, SIM_TX);

    // Check if power is on
    for (int i = 0; i < 4 && !ATSerial->available(); i++) {
        DEBUGLN(F("Sending test AT"));
        ATSerial->print(F("AT\r\n"));
        delay(1000);
    }

    // If not, power up
    if (!ATSerial->available()) {
        digitalWrite(SIM_PWR, HIGH);
        delay(2000);
        digitalWrite(SIM_PWR, LOW);
        delay(1000);
    }
    
    // Reset to default configuration
    DEBUGLN(sendAT(F("AT+CFUN=1,1")));

    readySim();
}

// Use 'float', 'int' or 'std::string with .c_str()'
template<typename T>
T getATarg(String AT, int index = 0) {
    char* start = (char*)AT.c_str();
    if (index == 0) {
        start = strstr(start, ":")+2;
    } else {
        for (int i = 0; i < index; i++) {
            start = strstr(start, ",")+1;
        }
    }
    char* end = strstr(start, ",");
    if (end != NULL) {
        start[end-start] = '\0';
    } else {
        start[strstr(start, "\r")-start] = '\0';
    }
    T arg;
    std::stringstream ss;
    ss << start;
    ss >> arg;
    return arg;
}

// Set error reporting to verbose
// DEBUGLN(sendAT(F("AT+CMEE=2")));

// Delete all messages in SM and ME storage
// DEBUGLN(sendAT("AT+CMGD=0,4"));

// Turn off echo
// DEBUGLN(sendAT("ATE0"));

// Turn on GNSS receiver

// AT+CSQ
// AT+CGNSPWR?

// https://randomnerdtutorials.com/esp32-save-data-permanently-preferences/

#include "Preferences.h"


// Initiate instance
Preferences preferences;


// Open namespace in read/write mode
void prefSetup() {
    preferences.begin("database", false);
}

// Get pointer number
uint16_t retrievePointer() {
    return preferences.getUShort("pointer", 0);
}


// Get array of telephone numbers
void retrieveTelNumbers(char* telNumbers) {
    preferences.getBytes("telNumbers", telNumbers, retrievePointer()*16);
}


// Check for each telephone number if it matches the given telephone number
int inTelNumbersArray(uint16_t pointer, char* telNumbers, char* telNumber) {
    for (uint16_t i = 0; i < pointer; i++) {
        if (strncmp((char*)(telNumbers+i*16), telNumber, 16) == 0) {
            return i;
        }
    }
    return -1;
}


// Add new telephone number to arrray
void addTelNumber(char* telNumber) {
    // Get pointer
    uint16_t pointer = retrievePointer();
    // Create new array to hold old telephone numbers plus given new one
    char newTelNumbers[(pointer+1)*16];
    // Put old ones in the new array
    retrieveTelNumbers(newTelNumbers);

    // Check if old telephone numbers contain given telephone number
    if (inTelNumbersArray(pointer, newTelNumbers, telNumber) != -1) {
        DEBUGLN(F("error: Could not add telephone number, already present"));
    } else {
        // Put new one in new array
        preferences.getBytes("telNumbers", newTelNumbers, pointer*16);
        strncpy((char*)(newTelNumbers+pointer*16), telNumber, 16);

        // Store telephone numbers and pointer
        preferences.putBytes("telNumbers", newTelNumbers, (++pointer)*16);
        preferences.putUShort("pointer", pointer);
    }
}


// Print all telephone numbers in array
void printTelNumbers() {
    // Get pointer and telephone numbers
    uint16_t pointer = retrievePointer();
    char telNumbers[pointer*16];
    retrieveTelNumbers(telNumbers);

    // Print pointer
    DEBUG(F("Pointer is: "));
    DEBUGLN(pointer);

    // Print telephone numbers
    DEBUGLN(F("Numbers are:"));
    for (int i = 0; i < pointer; i++) {
        DEBUGLN((char*)(telNumbers+i*16));
    }
}


// Create keys to be used
void resetTelNumbers() {
    // Inform of possible presence
    if (preferences.isKey("pointer")) {
        DEBUGLN(F("Key \"pointer\" already exists"));
    }
    if (preferences.isKey("telNumbers")) {
        DEBUGLN(F("Key \"telNumbers\" already exists"));
    }
    
    // Set default values
    preferences.putUShort("pointer", 2);
    preferences.putBytes("telNumbers", "+31624213073\0xxx+31621376240\0xxx", 2*16);
}


#define ALLOW_ALL 0 // should we add nAdmins / make possible to add/del admins? (new keyValue pair? and add functions/modify functions)
#define ALLOW_WHITELIST 1
#define ALLOW_ADMIN 2

// Check whether the given telephone number has the permission to run the function
bool isAllowed(char* telNumber, byte permissionLevel) {
    int address = -1;
    if (permissionLevel != ALLOW_ALL) {
        // Get pointer and telephone numbers
        uint16_t pointer = retrievePointer();
        char telNumbers[pointer*16];
        retrieveTelNumbers(telNumbers);

        // Get address of given telephone number in array
        address = inTelNumbersArray(pointer, telNumbers, telNumber);
    }

    switch (permissionLevel) {
        // Allow everyone to run the function
        case ALLOW_ALL:
            return true;
            break;
        // Allow only those on the whitelist to run the function
        case ALLOW_WHITELIST:
            if (address != -1)
                return true;
            break;
        // Allow only admins to run the function
        case ALLOW_ADMIN:
            if (address != -1 && address < 2)
                return true;
            break;
    }

    // Access denied
    return false;
}


// Delete given telephone number if present
void delTelNumber(char* telNumber) {
    // Get pointer and telephone numbers
    uint16_t pointer = retrievePointer();
    char telNumbers[pointer*16];
    retrieveTelNumbers(telNumbers);

    if (pointer == 2) {
        DEBUGLN(F("Can not delete telephone numbers, minimum reached"));
    } else {
        int address = inTelNumbersArray(pointer, telNumbers, telNumber);
        if (address < 2) {
            DEBUGLN(F("Can not delete telephone number, not present or below minimum"));
        } else {
            // Moving telephone numbers is not nessecery if only one is present or it's the last one
            if (!(pointer == 2+1 || address == pointer)) {
                // Shift every telephone number after address one place back
                for (int i = address; i < pointer-1; i++) {
                    strncpy((char*)(telNumbers+i*16), (char*)(telNumbers+(i+1)*16), 16);
                } 
            }

            // Create new array with smaller size and less telephone numbers
            char newTelNumbers[(--pointer)*16];
            for (int i = 0; i < pointer; i++) {
                strncpy((char*)(newTelNumbers+i*16), (char*)(telNumbers+i*16), 16);
            }

            // Store new pointer and telephone numbers
            preferences.putBytes("telNumbers", newTelNumbers, pointer*16);
            preferences.putUShort("pointer", pointer);
        }
    }
}
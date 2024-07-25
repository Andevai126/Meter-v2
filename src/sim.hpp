// Possible matches
const char* SMStemplateSim = "\r\n+CMTI: \"SM\",";
const char* SMStemplatePhone = "+CMTI: \"ME\",";
// const char* CALLtemplate = "+CLIP: \"";


// Some constants
// const uint16_t maxLenNotification = 64;
const uint16_t maxLenData = 256;
const uint16_t maxLenTelNumber = 16;
const uint16_t maxNumberOfSMS = 20;


// Retrieve telnumber and data from SMS message
bool parseMessage(const char* message, char* telNumberBuffer, char* dataBuffer) {
    // Find opening and closing quote
    const char* telNumberStart = strstr(message, ",\"") + 2;
    const char* telNumberEnd = strstr(telNumberStart, "\"");

    if (!telNumberStart || !telNumberEnd) {
        // Telnumber not found
        return false;
    }
    
    // Copy telnumber to provided buffer
    size_t telNumberLength = telNumberEnd - telNumberStart;
    strncpy(telNumberBuffer, telNumberStart, telNumberLength);
    telNumberBuffer[telNumberLength] = '\0';

    // Find first newline and start of ending
    const char* dataStart = strstr(telNumberEnd + 1, "\n") + 1;
    const char* dataEnd = message + strlen(message) - strlen("\r\n\r\nOK\r\n");

    if (!dataStart || !dataEnd) {
        // Data not found
        return false;
    }
    
    // Copy data to provided buffer
    size_t dataLength = dataEnd - dataStart;
    strncpy(dataBuffer, dataStart, dataLength);
    dataBuffer[dataLength] = '\0';

    // Both phone number and data found
    return true;
}


// Delete all messages in SM and ME storage
void deleteAllMessages() {
    DEBUGLN(sendAT("AT+CMGD=0,4"));
}


// Check if function must be run
bool isMatch(char* data, char* COMMANDtemplate, char* telNumber, byte permissionLevel) {
    // Check permission
    if (isAllowed(telNumber, permissionLevel)) {
        // Check if first bit of SMS data matches a command
        if (strncmp(data, COMMANDtemplate, strlen(COMMANDtemplate)) == 0) {
            return true;
        }
        return false;
    }
    return false;
}


// Handle outside input
void simHandler() {
    if (ATSerial->available()) {
        // Read notification
        String notification = ATSerial->readString();
        DEBUGF("Notification: %s", notification.c_str()+2);

        // Notification is incoming SMS
        if (notification.indexOf(SMStemplateSim) != -1) {
            // Convert last chars into int
            int index;
            sscanf(notification.c_str()+strlen(SMStemplateSim), "%d", &index);
            DEBUGF("Index: %d\n", index);
                
            // Get message
            std::string atcommand = "AT+CMGR=" + std::to_string(index);
            String message = sendAT(atcommand.c_str());
            // DEBUGF("Message: %s\n", message);
            
            // Get telephone number and data
            char telNumber[maxLenTelNumber];
            char data[maxLenData];
            parseMessage(message.c_str(), telNumber, data);
            DEBUGF("Phone number: %s\n", telNumber);
            DEBUGF("Data: %s\n", data);

            // Clear SMS message buffer if almost full
            if (index > maxNumberOfSMS-5) {
                deleteAllMessages();
            }

            // Run function matched with template and correct permission
            for (int i = 0; i < nCommands; i++) {
                if (isMatch(data, commands[i].COMMANDtemplate, telNumber, commands[i].permissionLevel)) {
                    commands[i].func(data);
                    break;
                }
            }

        // Notification is incoming call
        // } else if (strncmp(notification, CALLtemplate, strlen(CALLtemplate)) == 0) {
        //     // Set char after telephone number to null char
        //     notification[strlen(CALLtemplate)+12] = '\0';
            
        //     // Check if telephone number is present in stored telephone numbers
        //     if (isAllowed((char*)(notification+strlen(CALLtemplate)), ALLOW_WHITELIST)) {
        //         ATSerial->printf("ATA\r\n");
        //         DEBUGLN(F("Accepted incoming call"));
        //     } else {
        //         ATSerial->printf("ATH0\r\n");
        //         DEBUGLN(F("Declined incoming call"));
        //     }
            
        // If multiple SMS are send before SIM7070G is turned on, SMS could overflow into phone storage
        } else if (notification.indexOf(SMStemplatePhone) != -1) {
            // Clear phone storage
            deleteAllMessages();
            
        // No match for notification found
        } else {
            // Print error
            DEBUGLN("No match found");
        }
    }
}
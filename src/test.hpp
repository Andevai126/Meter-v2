void testHandler() {
    // Buffer to store the received input
    static String inputString;

    if (Serial.available()) {
        // Read input
        inputString = Serial.readString();

        // Test clear messages
        if (strcmp(inputString.c_str(), "clear") == 0) {
            deleteAllMessages();
            DEBUGLN(F("cleared!"));
            inputString = "";
            return;
        }

        // Test GPS
        if (inputString.indexOf("arg") != -1) {
            String inf = "AT+CGNSINF\r\n+CGNSINF: 1,,,52.000002,6.000000,-46.760,,,1,,0.1,0.1,0.1,,,,93394.4,6000.0\r\n\r\nOK\r\n";
            DEBUGF("lat:   %f\n", getATarg<float>(inf, 3));
            DEBUGF("lon:   %f\n", getATarg<float>(inf, 4));
            DEBUGF("first: %d\n", getATarg<int>(inf, 0));
            DEBUGF("last:  %s\n", getATarg<std::string>(inf, 17).c_str());

            inputString = "";
            return;
        }

        // Test SMS input
        if (inputString.indexOf("com") != -1) {
            char* com = (char*)inputString.c_str();
            com = strstr(com, " ")+1;

            for (int i = 0; i < nCommands; i++) {
                if (strncmp(com, commands[i].COMMANDtemplate, strlen(commands[i].COMMANDtemplate)) == 0) {
                    commands[i].func(com);
                    break;
                }
            }

            inputString = "";
            return;
        }

        DEBUGLN(sendAT(inputString));
      
        // Clear string for the next input
        inputString = "";
    }
    return;
}

/* Example commands:
AT+CMGR=<index>             -to read specific message
AT+CMGD=<index>[,<delflag>] -to delete specific message
AT+CPMS="SM"                -to see used slots and total slots (or "ME" for phone memory)
ATI                         -to get the type of the modum
AT+CSQ                      -to get signal strength (0-31) and (comma-separated) quality of the cellular (0-7)
AT+CSDH=1                   -to show all text mode parameters
ATA                         -to accept call
AT+CBC                      -to get battery connectivity, percentage and voltage
AT+CMGDA="DEL ALL"          -to delete all sms messages
AT+STTONE                   -to play any (ring)-tone
AT+CMGL="ALL"               -to show all SMS messages
AT+CMEE=2                   -to set error reporting to verbose
AT+CMGD=0,4                 -to delete all messages in SM and ME storage
ATE0                        -to turn off serial (monitor) input echo
AT+CGNSPWR?                 -to check power state of GNSS / CGNS / GPS antenna
AT+CGNSPWR=1                -to turn on GPS antenna
AT+CGNSINF                  -to get GPS info

When a long string of numbers is returned, send the AT+CMGF=1 command.
*/
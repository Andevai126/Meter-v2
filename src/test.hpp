void testHandler() {
    // String to store the received input
    static String inputString;

    if (Serial.available()) {
        // Read input
        inputString = Serial.readString();

        if (strcmp(inputString.c_str(), "clear") == 0) {
            // Something else ...
            deleteAllMessages();
            DEBUGLN(F("cleared!"));
            inputString = "";
            return;
        }


        if (inputString.indexOf("arg") != -1) {
            String inf = "AT+CGNSINF\r\n+CGNSINF: 1,,,52.000002,6.000000,-46.760,,,1,,0.1,0.1,0.1,,,,93394.4,6000.0\r\n\r\nOK\r\n";

            DEBUGF("lat:   %f\n", getATarg<float>(inf, 3));
            DEBUGF("lon:   %f\n", getATarg<float>(inf, 4));
            DEBUGF("first: %d\n", getATarg<int>(inf, 0));
            DEBUGF("last:  %s\n", getATarg<std::string>(inf, 17).c_str());

            inputString = "";
            return;
        }

        // if (inputString.indexOf("ttt") != -1) { //number
        //     char* mes = (char*)inputString.c_str();
        //     mes = mes+strlen("ttt ");
        //     int64_t test;
        //     sscanf(mes, "%d", &test);
        //     nutt.say(test);

        //     inputString = "";
        //     return;
        // }

        // if (inputString.indexOf("uuu") != -1) { //procent
        //     char* mes = (char*)inputString.c_str();
        //     mes = mes+strlen("uuu ");
        //     double test;
        //     sscanf(mes, "%lf", &test);
        //     Serial.println(test);
        //     nutt.say(test, "%");

        //     inputString = "";
        //     return;
        // }

        // if (inputString.indexOf("sss") != -1) { //word
        //     char* mes = (char*)inputString.c_str();
        //     mes = mes+strlen("sss ");
        //     tts.say(mes);

        //     inputString = "";
        //     return;
        // }

        if (inputString.indexOf("com") != -1) {
            // Expected input: "com Spiral 3"
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

/* Notes:
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
When a long string of numbers is returned, send the AT+CMGF=1 command.
*/

/*
geluid en leds tegelijkertijd
reactie op einde geluid in command
schrijven naar sd
meerdere letters van sd naar geluid

global var voor busy sending at command?

meerdere geluiden tegelijkertijd?

// Mischien kunnen we voor de 5 countdown nummers ook een random functie maken dat hij 1 van de 5 pakt

// Voor disco kunnen we ook een functie maken met #nummers als input en dan pakt hij
dat aantal random nummbers + juiste lengte lichtjes
*/

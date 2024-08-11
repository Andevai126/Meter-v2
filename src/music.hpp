// https://github.com/pschatzmann/arduino-audio-tools
// https://github.com/pschatzmann/arduino-libhelix

#include "AudioTools.h"
#include "SdFat.h"
// #include "SimpleTTS.h"
#include "AudioCodecs/CodecMP3Helix.h"


//--- SD card connection ---
// In AudioSourceIdxSDFAT.h (.pio\libdeps\esp-wrover-kit\audio-tools\src\AudioLibs\AudioSourceIdxSDFAT.h)
#define SD_SS 13
#define SD_MOSI 15
#define SD_MISO 2
#define SD_SCK 14
// Comment default constructor with SdSpiConfig
// And rename constructor with SdSpiConfig to AudioSourceIdxSDFAT

//--- Define (audio)streams ---
SdSpiConfig spi_cfg(SD_SS, DEDICATED_SPI, SD_SCK_MHZ(10));
File32 currentlyPlaying;
MultiOutput multi_output;
VolumeStream volume(multi_output);
EncodedAudioStream decoder(&volume, new MP3DecoderHelix());
StreamCopy copier(decoder, currentlyPlaying);
I2SStream i2s;


// Audioflow:
// sd(mp3) -> currentPlaying -> decoder ->       volume   -> multi_output -> i2s            ->      DAC
//                                                                        -> fft            ->      LEDs
// Stream pushers (at every arrow):    
//     SD.begin    |  copier.begin | decoder.begin | volume.begin | multi_output.add() | i2s.begin
//                                                                                       fft.begin

SdFat32 SD; // Hardcoded in the AudioSourceIdxSDFAT settings
File32 queue; // Accompanying file format
File32 root;
String titles;
int nTitles = 0;

bool audioPaused = true;
bool audioFinished = true;
bool audioAutoNext = true;

// Pins for speaker
#define DAC_AMP_LRC 19
#define DAC_AMP_BCLK 23 
#define DAC_AMP_DIN 18

I2SConfig cfg;

void searchDirectory(File32 dir, String prepend);

// Setup DAC Amp and start communication
void musicSetup() {
    // Start SPI connection
    SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_SS);

    // Open SD card to read/write files (try 5 times)
    for (int i = 0; i < 5; i++) {
        if (!SD.begin(spi_cfg)) {
            DEBUGLN(F("SD.begin failed"));
            delay(1000);
        } else {
            DEBUGLN(F("SD card connected"));
            break;
        }
    }
    
    // Create queue file if it doesn't exist
    if  (!SD.exists("/q.txt")) {
        queue = SD.open("/q.txt", O_CREAT);
        // Write zero at Number of titles and Player position
        for (int i = 0; i < 4; i++) {
            queue.write(uint8_t(0));
        }
        queue.close();
    }

    // Retrieve all music sound paths (.mp3)
    root = SD.open("/Music");
    searchDirectory(root, "");
    root.close();
    // DEBUGF("Titles: <%s>\n", titles.c_str());
    for (int i = 0; titles.c_str()[i] != '\0'; ++i) {
        if (titles.c_str()[i] == '\n') {
            nTitles++;
        }
    }
    // DEBUGF("nTitles: <%d>\n", nTitles);

    // Start decoder
    decoder.begin();

    // Setup volume
    auto volumeConfig = volume.defaultConfig();
    // volumeConfig.allow_boost = true; (changed headerfile VolumeStream.h line:19)
    volume.begin(volumeConfig);

    // Add i2s to multi_output
    multi_output.add(i2s);

    // Setup i2s
    cfg = i2s.defaultConfig(RXTX_MODE); // TX_MODE
    cfg.pin_bck =  DAC_AMP_BCLK; // tx bclk (mosi) (blue)
    cfg.pin_ws =   DAC_AMP_LRC;  // rx lrc  (miso) (darkpurple)
    cfg.pin_data = DAC_AMP_DIN;  // d  din  (sclk) (green)
    i2s.begin(cfg);

    // TODO Testing !!!
    // currentlyPlaying = SD.open("/TextToSpeech/knowledge.mp3");
    // copier.begin();
    // for (int i = 0; i < 6; i++) { Serial.println(copier.copy()); delay(1000); }
    
    // Serial.println("nu empty");
    // delay(1000);
    // currentlyPlaying = SD.open("/TextToSpeech/EMPTY.mp3");
    // copier.begin();
    // Serial.println(copier.copy());
    // Serial.println(copier.copy());

    // delay(1000);
    // Serial.println("na één");
    // delay(1000);

    // currentlyPlaying = SD.open("/TextToSpeech/management.mp3");
    // copier.begin();
    // for (int i = 0; i < 6; i++) { Serial.println(copier.copy()); delay(1000);}
    // currentlyPlaying.close();

    // delay(1000);
    // Serial.println("na twee");
    // delay(1000);

    // currentlyPlaying = SD.open("/TextToSpeech/knowledge.mp3");
    // copier.begin();
    // for (int i = 0; i < 6; i++) { Serial.println(copier.copy()); delay(1000); }
    // currentlyPlaying.close();

    // delay(1000);
    // Serial.println("na drie");
    // delay(1000);

    // currentlyPlaying = SD.open("/TextToSpeech/knowledge.mp3");
    // copier.begin();
}

void searchDirectory(File32 dir, String prepend) {
    // Begin at the start of the directory
    dir.rewindDirectory();

    while(true) {
        File32 entry =  dir.openNextFile();
        // Reached end
        if (!entry) {
            break;
        }
        // Get name of file
        char name[64];
        entry.getName(name, 64);
        // Recurse for directories
        if (entry.isDirectory()) {
            DEBUGF("Searching dir \"%s\"\n", name);
            String newPrepend = "/";
            newPrepend += name;
            searchDirectory(entry, newPrepend);
            
        // Save files ending with .mp3
        } else if (strstr(name, ".mp3")) {
            titles += "/Music";
            titles += prepend;
            titles += "/";
            titles += name;
            titles += "\n";
        }
        entry.close();
    }
}

// Print the contents of q-file (for testing purposes only)
void printQueue() {
    if (!queue.open("q.txt", O_READ)) {
        SD.errorHalt("opening test.txt for read failed");
    }
    DEBUGLN(F("--- printing q.txt ---"));

    // Read first four bytes
    uint16_t nPaths = queue.read() << 8 | queue.read();
    uint16_t playerIndex = queue.read() << 8 | queue.read();
    DEBUGF("print nPaths: <%d>\n", nPaths);
    DEBUGF("print playerIndex: <%d>\n", playerIndex);
   
    // Read from the file until there's nothing else in it:
    DEBUG(F("print paths: <"));
    int data;
    while ((data = queue.read()) >= 0) {
        DEBUGWRITE(data);
    }
    DEBUG(F(">\n--- printing q.txt done ---\n"));
    // Close the file:
    queue.close();
}

// Remove the queue
void emptyQueue() {
    if(SD.truncate("/q.txt", 0)) {
        queue = SD.open("/q.txt", O_RDWR);
        // Write zero at nPaths and playerIndex
        for (int i = 0; i < 4; i++) {
            queue.write(uint8_t(0));
        }
        queue.close();
        DEBUGLN(F("Succesfully emptied q.txt"));
    } else {
        DEBUGLN(F("Failed to empty q.txt"));
    }
}

// Sets all paths, matching a given filter, returns number of matches
uint16_t filterToPaths(String filter, String& pathsA, bool tts=false) {
    char* start = (char*)titles.c_str();
    String paths;
    uint16_t nPaths = 0;

    if (tts) {
        char* word = strtok((char*)filter.c_str(), " ");
        while (word != NULL) {
            paths += "/TextToSpeech/";
            paths += word;
            paths += ".mp3\n";
            nPaths++;
            word = strtok(NULL, " ");
        } 
    } else {
        for (int i = 0; i < nTitles; i++) {
            char* end = strstr(start, "\n")+1;
            if (strnstr(start, filter.c_str(), end-start)) {
                for (int i = 0; i < end-start; i++) {
                    paths += start[i];
                }
                nPaths++;
            }
            start = end;
        }
    }

    if (strlen(paths.c_str()) == 0) {
        DEBUGLN(F("No matching titles found, maybe check caps or no capital letters?"));
    }

    // Set the pointer to the new String at the 'a'ddress of paths'A'
    pathsA = paths;
    return nPaths;
}

// Returns path at player index
String getPathOfPlayerIndex() {
    queue = SD.open("/q.txt", O_READ);
    queue.seekSet(2);
    uint16_t playerIndex = queue.read() << 8 | queue.read();
    for (int i = 0; i < playerIndex; i++) {
        queue.find('\n');
    }
    String path = queue.readStringUntil('\n');
    queue.close();
    return path;
}

// Shuffle all titles in queue present after playerIndex
bool shuffleQueue() {
    // Open file
    queue = SD.open("/q.txt", O_RDWR);
    // Read first four bytes
    uint16_t nPaths = queue.read() << 8 | queue.read();
    uint16_t playerIndex = queue.read() << 8 | queue.read();
    uint16_t shuffleStartIndex = playerIndex+1;

    // Can be shuffled
    if (nPaths == 0 || nPaths == 1 || shuffleStartIndex >= nPaths-1) {
        queue.close();
        return false;
    }

    // Move reader pointer past current title
    for (int i = 0; i < shuffleStartIndex; i++) {
        queue.find('\n');
    }

    // Read titles after playerIndex
    String titlesAfter[nPaths-shuffleStartIndex];
    for (int i = 0; i < nPaths-shuffleStartIndex; i++) {
        titlesAfter[i] = queue.readStringUntil('\n') + "\n";
    }

    // Shuffle titles
    customShuffleStringArray(titlesAfter, nPaths-shuffleStartIndex);

    // Move reader pointer to current title
    queue.seekSet(0);
    for (int i = 0; i < shuffleStartIndex; i++) {
        queue.find('\n');
    }

    // Overwrite titles with shuffled titles
    for (int i = 0; i < nPaths-shuffleStartIndex; i++) {
        queue.print(titlesAfter[i]);
    }

    queue.close();
    DEBUGLN("Shuffle complete");
    return true;
}

// Updates playerIndex, returns if change occurred
bool movePlayerIndex(int change) {
    // Open file
    queue = SD.open("/q.txt", O_RDWR);
    // Read first four bytes
    uint16_t nPaths = queue.read() << 8 | queue.read();
    uint16_t playerIndexOld = queue.read() << 8 | queue.read();

    // Can playerIndex be moved
    if (nPaths == 0 || nPaths == 1) {
        queue.close();
        return false;
    }

    // Can playerIndex become 'negative'
    if (change < 0 && change*(-1) > playerIndexOld) {
        queue.close();
        return false;
    }

    // Can playerIndex become to large 
    if (playerIndexOld+change >= nPaths) {
        // Set player index to zero
        queue.seekSet(2);
        queue.write(uint8_t(0));
        queue.write(uint8_t(0));

        queue.close();
        return true;
    }
    
    // Update player index
    uint16_t playerIndexNew = playerIndexOld + change;
    queue.seekSet(2);
    queue.write(playerIndexNew >> 8); // MSB
    queue.write(playerIndexNew); // LSB

    queue.close();
    return true;
}

// Add 0, 1 or multiple paths at the end of the queue or at playerIndex
String addToQueue(String filter, bool atPlayerIndex=false, bool tts=false) {
    // Convert filter to paths
    String paths;
    uint16_t nPaths = filterToPaths(filter, paths, tts);
    DEBUGF("paths: <%s>\nnPaths: <%d>\n", paths.c_str(), nPaths);

    // Filter didn't correspond to any files or directories
    if (nPaths == 0) { return ""; }

    // Open file
    queue = SD.open("/q.txt", O_RDWR);
    // Read first four bytes
    uint16_t nPathsOld = queue.read() << 8 | queue.read();
    uint16_t playerIndexOld = queue.read() << 8 | queue.read();

    // Update number of paths
    uint16_t nPathsNew = nPathsOld+nPaths;
    queue.seekSet(0);
    queue.write(nPathsNew >> 8); // MSB
    queue.write(nPathsNew); // LSB

    if (atPlayerIndex) {
        // Update player index
        uint16_t playerIndexNew = playerIndexOld;
        if (nPathsOld > 0) {
            playerIndexNew++;
        }
        queue.write(playerIndexNew >> 8); // MSB
        queue.write(playerIndexNew); // LSB
        
        // Read left over file
        int8_t data;
        String queueText;
        while ((data = queue.read()) >= 0) {
            queueText += (char)data;
        }

        // Find position of playerIndex
        char* beginP = (char*)queueText.c_str();
        char* endP = beginP;

        for (int i = 0; i < playerIndexNew; i++) {
            endP = strstr(endP, "\n");
            endP += 1;
        }

        // Add four because first 4 bytes are not included in beginP
        // because those are already written
        uint32_t newPos = 4 + endP - beginP;
        queue.seekSet(newPos);

        // Write new paths
        queue.print(paths);

        // Write old paths (which were after old player index)
        queue.print(endP);
    } else {
        queue.seekEnd();
        queue.print(paths);
    }
    queue.close();

    // Return first path matching the given filter
    char* firstPath = (char*)paths.c_str();
    firstPath[strstr(paths.c_str(), "\n")-firstPath] = '\0';
    return firstPath;
}

// Insert into queue and start specific audio
void playAudio(String filter) {
    // Add to queue
    String sPath = addToQueue(filter, true);
    char* path = (char*)sPath.c_str();

    // Break when no match for filter was found
    if (*path == '\0') {
        DEBUGLN(F("No match for filter"));
        return;
    }
    
    DEBUGF("Now playing: \"%s\"\n", path);
    currentlyPlaying.close();
    currentlyPlaying = SD.open(path);
    copier.begin();
    audioFinished = false;
    audioPaused = false;
}

// Add songs according to filter to end of queue
void queueAudio(String filter) {
   addToQueue(filter);
}

// Move player index by number
void moveAudio(bool positive, int number = 1) {
    if (movePlayerIndex(number*(positive*2-1))) { // TODO combine move and get, to save time for TTS?, maybe seperate function?
        String path = getPathOfPlayerIndex();
        DEBUGF("Now playing: \"%s\"\n", path.c_str());
        currentlyPlaying.close();
        currentlyPlaying = SD.open(path);
        copier.begin();
        audioFinished = false;
        audioPaused = false;
    }
}

void setPaused(bool doit) {
    audioPaused = doit;
}

void togglePaused() {
    audioPaused = !audioPaused;
    DEBUGF("audioPaused is now: %d\n", audioPaused);
}

void setAutoNext(bool doit) {
    audioAutoNext = doit;
}

void toggleAutoNext() {
    audioAutoNext = !audioAutoNext;
    DEBUGF("audioAutoNext is now: %d\n", audioAutoNext);
}

float volumeScaler = 1;

void setVolume(float scaler) {
    if (scaler < 0 || scaler > 3) {
        DEBUGF("Invalid volume: %f\n", scaler);
        return;
    }

    volumeScaler = scaler;
    volume.setVolume(scaler);
    DEBUGF("Volume: %f\n", scaler);
}

// Insert into queue and start specific audio
void playTTS(char* words) {
    // Add all words with path to queue
    addToQueue(words, true, true);
    
    // Turn on autoNext
    setAutoNext(true);

    // Start with first
    currentlyPlaying.close();
    currentlyPlaying = SD.open(getPathOfPlayerIndex());
    copier.begin();
    audioFinished = false;
    audioPaused = false;
    
    DEBUGLN(F("Starting TTS!"));
}

void musicHandler() {
    // Pause audio
    if (!audioPaused) {
        // Check if audio is finished
        if (copier.copy() == 0 && !audioFinished) {
            DEBUGLN(F("Audio finished"));
            audioFinished = true;
            // Auto next functionality
            if (audioAutoNext) {
                moveAudio(true);
            }
        }
    }
}
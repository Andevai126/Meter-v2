#include <map>

// FastLED, by Daniel Garcia, v3.6.0
// https://github.com/FastLED/FastLED

// Colorpicker CHSV() inputs are (hue 0-360, saturation 0-100 , value 0-100) converted to 256 int values
// https://github.com/FastLED/FastLED/wiki/FastLED-HSV-Colors

#define FASTLED_INTERNAL			// Prevents warnings from FastLED during compiling
#include <FastLED.h>				// Led driver

// Led strip characteristics
#define NUM_LEDS_PER_CIRCLE 14		// 14  // 2  // 4
#define NUM_CIRCLES 13				// 13  // 16 // 32
#define NUM_LEDS 182				// 182 // 32 // 128

// Data line
#define LEDS_DATA 32				// Leds data-signal pin

// Define the array of leds
CRGB leds[NUM_LEDS];

// --- For FFT ---
#include "AudioLibs/AudioRealFFT.h"

// --- Create Buffer ---
#include <Framebuffer_GFX.h>
Framebuffer_GFX *gfx = new Framebuffer_GFX(leds, NUM_CIRCLES, NUM_LEDS_PER_CIRCLE, NULL);

// --- Create fft output ---
AudioRealFFT fft;

// --- For Patterns ---
unsigned long recordedTime = 0;
unsigned long timeToWait = 0;

// Define indices of possible patterns
#define RAINBOW 	0
#define SPIRAL 		1
#define ROULETTE 	2
#define CHARGE 		3
#define BATTERY 	4
#define DISCO 		5
#define DANGER 		6
#define VUMETER		7
#define MORSECODE   8

// Define content of a pattern
typedef struct {
	bool running;
	bool (*func) (int argument);
	int input;
} patternType;

// Function prototypes
bool rainbow(int givenNumLoop);
bool spiral(int givenNumLoop);
bool roulette(int givenNumLoop);
bool charge(int givenTime);
bool battery(int percentage); //Not implemented
bool disco(int givenTime); 
bool danger(int givenTime); 
bool vumeter(int notUsed);
bool morsecode(int notUsed);

// Array of all patterns and specifications
patternType patterns[] = {
	{false, &rainbow, 0},
	{false, &spiral, 0},
	{false, &roulette, 0},
	{false, &charge, 0},
	{false, &battery, 0},
	{false, &disco, 0},
	{false, &danger, 0},
	{false, &vumeter, 0},
	{false, &morsecode, 0},
};
static int nPatterns = sizeof(patterns) / sizeof(patternType);

void ledsSetup() {
    // FastLED.addLeds<WS2812,LEDS_DATA,RGB>(leds, NUM_LEDS);
    FastLED.addLeds<WS2812,LEDS_DATA,GRB>(leds, NUM_LEDS);

	FastLED.setBrightness(10);

    // Clear all pixel data
	FastLED.clear(true);

	// Setup FFT
    auto tcfg = fft.defaultConfig();
    tcfg.length = 1024;
    tcfg.bits_per_sample = 16; // 16 24 32
    tcfg.window_function = new FlatTop();
    fft.begin(tcfg);

    // Setup Multioutput
    multi_output.add(fft);
}

// Execute pattern functions that are turned on
void ledsHandler() {
	// For each pattern in patterns
	for (int i = 0; i < nPatterns; i++) {
		// Check if it should be run
		if (patterns[i].running) {
			// Run function and if it returns true, (ended), set running to false 
			if (patterns[i].func(patterns[i].input)) {
				patterns[i].running = false;
			}
		}
	}
}

// Turn all running patterns off
void resetPatterns() {
	for (int i = 0; i < nPatterns; i++) {
		patterns[i].running = false;
	}
	// If pattern interrupted and not finished, resetting values is important
	recordedTime = 0;
	timeToWait = 0;
	FastLED.clear(true);
}

void fadeall(uint8_t scaledown) { for(int i = 0; i < NUM_LEDS; i++) { leds[i].nscale8(scaledown); } }

bool rainbow(int givenNumLoop) {

	// Variables to be used later
	static int numLoop;
	static int m;
	static int j;
	static uint8_t hue;
	static bool forward;

	// First time this function is called
	if (recordedTime == 0) {
		// Record time
		recordedTime = millis();

		// Limit size of numLoop
		numLoop = min(givenNumLoop, 1027);

		m = 0;
		j = 0;
		hue = 0;
		forward = true;

		FastLED.setBrightness(10);
		timeToWait = 0;
	}

	// Wait
	if (millis() < recordedTime + timeToWait) {
		return false;
	} else {
		recordedTime = millis();
		timeToWait = 0;
	}

	if (m < numLoop) {
		//Times light goes to the end AND back
		if((j < NUM_LEDS-1 && forward ) || j == 0) { 
			// when moving forward
			forward = true;
			leds[j] = CHSV(hue++, 255, 255);
			j++;
		} else {
			// when moving backward
			forward = false;
			leds[j] = CHSV(hue++, 255, 255);
			j--;
			if (j==0) {
				m++;
			}
		}
		FastLED.show(); 
		fadeall(250);
		timeToWait = 10;
		return false;
	} else { // Fade out at the end
		if (j < NUM_LEDS) { 
			fadeall(250);			
			FastLED.show();
			j++;
			return false;
		}
	}
	// Program has ended
	recordedTime = 0;
	FastLED.clear(true); // reset all led data (leds are off)
	DEBUGLN(F("Rainbow function ended"));
	return true;
}

bool spiral(int givenNumLoop) {
	// Variables to be used later
	static int numLoop;
	static int m;
	static int i;
	static int j;
	static uint8_t hue;

	// First time this function is called
	if (recordedTime == 0) {
		// Record time
		recordedTime = millis();

		// Limit size of numLoop
		numLoop = min(givenNumLoop, 1027);

		m = 0;
		i = 0;
		j = 0;
		hue = 0;
		FastLED.setBrightness(10);
		timeToWait = 0;
	}

	// Wait
	if (millis() < recordedTime + timeToWait) {
		return false;
	} else {
		recordedTime = millis();
		timeToWait = 0;
	}

	if (m < numLoop*2) {
		//Times light goes around a circle
		if (i < NUM_LEDS_PER_CIRCLE) {   
			// for each led in a circle
			for (j=0; j < NUM_CIRCLES; j++) { 
				// for each circle  
				leds[i+j*NUM_LEDS_PER_CIRCLE] = CHSV(hue, 255, 255);				
			}
			hue+=4;
			FastLED.show(); 
			fadeall(50);
			timeToWait = 40;
			i++;
			return false;
		} else {
			i = 0;
		}
		m++;
		return false;
	} 
	// Program has ended
	recordedTime = 0;
	FastLED.clear(true); // reset all led data (leds are off)
	DEBUGLN(F("Spiral function ended"));
	return true;
}

bool roulette(int givenNumLoop) {

	// Variables to be used later
	static int numLoop;
	static int m;
	static int i;
	static int k;
	static int j;
	static int possibleNumbers[NUM_CIRCLES];
	static int numRounds;

	// First time this function is called
	if (recordedTime == 0) {
		// Record time
		recordedTime = millis();

		// Limit size of numLoop
		numLoop = min(givenNumLoop, 13);

		m = 0;
		i = 0;
		k = 0;
		j = 0;
		FastLED.setBrightness(10);

		// Create shuffled array containing numbers 0 to NUM_CIRCLES-1
		setShuffledIntArray(possibleNumbers, NUM_CIRCLES);

		// Choose random amount of rounds
		numRounds = (rand()%6)+4;

		timeToWait = 0;
	}

	// Wait
	if (millis() < recordedTime + timeToWait) {
		return false;
	} else {
		recordedTime = millis();
		timeToWait = 0;
	}
	
	if (m < numLoop) {
		//Times red light goes around
		if (k < NUM_CIRCLES) { 
			// for each circle  
			if (i == numRounds && k == possibleNumbers[m]){            
				FastLED.clear(); //Turn of red trail lights
				for (j = 0; j < NUM_LEDS_PER_CIRCLE; j++) {
					leds[k*NUM_LEDS_PER_CIRCLE+j] = CRGB(153, 0, 0); // Green              
				}
				timeToWait = 10000; // Wait for 10 seconds
				//Reset for next numLoop
				numRounds = (rand()%6)+4;
				i = 0;     
				m++;         
			}else{
				for (j = 0; j < NUM_LEDS_PER_CIRCLE; j++) {
					leds[k*NUM_LEDS_PER_CIRCLE+j] = CRGB(42, 255, 4); // Red
				}
				timeToWait = 10+3*(13*i+k);
			}
			k++;
			fadeall(128);
			FastLED.show();         
		}else{
			k = 0;
			i++;
		}
		return false;
	}  
	// Program has ended
	recordedTime = 0;
	FastLED.clear(true); // reset all led data (leds are off)
	DEBUGLN(F("Roulette function ended"));
	return true;
}

bool charge(int givenTime) { // givenTime is in seconds
	// Variables to be used later
	static int i;
	static int j;
	static int k;
	static bool uneven;
	static int pairs;
	static int segments;
	static int timeCap = 300;

	static unsigned long firstRecordedTime;

	// First time this function is called
	if (recordedTime == 0) {
		// Record time
		recordedTime = millis();
		firstRecordedTime = recordedTime;

		i = 0;
		j = 0;
		k = 0;
		FastLED.setBrightness(10);
		timeToWait = 0;

		uneven = NUM_CIRCLES % 2;
		pairs = NUM_CIRCLES >> 1;
		segments=pairs;
		if (uneven)
			segments++;
	}

	// Wait
	if (millis() < recordedTime+timeToWait) {
		return false;
	} else {
		recordedTime = millis();
		timeToWait = 0;
	}

	if (millis() < (firstRecordedTime+min(givenTime, timeCap)*1000)) {
		// Time for which this pattern runs
		if (i < pairs){
			FastLED.clear(); 
			// for pair
			if (i%2){
				for (k = 0; k < NUM_LEDS_PER_CIRCLE; k++) { // Orange
					leds[i*NUM_LEDS_PER_CIRCLE+k] = CRGB(255, 50, 0);
					leds[(NUM_CIRCLES-1-i)*NUM_LEDS_PER_CIRCLE+k] = CRGB(255, 50, 0);    
				}
			} else {
				for (k = 0; k < NUM_LEDS_PER_CIRCLE; k++) { //Blue
					leds[i*NUM_LEDS_PER_CIRCLE+k] = CRGB(0, 90, 255);  
					leds[(NUM_CIRCLES-1-i)*NUM_LEDS_PER_CIRCLE+k] = CRGB(0, 90, 255);          
				}
			}			
			i++;
		} else if (uneven) {
			FastLED.clear(); 
			for (k = 0; k < NUM_LEDS_PER_CIRCLE; k++) { // Blue
				leds[i*NUM_LEDS_PER_CIRCLE+k] = CRGB(0, 90, 255);  
			}
			i = 0;
			j++;
		} else {
			i = 0;
			j++;
			return false;
		}
		FastLED.show();
		// Create range from 0 to 1 over given time period
		double x = (double(millis()-firstRecordedTime))/(double(min(givenTime, timeCap)*1000));
		timeToWait = ((min(givenTime, timeCap)*1000)/pairs)*(0.5-0.5*x);
		// Create range from 0 to 1 over number of segments
		double y = (double(i))/(double(segments));
		timeToWait = max(timeToWait*(-0.5*atan(j*y)+1), 20.0);
		return false;
	}

	// Program has ended, initialise danger function
	DEBUGLN(F("Start danger function"));
	if (givenTime == 60 || givenTime == 80 || givenTime == 120 || givenTime == 300) {
		patterns[DANGER].input = 20;
        patterns[DANGER].running = true;
	} else {
		patterns[DANGER].input = 10;
        patterns[DANGER].running = true;
	}
	
	// Program has ended, reset all variables
	recordedTime = 0;
	FastLED.clear(true); // reset all led data (leds are off)
	DEBUGLN(F("Charge function ended"));
	return true;
}

bool battery(int percentage) {
	// Variables to be used later
	static int numLoop = 3;
	static int m;
	static int j;
	static int numLeds;	
	static double hueIncrease;
	static double hueTemp;
	static uint8_t hue;

	// First time this function is called
	if (recordedTime == 0) {
		// Record time
		recordedTime = millis();
		
		DEBUGF("Battery percentage: %d\n", percentage);

		m = 0;
		j = 0;
		hue = 0;
		hueTemp = 0;
		hueIncrease = 120.0/(double(NUM_LEDS));

		FastLED.setBrightness(10);
		timeToWait = 0;

		// Convert percentage to number of leds
		numLeds = round(((((double)percentage))/100.0)*NUM_LEDS);
	}

	// Wait
	if (millis() < recordedTime + timeToWait) {
		return false;
	} else {
		recordedTime = millis();
		timeToWait = 0;
	}

	if (m < numLoop) {
		//Times light goes to the end AND back
		if (j < numLeds) {
			leds[j] = CHSV(hue, 255, 255);
			j++;
			hueTemp += hueIncrease;
			hue = hueTemp;
			FastLED.show(); 
			fadeall(250);
			timeToWait = 20;
		} else {
			j = 0;
			m++;
			hue = 0;
			hueTemp = 0;
		}		
		return false;
	} 
	// Program has ended
	recordedTime = 0;
	FastLED.clear(true); // reset all led data (leds are off)
	DEBUGLN(F("Battery function ended"));
	return true;
}

typedef struct {
	unsigned long endTime;
	uint8_t hue;
	int indexCircle;
} particleType;

bool disco(int givenTime) {
	// Variables to be used later
	const int numParticles = 6;
	static particleType particles[numParticles];
	static int timeCap = 300;
	static unsigned long firstRecordedTime;
	

	// First time this function is called
	if (recordedTime == 0) {
		// Record time
		recordedTime = millis();
		firstRecordedTime = recordedTime;

		FastLED.setBrightness(10);
		
		// ------- Seed particles ------- //
		int possibleCircles[NUM_CIRCLES];
		// Create shuffled array containing numbers 0 to NUM_CIRCLES-1
		setShuffledIntArray(possibleCircles, NUM_CIRCLES);

		for (int i = 0; i < NUM_CIRCLES; i++)
		{
			DEBUGF("%d ", possibleCircles[i]);
		}
		DEBUG('\n');
		// Create starting particles
		for (int i = 0; i < numParticles; i++) {
			particles[i].indexCircle = possibleCircles[i];
			particles[i].endTime = 40+(rand()%1960)+recordedTime;
			particles[i].hue = (rand()%16)*16;
			DEBUGF("%d\t", particles[i].indexCircle);
			DEBUGF("%d\t", particles[i].endTime);
			DEBUGF("%d\n", particles[i].hue);
		}
	}


	if (millis() < (firstRecordedTime+min(givenTime, timeCap)*1000)) {		
		int possibleCircles[NUM_CIRCLES-numParticles];
		
		int index = 0;
		// For each circle
		for (int i = 0; i < NUM_CIRCLES; i++) {
			bool flagAdd = true;
			// Check if a particle is there
			for (int j = 0; j < numParticles; j++) {
				if (particles[j].indexCircle == i) {
					flagAdd = false;
					break;
				}
			}
			// If not, add the circle to possible circles
			if (flagAdd) {
				possibleCircles[index]=i;
				index++;
			}
		}

		// Shuffle possible circles
		shuffleIntArray(possibleCircles, NUM_CIRCLES-numParticles);

		// For each particle
		for (int i = 0; i < numParticles; i++) {
			// If its time is over
			if (millis() > particles[i].endTime) {
				// Create a new particle
				particles[i].endTime = 40+(rand()%1960)+millis();
				particles[i].hue = (rand()%16)*16;
				particles[i].indexCircle = possibleCircles[i];
			}
		}

		FastLED.clear();
		// For each particle
		for (int i = 0; i < numParticles; i++) {
			// Color the corresponding circle
			for (int j = 0; j < NUM_LEDS_PER_CIRCLE; j++) {
				leds[particles[i].indexCircle*NUM_LEDS_PER_CIRCLE+j] = CHSV(particles[i].hue, 255, 255);              
			}
		}
		FastLED.show();
		return false;
	}


	// Program has ended
	recordedTime = 0;
	FastLED.clear(true); // reset all led data (leds are off)
	return true;
}

bool danger(int givenTime) { // givenTime is in seconds	// blinking red lights
	// Variables to be used later
	static int timeCap = 300;	
	static bool odd;

	static int i;
	static int j;
	static int k;

	static unsigned long firstRecordedTime;

	// First time this function is called
	if (recordedTime == 0) {
		// Record time
		recordedTime = millis();
		firstRecordedTime = recordedTime;


		FastLED.setBrightness(10);
		timeToWait = 0;
		odd = true;
		
		i = 0;
		j = 0;
		k = 0;
	}

	// Wait
	if (millis() < recordedTime+timeToWait) {
		return false;
	} else {
		recordedTime = millis();
		timeToWait = 0;
	}

	if (millis() < (firstRecordedTime+min(givenTime, timeCap)*1000)) {
		// Time for which this pattern runs		
		FastLED.clear(); 
		// Alternate red circles
		for (i=0; i < NUM_CIRCLES; i++){ 
			if ((i+odd)%2){
				for (k = 0; k < NUM_LEDS_PER_CIRCLE; k++) { // Red
					// leds[i*NUM_LEDS_PER_CIRCLE+k] = CHSV(100, 255, 255);
					leds[i*NUM_LEDS_PER_CIRCLE+k] = CRGB(255, 0, 0);
				}
			}
		}
		FastLED.show();
		timeToWait = 100;
		odd = 1-odd;
		return false;
	}

	// Program has ended
	recordedTime = 0;
	FastLED.clear(true); // reset all led data (leds are off)
	DEBUGLN(F("Danger function ended"));
	return true;
}

//--- FFT patterns ---
void colorCircleFFT(int band, int barHeight);
void colorPeakFFT(int band, int peakHeight);

typedef struct {
	int value;
	int heightOld;
	int peak;
	unsigned long endTime;
} bandType;
bandType bands[NUM_CIRCLES] = {0,0,0,0};

bool vumeter(int notUsed) {
	if (!audioPaused) {
		// Acquire fft from sound
		float* result = fft.magnitudes();
		float* ptr = result;

		// Reset bandValues in bands
		for (int i = 0; i<NUM_CIRCLES; i++) {
			bands[i].value = 0;
		}
		
		
		// Skip 0Hz and close to 0Hz
		ptr++;
		ptr++;
		// Sort frequency magnitudes into bins
		for (int i = 2; i < fft.size(); i++) { // fft.size()=512

			*ptr /= volumeScaler; // adjusted with volume change

			if (*ptr > 50000) { // Crude noise filter
				// 13 bands, 12kHz top band
				if (i <= 3) {
					bands[0].value += (int)(*ptr);
				} else if (i > 3  && i <= 4) {
					bands[1].value += (int)(*ptr);
				} else if (i > 4  && i <= 6) {
					bands[2].value += (int)(*ptr);
				} else if (i > 6  && i <= 9) {
					bands[3].value += (int)(*ptr);
				} else if (i > 9  && i <= 14) {
					bands[4].value += (int)(*ptr);
				} else if (i > 14 && i <= 21) {
					bands[5].value += (int)(*ptr);
				} else if (i > 21 && i <= 32) {
					bands[6].value += (int)(*ptr);
				} else if (i > 32 && i <= 48) {
					bands[7].value += (int)(*ptr);
				} else if (i > 48 && i <= 73) {
					bands[8].value += (int)(*ptr);
				} else if (i > 73 && i <= 111) {
					bands[9].value += (int)(*ptr);
				} else if (i > 111 && i <= 168) {
					bands[10].value += (int)(*ptr);
				} else if (i > 168 && i <= 255) {
					bands[11].value += (int)(*ptr); //*1.5
				} else if (i > 255) { // && i<=387
					bands[12].value += (int)(*ptr); //*2
				}
			}
			ptr++;
		}
		// freq magnitude: 0 - x (x is de maximimale magnitude over het hele liedje/alle liedjes)
		// -> 0 - NUM_LEDS_PER_CIRCLE (maximale waardes voor hoogte band)
		// -> x/y = NUM_LEDS_PER_CIRCLE (y is een conversie factor) (moet wss een cap op)

		FastLED.clearData();

		for (int i = 0; i < NUM_CIRCLES; i++) {

			// Scale the bars for the display
			int height = bands[i].value / 120000; 
			if (height > NUM_LEDS_PER_CIRCLE) height = NUM_LEDS_PER_CIRCLE;

			// Small amount of averaging between frames
			height = ((bands[i].heightOld * 0.4) + (height * 1)) / 1.4;
		
			// Draw bar
			// rainbowBars(band, barHeight);
			colorCircleFFT(i, height);

			// Save oldBarHeights for averaging later
			bands[i].heightOld = height;

			// Processing the peak pixel
			if (height > bands[i].peak) {
				bands[i].peak = height;
				colorPeakFFT(i, bands[i].peak);
			} else if (bands[i].peak > 0) {
				if (millis()>bands[i].endTime+100) {
					colorPeakFFT(i, --bands[i].peak);
					bands[i].endTime = millis();
				} else {
					colorPeakFFT(i, bands[i].peak);
				}
			}
		}
		FastLED.show();	
		return false;
	} else {
		FastLED.clear(true); // reset all led data (leds are off)
		DEBUGLN(F("FFT lights function ended"));
		DEBUGF("audioPaused: %d\n", audioPaused);
		return true;
	}	
	return true;
}

void colorCircleFFT(int band, int barHeight) {
	CHSV color = CHSV(band * (255 / NUM_CIRCLES), 255, 255);
	for (int i = 0; i < barHeight; i++) {
		leds[band*NUM_LEDS_PER_CIRCLE+i] = color;
	}
	
	// for (int i = 0; i < barHeight; i++) {
	// 	CHSV color = CHSV(band * (255 / NUM_CIRCLES), int(255 * (1-(float)i / (float)NUM_LEDS_PER_CIRCLE)), 255);
	// 	leds[band*NUM_LEDS_PER_CIRCLE+i] = color;
	// }

	// for (int i = 0; i < barHeight; i++) {
	// 	CHSV color = CHSV(band * (255 / NUM_CIRCLES), (i + 1 != barHeight) ? 255 : 0, 255);
	// 	leds[band*NUM_LEDS_PER_CIRCLE+i] = color;
	// }
}

void colorPeakFFT(int band, int peakHeight) {
	if (peakHeight > 0) {
		CHSV color = CHSV(band * (255 / NUM_CIRCLES), 110, 255);
		leds[band*NUM_LEDS_PER_CIRCLE+peakHeight] = color;
	}
}

String morseCodeSentence;

std::map<char, String> morseCodeMap = {
    {'a', ".-"},    {'b', "-..."},  {'c', "-.-."},  {'d', "-.."},
    {'e', "."},     {'f', "..-."},  {'g', "--."},   {'h', "...."},
    {'i', ".."},    {'j', ".---"},  {'k', "-.-"},   {'l', ".-.."},
    {'m', "--"},    {'n', "-."},    {'o', "---"},   {'p', ".--."},
    {'q', "--.-"},  {'r', ".-."},   {'s', "..."},   {'t', "-"},
    {'u', "..-"},   {'v', "...-"},  {'w', ".--"},   {'x', "-..-"},
    {'y', "-.--"},  {'z', "--.."},

    {'1', ".----"}, {'2', "..---"}, {'3', "...--"}, {'4', "....-"},
    {'5', "....."}, {'6', "-...."}, {'7', "--..."}, {'8', "---.."},
    {'9', "----."}, {'0', "-----"},

    {'.', ".-.-.-"},
    {',', "--..--"},
    {'?', "..--.."},
    {'\'', ".----."},
    {'!', "-.-.--"},
    {'/', "-..-."},
    {'(', "-.--."},
    {')', "-.--.-"},
    {'&', ".-..."},
    {':', "---..."},
    {';', "-.-.-."},
    {'=', "-...-"},
    {'+', ".-.-."},
    {'-', "-....-"},
    {'_', "..--.-"},
    {'"', ".-..-."},
    {'$', "...-..-"},
    {'@', ".--.-."},

    {' ', " "}
};

String charToMorseCode(char character){
    if (morseCodeMap.find(tolower(character)) != morseCodeMap.end()) {
        return morseCodeMap[tolower(character)];
    }
    return "";
}

std::map<char, int> charToDelay = {
    {'|', 1}, // Delay between dots and dashes
    {'_', 3}, // Delay between characters
    {' ', 7}, // Delay between words
    {'.', 1},
    {'-', 3},
};

bool morsecode(int notUsed) {
    static int timeUnit = 100; // sms
    static String morseCode;
    static int index;

    // First time this function is called
    if (recordedTime == 0) {
        recordedTime = millis();
        timeToWait = 0;
        FastLED.setBrightness(10);

        // Convert sentence to morse code
        morseCode = "";
        for (int i = 0; i < morseCodeSentence.length(); i++) {
            String morseChar = charToMorseCode(morseCodeSentence[i]);
            for (int j = 0; j < morseChar.length(); j++) {
                morseCode += morseChar[j];
                // Intra-character gap
                if (j != morseChar.length()-1) {
                    morseCode += "|";
                }
            }

            // Inter-character gap
            if (i != morseCodeSentence.length()-1 && (morseCodeSentence[i] != ' ' && morseCodeSentence[i+1] != ' ')) {
                morseCode += "_";
            }
        }

        // Reset just to be sure
        index = 0;

        // Some checks
        DEBUGLN(morseCode);
        DEBUGLN(morseCode.length());
    }

    // Wait
    if (millis() < recordedTime+timeToWait) {
        return false;
    } else {
        recordedTime = millis();
        timeToWait = 0;
    }

    // Function has ended
    if (index >= morseCode.length()) {
        recordedTime = 0;
        FastLED.clear(true); // reset all led data (leds are off)
        DEBUGLN(F("Morsecode function ended"));
        return true;
    }

    FastLED.clear();

    // Turn leds on if dash or dot
    if (morseCode[index] == '.' || morseCode[index] == '-') {
        for (int i=0; i < NUM_CIRCLES; i++){ 
            for (int k = 0; k < NUM_LEDS_PER_CIRCLE; k++) { // Red
                leds[i*NUM_LEDS_PER_CIRCLE+k] = CRGB(255, 0, 0);
            }
        }
    }

    // Delay
    timeToWait = charToDelay[morseCode[index++]]*timeUnit;
    FastLED.show();
    return false;
}
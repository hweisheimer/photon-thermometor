const int TEMP_PIN = A0;
const int DISP_DATA_PIN = D0;
const int DISP_CLOCK_PIN = D1;
const int DISP_LATCH_PIN = D2;
const int DISP_SELECT_PINS[4] = { D3, D4, D5, D6 };

const byte DIGIT_SEGMENTS[10] = {
    0b11111100,
    0b01100000,
    0b11011010,
    0b11110010,
    0b01100110,
    0b10110110,
    0b10111110,
    0b11100000,
    0b11111110,
    0b11110110
};

const int DISP_DECIMAL_PLACES = 1;

const double TEMP_SENSOR_MV_OFFSET = 424;
const double TEMP_SENSOR_MV_PER_CELCIUS = 6.25;
const double TEMP_SENSOR_ERROR_CELCIUS = -1.5;
const double ANALOG_RAW_TO_MV_MULT = 0.8; // 3.3volts / 4096 values

double tempC = 0.0;
double tempF = 0.0;
int tempRaw = 0;
long lastReadMillis = 0;
const int READ_INTERVAL_MILLIS = 1000;
    
void setup() {    
    pinMode(DISP_DATA_PIN, OUTPUT);
    pinMode(DISP_CLOCK_PIN, OUTPUT);
    pinMode(DISP_LATCH_PIN, OUTPUT);
    
    for (int i = 0; i < 4; i++) {
        pinMode(DISP_SELECT_PINS[i], OUTPUT);
    }
    
    unlatch();
    
    pinMode(TEMP_PIN, INPUT);
    
    Particle.variable("tempIndoorF", tempF);
    Particle.variable("tempRaw", tempRaw);
}

void loop() {
    long now = millis();
    // first iteration, millis() overflow, or interval reached
    if (lastReadMillis == 0 || now < lastReadMillis || now - lastReadMillis > READ_INTERVAL_MILLIS) {
        lastReadMillis = now;
        readTemp();
    }
    
    // TODO: enhance with adjustable decimal places
    // int multiplier = 10 ^ DISP_DECIMAL_PLACES;
    
    // TODO: remove leading zeroes
    // TODO: negative number support (requires circuit changes, as well)
    
    int tempTemp = int(tempF * 10);
    int digits[4] = {
        tempTemp / 1000,
        tempTemp % 1000 / 100,
        tempTemp % 1000 % 100 / 10,
        tempTemp % 1000 % 100 % 10
    };
    
    for (int i = 0; i < 4; i++) {
        // select display digit (using PNP transistors, so LOW = on)
        for (int j = 0; j < 4; j++) {
            digitalWrite(DISP_SELECT_PINS[j], HIGH);
        }
        digitalWrite(DISP_SELECT_PINS[i], LOW);
        
        bool writeDecimalPoint = (i == 2 ? true : false);
        writeDigit(digits[i], writeDecimalPoint);
        latch();
        delayMicroseconds(1000);
        unlatch();
    }
    
    delay(5);
}

void readTemp() {
    tempRaw = analogRead(TEMP_PIN);
    tempC = ((tempRaw * ANALOG_RAW_TO_MV_MULT) - TEMP_SENSOR_MV_OFFSET) / TEMP_SENSOR_MV_PER_CELCIUS + TEMP_SENSOR_ERROR_CELCIUS;
    tempF = tempC * 1.8 + 32.0;
}

void latch() {
    digitalWrite(DISP_LATCH_PIN, LOW);
}

void unlatch() {
    digitalWrite(DISP_LATCH_PIN, HIGH);
}

void writeDigit(int digit, boolean includeDecimal) {
    // segment 8 = decimal point
    byte segments = DIGIT_SEGMENTS[digit] | (includeDecimal ? 0b00000001 : 0b00000000);
    shiftOut(DISP_DATA_PIN, DISP_CLOCK_PIN, LSBFIRST, segments);
}
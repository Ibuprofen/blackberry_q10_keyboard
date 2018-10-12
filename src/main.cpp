#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Keypad.h>
#include <U8g2lib.h>

// display
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
const byte LCDROWS = 5;
const byte LCDCOLS = 21;
const byte FONTHEIGHT = 11;
char buf[LCDROWS][LCDCOLS + 1];
// cursor
short int pos = 0;

// keyboard
const byte ROWS = 7;
const byte COLS = 5;

const byte KS1 = '^'; // case shift character
const byte KS2 = '%'; // symbol lock character

char lowerKeys[ROWS][COLS] = {
    {'q', 'e', 'r', 'u', 'o'},
    {'w', 's', 'g', 'h', 'l'},
    {KS2, 'd', 't', 'y', 'i'},
    {'a', 'p', KS1, '{', '<'},
    {'>', 'x', 'v', 'b', '$'},
    {' ', 'z', 'c', 'n', 'm'},
    {'&', KS1, 'f', 'j', 'k'}
};

char upperKeys[ROWS][COLS] = {
    {'Q', 'E', 'R', 'U', 'O'},
    {'W', 'S', 'G', 'H', 'L'},
    {KS2, 'D', 'T', 'Y', 'I'},
    {'A', 'P', KS1, '{', '<'},
    {'>', 'X', 'V', 'B', '$'},
    {' ', 'Z', 'C', 'N', 'M'},
    {'&', KS1, 'F', 'J', 'K'}
};

char symKeys[ROWS][COLS] = {
    {'#', '2', '3', '_', '+'},
    {'1', '4', '/', ':', '"'},
    {KS2, '5', '(', ')', '-'},
    {'*', '@', KS1, '{', '<'},
    {'>', '8', '?', '!', '$'},
    {' ', '7', '9', ',', '.'},
    {'0', KS1, '6', ';', '\''}
};

// all numbers are +1 since 0 is equal to NO_KEY
// we're using this keyboard as a lookup table for the
// above character arrays
char indexKeys[ROWS][COLS] = {
    { 1,  2,  3,  4,  5},
    { 6,  7,  8,  9, 10},
    {11, 12, 13, 14, 15},
    {16, 17, 18, 19, 20},
    {21, 22, 23, 24, 25},
    {26, 27, 28, 29, 30},
    {31, 32, 33, 34, 35}
};

byte rowPins[ROWS] = {0, 1, 2, 3, 4, 5, 6};
byte colPins[COLS] = {7, 8, 9, 10, 11};

Keypad indexedKeypad = Keypad(makeKeymap(indexKeys), rowPins, colPins, ROWS, COLS);

bool symbolShift = false;
bool capShift = false;


void setupDisplay(void) {
    u8g2.begin();
    u8g2.setFont(u8g2_font_t0_11_mr);
    u8g2.clearDisplay();
}

void draw(void) {
    u8g2.clearBuffer(); // clear the internal memory

    for (int i = 0; i < LCDROWS; i++) {
        // draw each row
        u8g2.drawStr(0, FONTHEIGHT * (i + 1), buf[i]);
    }

    u8g2.sendBuffer(); // transfer internal memory to the display
}

char setKeyXY(int n, int & x, int & y) {
    // account for off by one since indexKeys starts at 1
    if (n > 0) { --n; }

    if (n < COLS) {
        x = 0;
        y = n;
    } else {
        x = n / COLS;
        y = n % COLS;
    }
}

bool isSymShift(int x, int y) {
    return lowerKeys[x][y] == KS2;
}

bool isCaseShift(int x, int y) {
    return lowerKeys[x][y] == KS1;
}

char getKeypadChar() {
    char key = NO_KEY;
    char indexKeyChar = indexedKeypad.getKey();

    if (indexKeyChar != NO_KEY) {
        int x, y;
        setKeyXY((int)indexKeyChar, x, y);

        if (isSymShift(x, y)) {
            symbolShift = !symbolShift;
            capShift = false;
        } else if (isCaseShift(x, y)) {
            capShift = true;
        } else {
            if (symbolShift) {
                key = symKeys[x][y];
            } else if (capShift) {
                key = upperKeys[x][y];
                capShift = false;
            } else {
                key = lowerKeys[x][y];
            }
        }
    }
    return key;
}

void clearBuf() {
    for (int i = 0; i < LCDROWS; i++) {
        for (int j = 0; j < LCDCOLS; j++) {
            buf[i][j] = 0;
        }
    }
}

void setup() {
    Serial.begin(115200);

    clearBuf();
    setupDisplay();

    Serial.println("end of setup");
}

void loop() {
    char key = getKeypadChar();

    if (key != NO_KEY) {
        Serial.println(key);

        // basic cursor position
        if (pos == LCDROWS * LCDCOLS) {
            pos = 0;
            clearBuf();
            buf[0][0] = key;
        } else {
            int x = pos / LCDCOLS;
            int y = pos % LCDCOLS;
            buf[x][y] = key;
        }
        ++pos;

        u8g2.firstPage();
        do {
            draw();
        } while (u8g2.nextPage());
    }
}

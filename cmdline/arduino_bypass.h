#define PROGMEM
using byte = unsigned char;
using __FlashStringHelper = char;
#define F(X) (X)
#define ARDUINO_VOLATILE

void WriteAt(short x, short y, const char* text)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos = { x, y };
    SetConsoleCursorPosition(hConsole, pos);
    DWORD written;
    WriteConsoleA(hConsole, text, static_cast<DWORD>(strlen(text)), &written, nullptr);
}

void ClearConsole() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD count;
    DWORD cellCount;
    COORD homeCoords = { 0, 0 };

    if (hConsole == INVALID_HANDLE_VALUE) return;

    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) return;
    cellCount = csbi.dwSize.X * csbi.dwSize.Y;

    FillConsoleOutputCharacter(hConsole, ' ', cellCount, homeCoords, &count);
    FillConsoleOutputAttribute(hConsole, csbi.wAttributes, cellCount, homeCoords, &count);
    SetConsoleCursorPosition(hConsole, homeCoords);
}

#include "../transient-38/keys.h"

class KEYPAD
{
public:
    bool isPressed(int x) { return false; };
    bool consumePressed(int x) { return false; };
    bool consumePressedDigit(int x) { return false; };
} keypad;

class LCD
{
    int x, y;
public:
    void print(const char* str)
    {
        WriteAt(x, y, str);
        x += strlen(str);
    }

    void print(byte b)
    {
        char temp[1024];
        sprintf_s(temp, 1024, "%d", b);
        print(temp);
    }

    void write(char c)
    {
        char temp[1024];
        sprintf_s(temp, 1024, "%c", c);
        print(temp);
    }

    void clear()
    {
        ClearConsole();
    }

    void setCursor(int x_, int y_)
    {
        x = x_;
        y = y_;
    }
} lcd;

uint16_t pgm_read_word(const void* addr)
{
    return *(static_cast<const uint16_t*>(addr));
}

byte pgm_read_byte(const void* addr)
{
    return *(static_cast<const byte*>(addr));
}

byte pgm_read_byte_near(const void* addr)
{
    return *(static_cast<const byte*>(addr));
}


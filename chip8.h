#ifndef CHIP8_H
#define CHIP8_H

class chip8
{
public:
    chip8();

    void initialize();
    bool loadGame(const char *filename);
    void emulateCycle();
    void setKeys(unsigned char key[]);

    void consoleRender();

    bool drawFlag;
    bool isBeep;
    unsigned char gfx[64 * 32]; // Graphics (64 * 32 = 2048 pixels)

    unsigned short getPC();

private:
    unsigned short opcode;      // 35 opcodes (2 bytes = 16 bits)

    /*
     * Memory Map: (0 ~ 4095)
     * 0x000-0x1FF - Chip 8 interpreter (contains font set in emu)
     * 0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
     * 0x200-0xFFF - Program ROM and work RAM
     */
    unsigned char memory[4096]; // 4K Memory

    unsigned char V[16];        // 15 8-bit general purpose registers named V0, V1, ... , VE
    unsigned short I;           // Index register I
    unsigned short PC;          // Program counter 0x0000 ~ 0x0FFF

    unsigned char delay_timer;  // 60 Hz
    unsigned char sound_timer;  // 60 Hz

    unsigned short stack[16];   // Stack
    unsigned short sp;          // Stack pointer

    unsigned char key[16];      // HEX based keypad (0x0-0xF)
};

#endif // CHIP8_H

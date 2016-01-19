#include "chip8.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

using namespace std;

unsigned char chip8_fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

chip8::chip8()
{
    srand((unsigned) time(NULL));
}

void chip8::initialize()
{
    // Reset opcode
    this->opcode = 0;

    // Reset index register
    this->I = 0;

    // Reset stack pointer
    this->sp = 0;

    // Reset timers
    this->delay_timer = 0;
    this->sound_timer = 0;

    // Reset ...
    memset(this->memory, 0, sizeof(unsigned char) * 4096);
    memset(this->V, 0, sizeof(unsigned char) * 16);
    memset(this->gfx, 0, sizeof(unsigned char) * 64 * 32);
    memset(this->stack, 0, sizeof(unsigned short) * 16);
    memset(this->key, 0, sizeof(unsigned char) * 16);

    // Program counter starts at 0x200
    this->PC = 0x0200;

    this->drawFlag = false;
    this->isBeep = false;

    // Load fontset to memory (80 bytes)
    memcpy(this->memory, chip8_fontset, sizeof(unsigned char) * 80);
}

bool chip8::loadGame(const char *filename)
{
    FILE *fp = fopen(filename, "rb");
    unsigned char *dst = &memory[0] + 0x0200;
    for (size_t rb = 0; (rb = fread(dst, 1, 512, fp)) > 0; dst += rb) {
        //printf("%d\n", rb);
    }
    fclose(fp);

    return true;
}

void chip8::emulateCycle()
{
    // Fetch Opcode
    opcode = (memory[PC] << 8) | memory[PC+1];

    //printf("OP: 0x%X, PC: 0x%X, SP: %d, I: 0x%X, V0: 0x%X\n", opcode, PC, sp, I, V[0]);

    // Decode and Execute Opcode
    switch (opcode & 0xF000)
    {
    case 0x0000:
    {
        if (opcode == 0x00E0) {
            // 00E0: Clears the screen
            memset(gfx, 0, sizeof(char) * 64 * 32);
            drawFlag = true;
            PC += 2;
        } else if (opcode == 0x00EE) {
            // 00EE: Returns from a subroutine
            PC = stack[--sp];
            PC += 2;
        } else {
            // 0NNN: Calls RCA 1802 program at address NNN. Not necessary for most ROMs
            // TODO
            //PC += 2;
        }
    }
    break;

    // 1NNN: Jumps to address NNN
    case 0x1000:
    {
        PC = opcode & 0x0FFF;
    }
    break;

    // 2NNN: Calls subroutine at NNN
    case 0x2000:
    {
        stack[sp++] = PC;
        PC = (opcode & 0x0FFF);
    }
    break;

    // 3XNN: Skips the next instruction if VX equals NN
    case 0x3000:
    {
        PC += (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) ? 4 : 2;
    }
    break;

    // 4XNN: Skips the next instruction if VX doesn't equal NN
    case 0x4000:
    {
        PC += (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) ? 4 : 2;
    }
    break;

    // 5XY0: Skips the next instruction if VX equals VY
    case 0x5000:
    {
        PC += (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4]) ? 4 : 2;
    }
    break;

    // 6XNN: Sets VX to NN
    case 0x6000:
    {
        V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
        PC += 2;
    }
    break;

    // 7XNN: Adds NN to VX
    case 0x7000:
    {
        V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
        PC += 2;
    }
    break;

    // 8XXI:
    case 0x8000:
    {
        unsigned short lastbit = opcode & 0x000F;
        if (lastbit == 0x0000) {
            // 8XY0: Sets VX to the value of VY
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
        } else if (lastbit == 0x0001) {
            // 8XY1: Sets VX to VX or VY
            V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
        } else if (lastbit == 0x0002) {
            // 8XY2: Sets VX to VX and VY
            V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
        } else if (lastbit == 0x0003) {
            // 8XY3: Sets VX to VX xor VY
            V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
        } else if (lastbit == 0x0004) {
            // 8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't
            V[0xF] = V[(opcode & 0x0F00) >> 8] > (0xFF - V[(opcode & 0x00F0) >> 4]);
            V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
        } else if (lastbit == 0x0005) {
            // 8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't
            V[0xF] = V[(opcode & 0x0F00) >> 8] >= V[(opcode & 0x00F0) >> 4];
            V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
        } else if (lastbit == 0x0006) {
            // 8XY6: Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift
            V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
            V[(opcode & 0x0F00) >> 8] >>= 1;
        } else if (lastbit == 0x0007) {
            // 8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't
            V[0xF] = ( V[(opcode & 0x0F00) >> 8] <= V[(opcode & 0x00F0) >> 4] );	// VY-VX
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
        } else if (lastbit == 0x000E) {
            // 8XYE: Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift
            V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
            V[(opcode & 0x0F00) >> 8] <<= 1;
        } else {
            printf("Unknown opcode: 0x%X\n", opcode);
        }
        PC += 2;
    }
    break;

    // 9XY0: Skips the next instruction if VX doesn't equal VY
    case 0x9000:
    {
        PC += (V[opcode & 0x0F00] != V[opcode & 0x00F0]) ? 4 : 2;
    }
    break;

    // ANNN: Sets I to the address NNN
    case 0xA000:
    {
        I = opcode & 0x0FFF;
        PC += 2;
    }
    break;

    // BNNN: Jumps to the address NNN plus V0.
    case 0xB000:
    {
        PC = (opcode & 0x0FFF) + V[0];
    }
    break;

    // CXNN: Sets VX to the result of a bitwise and operation on a random number and NN
    case 0xC000:
    {
        V[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
        PC += 2;
    }
    break;

    // DXYN: Sprites stored in memory at location in index register (I), 8bits wide.
    // Wraps around the screen. If when drawn, clears a pixel, register VF is set to 1 otherwise it is zero.
    // All drawing is XOR drawing (i.e. it toggles the screen pixels).
    // Sprites are drawn starting at position (VX, VY).
    // N is the number of 8bit rows that need to be drawn.
    // If N is greater than 1, second line continues at position (VX, VY+1), and so on.
    case 0xD000:
    {
        unsigned char x = V[(opcode & 0x0F00) >> 8];
        unsigned char y = V[(opcode & 0x00F0) >> 4];
        unsigned char rows = opcode & 0x000F;
        unsigned short pixel;

        //printf("%d, %d\n", x, y);

        V[0xF] = 0;
        for (int yline = 0; yline < rows; yline++)
        {
            pixel = memory[I + yline];
            for(int xline = 0; xline < 8; xline++)
            {
                if((pixel & (0x80 >> xline)) != 0)
                {
                    V[0xF] = gfx[(x + xline + ((y + yline) * 64))] == 1;
                    gfx[x + xline + ((y + yline) * 64)] ^= 1;
                }
            }
        }

        drawFlag = true;
        PC += 2;
    }
    break;

    // EX9E: Skips the next instruction if the key stored in VX is pressed.
    // EXA1: Skips the next instruction if the key stored in VX isn't pressed.
    case 0xE000:
    {
        if ((opcode & 0x00FF) == 0x009E) {
            PC += (key[V[(opcode & 0x0F00) >> 8]] != 0) ? 4 : 2;
        } else if ((opcode & 0x00FF) == 0x00A1) {
            PC += (key[V[(opcode & 0x0F00) >> 8]] == 0) ? 4 : 2;
        } else {
            printf("Unknown opcode: 0x%X\n", opcode);
        }
    }
    break;

    // 0xFAAA:
    case 0xF000:
    {
        switch (opcode & 0x00FF)
        {
        // FX07: Sets VX to the value of the delay timer
        case 0x0007:
        {
            V[(opcode & 0x0F00) >> 8] = delay_timer;
            PC += 2;
        }
        break;

        // FX0A: A key press is awaited, and then stored in VX
        case 0x000A:
        {
            bool keyPress = false;

            for (int i = 0; i < 16; ++i)
            {
                if (key[i] != 0)
                {
                    V[(opcode & 0x0F00) >> 8] = i;
                    keyPress = true;
                }
            }

            // If we didn't received a keypress, skip this cycle and try again.
            if (!keyPress) {
                return;
            }

            PC += 2;
        }
        break;

        // FX15: Sets the delay timer to VX
        case 0x0015:
        {
            delay_timer = V[(opcode & 0x0F00) >> 8];
            PC += 2;
        }
        break;

        // FX18: Sets the sound timer to VX
        case 0x0018:
        {
            sound_timer = V[(opcode & 0x0F00) >> 8];
            PC += 2;
        }
        break;

        // FX1E: Adds VX to I
        // VF is set to 1 when range overflow (I+VX>0xFFF), and 0 when there isn't.
        case 0x001E:
        {
            V[0xF] = (I + V[(opcode & 0x0F00) >> 8]) > 0xFFF;
            I += V[(opcode & 0x0F00) >> 8];
            PC += 2;
        }
        break;

        // FX29: Sets I to the location of the sprite for the character in VX.
        // Characters 0-F (in hexadecimal) are represented by a 4x5 font.
        case 0x0029:
        {
            I = V[(opcode & 0x0F00) >> 8] * 0x5;
            PC += 2;
        }
        break;

        // FX33: Stores the Binary-coded decimal representation of VX,
        // with the most significant of three digits at the address in I,
        // the middle digit at I plus 1, and the least significant digit at I plus 2.
        // (In other words, take the decimal representation of VX,
        // place the hundreds digit in memory at location in I,
        // the tens digit at location I+1, and the ones digit at location I+2.)
        case 0x0033:
        {
            memory[I]     =  V[(opcode & 0x0F00) >> 8] / 100;
            memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
            memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
            PC += 2;
        }
        break;

        // FX55: Stores V0 to VX in memory starting at address I
        case 0x0055:
        {
            memcpy(memory + I, V, sizeof(char) * ((opcode & 0x0F00) >> 8));
            // On the original interpreter, when the operation is done, I = I + X + 1.
            I += ((opcode & 0x0F00) >> 8) + 1;
            PC += 2;
        }
        break;

        // FX65: Fills V0 to VX with values from memory starting at address I
        case 0x0065:
        {
            memcpy(V, &memory[I], sizeof(char) * (((opcode & 0x0F00) >> 8) + 1));
            // On the original interpreter, when the operation is done, I = I + X + 1.
            I += ((opcode & 0x0F00) >> 8) + 1;
            PC += 2;
        }
        break;
        }
    }
    break;

    // Unknown opcode
    default:
        printf("Unknown opcode: 0x%X\n", opcode);
    }

    // Update timers
    if (delay_timer > 0)
    {
        --delay_timer;
    }

    if (sound_timer > 0)
    {
        if (sound_timer == 1)
        {
            isBeep = true;
        }
        --sound_timer;
    }
}

void chip8::setKeys(unsigned char key[])
{
    memcpy(this->key, key, sizeof(char) * 16);
}

void chip8::consoleRender()
{
    system("cls");
    for (int y = 0; y < 32; ++y)
    {
        for (int x = 0; x < 64; ++x)
        {
            if (gfx[(y*64) + x] == 0)
                printf(" ");
            else
                printf("#");
        }
        printf("\n");
    }
    printf("\n");
}

unsigned short chip8::getPC()
{
    return PC;
}

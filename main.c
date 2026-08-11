#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>


typedef struct {
    uint8_t memory[4096];
    uint8_t V[16];
    uint16_t pc;
    uint16_t stack[16];
    uint8_t stackLocation;
    uint16_t I;
    uint8_t screen[32][64]; // y-x çünkü satırları sabit tutmak daha rahat


    // Opcodeların F grubunda kullanılıyor onun için ekledim
    uint8_t delayTimer;
    uint8_t soundTimer;

    uint8_t keys[16];

} Chip8;


void drawScreen(Chip8 *chip8)
{
    printf("\033[H\033[J");
    for (int y = 0; y < 32; y++)
    {
        for (int x = 0; x < 64; x++)
        {
            printf("%c", chip8->screen[y][x] ? 'X' : ' ');
        }
        printf("\n");
    }
}

uint8_t font[80] = {
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

void readRom(Chip8 *chip8, const char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (file == NULL)
    {
        printf("Error opening file: %s\n", filename);
        return;
    }

    // ROM'u 0x200'den itibaren belleğe alıyorum
    fread(&chip8->memory[0x200], 1, 4096 - 0x200, file);
    fclose(file);
}

int main(void)
{
    Chip8 chip8 = {0};

    for (int i = 0; i < 80; i++)
    {
        chip8.memory[0x50 + i] = font[i];
    }

    chip8.pc = 0x200; // Bu bulunduğumuz yeri gösteriyor

    readRom(&chip8, "C:\\Users\\CemII-2\\Downloads\\C Softwares\\Chip8Emulator\\IBMLogo.ch8");

    int timer = 0;

    while (1)
    {
        if (timer >= 10)
        {
            if (chip8.delayTimer > 0)
            {
                chip8.delayTimer--;
            }
            if (chip8.soundTimer > 0)
            {
                chip8.soundTimer--;
            }
            timer = 0;
        }


        // FETCH yani sayacın gösterdiği yerden 2 byte'ı çekip birleştirme işlemi
        uint16_t opcode = (chip8.memory[chip8.pc] << 8) | chip8.memory[chip8.pc + 1];

        // bir sonraki yere ilerlet
        chip8.pc += 2;

        // o yerin ilk rakamını/harfini çekip hangi komut olduğunu anlamamız gerekiyor. Buna opcode denir.
        uint8_t first = (opcode & 0xF000) >> 12;

        switch (first)
        {
            case 0x6:  // V[x]'e nn sayısını koy
            {
                uint8_t x = (opcode & 0x0F00) >> 8;
                uint8_t nn = opcode & 0x00FF;

                chip8.V[x] = nn;
                break;
            }

            case 0x7:  // V[x]'e nn'i ekle
            {
                uint8_t x = (opcode & 0x0F00) >> 8;
                uint8_t nn = opcode & 0x00FF;

                chip8.V[x] += nn;
                break;
            }

            case 0x3:  // V[x] nn'e eşitse bir komut atla
            {
                uint8_t x = (opcode & 0x0F00) >> 8;
                uint8_t nn = opcode & 0x00FF;

                if (chip8.V[x] == nn)
                {
                    chip8.pc += 2;
                }

                break;
            }

            case 0x4:  // V[x] nn'e eşit değilse bir komut atla
            {
                uint8_t x = (opcode & 0x0F00) >> 8;
                uint8_t nn = opcode & 0x00FF;

                if (chip8.V[x] != nn)
                {
                    chip8.pc += 2;
                }

                break;
            }

            case 0x5:  //  V[x] ile V[y] eşitse bir komut atla
            {
                uint8_t x = (opcode & 0x0F00) >> 8;
                uint8_t y = (opcode & 0x00F0) >> 4;

                if (chip8.V[x] == chip8.V[y])
                {
                    chip8.pc += 2;
                }

                break;
            }

            case 0x9:  // V[x] ile V[y] eşit değilse bir komut atla
            {
                uint8_t x = (opcode & 0x0F00) >> 8;
                uint8_t y = (opcode & 0x00F0) >> 4;

                if (chip8.V[x] != chip8.V[y])
                {
                    chip8.pc += 2;
                }

                break;
            }

            case 0x1:  // nnn adresine git
            {
                uint16_t nnn = opcode & 0x0FFF;

                chip8.pc = nnn;

                break;
            }

            case 0x2:  // alt program çağır (dönüş için pc'yi stack'e sakla, sonra atla)
            {
                uint16_t nnn = opcode & 0x0FFF;

                chip8.stack[chip8.stackLocation] = chip8.pc;
                chip8.stackLocation++;

                chip8.pc = nnn;

                break;
            }

            case 0xB:  // V[0] + nnn adresine atla
            {
                uint16_t nnn = opcode & 0x0FFF;

                chip8.pc = nnn + chip8.V[0];

                break;
            }

            case 0xC:  // rastgele sayı üretip, nn ile maskeleyip, V[x]'e koy
            {
                uint8_t x = (opcode & 0x0F00) >> 8;
                uint8_t nn = opcode & 0x00FF;

                chip8.V[x] = (rand() % 256) & nn;

                break;
            }

            case 0x8:  // iki register arası matematik işlemleri
            {
                uint8_t x = (opcode & 0x0F00) >> 8;
                uint8_t y = (opcode & 0x00F0) >> 4;
                uint8_t last = opcode & 0x000F;

                switch (last)
                {
                    case 0x0:  // V[x] = V[y]
                        chip8.V[x] = chip8.V[y];
                        break;

                    case 0x1:  // V[x] = V[x] OR V[y]
                        chip8.V[x] = chip8.V[x] | chip8.V[y];
                        break;

                    case 0x2:  // V[x] = V[x] AND V[y]
                        chip8.V[x] = chip8.V[x] & chip8.V[y];
                        break;

                    case 0x3:  // V[x] = V[x] XOR V[y]
                        chip8.V[x] = chip8.V[x] ^ chip8.V[y];
                        break;

                    case 0x4:  // V[x] += V[y]. Ama bu 255'i geçerse V[F]=1 yapıyoruz
                    {
                        uint16_t sum = chip8.V[x] + chip8.V[y];
                        if (sum > 255)
                        {
                            chip8.V[0xF] = 1;
                        }
                        else
                        {
                            chip8.V[0xF] = 0;
                        }
                        chip8.V[x] = sum & 0xFF;
                        break;
                    }

                    case 0x5:  // V[x] -= V[y]. Kalan yoksa V[F]=1 varsa 0
                    {
                        uint16_t diff = chip8.V[x] - chip8.V[y];
                        if (chip8.V[x] >= chip8.V[y])
                        {
                            chip8.V[0xF] = 1;
                        }
                        else
                        {
                            chip8.V[0xF] = 0;
                        }
                        chip8.V[x] = diff & 0xFF;
                        break;
                    }

                    case 0x6:  // V[x]'i sağa kaydırıyoruz (ikiye bölüyoruz yani). Düşen bit V[F]'e gidiyor
                        chip8.V[0xF] = chip8.V[x] & 0x1;
                        chip8.V[x] >>= 1;

                        break;

                    case 0x7:  // V[x] = V[y] - V[x] kalan yoksa V[F]=1
                        if (chip8.V[y] >= chip8.V[x])
                        {
                            chip8.V[0xF] = 1;
                        }
                        else
                        {
                            chip8.V[0xF] = 0;
                        }
                        chip8.V[x] = chip8.V[y] - chip8.V[x];
                        break;

                    case 0xE:  //  V[x]'i sola kaydır (ikiyle çarpma işlemi). Düşen bit V[F]'e
                        chip8.V[0xF] = (chip8.V[x] >> 7) & 1;
                        chip8.V[x] <<= 1;

                        break;
                }

                break;
            }

            case 0x0:
            {
                if (opcode == 0x00E0) // ekranı temizle (hepsini söndür)
                {
                    for (int y = 0; y < 32; y++)
                    {
                        for (int x = 0; x < 64; x++)
                        {
                            chip8.screen[y][x] = 0;
                        }
                    }

                }
                else if (opcode == 0x00EE) // alt programdan dön (stack'ten pc'yi geri al)
                {
                    chip8.stackLocation--;
                    chip8.pc = chip8.stack[chip8.stackLocation];
                }

                break;
            }

            case 0xA:  // I register'ına nnn adresini koy
            {
                uint16_t nnn = opcode & 0x0FFF;

                chip8.I = nnn;

                break;
            }

            case 0xD:  // ekrana sprite çizdirme işlemi
            {
                uint8_t x = (opcode & 0x0F00) >> 8;
                uint8_t y = (opcode & 0x00F0) >> 4;
                uint8_t n = opcode & 0x000F;
                uint8_t Row, Column;
                Row = chip8.V[y];
                Column = chip8.V[x];
                chip8.V[0xF] = 0;

                uint8_t nConstant = n;
                for(n = 0; n < nConstant; n++)
                {
                    uint8_t pixel = chip8.memory[chip8.I + n];
                    for(int bit = 0; bit < 8; bit++)
                    {
                        if((pixel & (0x80 >> bit)) != 0)
                        {
                            int screenX = (Column + bit) % 64;
                            int screenY = (Row + n) % 32;

                            if(chip8.screen[screenY][screenX] == 1)
                            {
                                chip8.V[0xF] = 1;
                            }

                            chip8.screen[screenY][screenX] ^= 1;
                        }
                    }
                }


                drawScreen(&chip8);
                break;
            }

            case 0xE:  // klavye: tuş durumuna göre atla
            {
                uint8_t x = (opcode & 0x0F00) >> 8;
                uint8_t last = opcode & 0x00FF;

                switch (last)
                {
                    case 0x9E:   // tuş basılıysa atla
                        if (chip8.keys[chip8.V[x]] == 1)
                        {
                            chip8.pc += 2;
                        }
                        break;
                    case 0xA1:   // tuş basılı değilse atla
                        if (chip8.keys[chip8.V[x]] == 0)
                        {
                            chip8.pc += 2;
                        }
                        break;
                }
                break;
            }

            case 0xF:
            {
                uint8_t x = (opcode & 0x0F00) >> 8;
                uint8_t last = opcode & 0x00FF;

                switch (last)
                {
                    case 0x07:
                        chip8.V[x] = chip8.delayTimer;
                        break;
                    case 0x15:
                        chip8.delayTimer = chip8.V[x];
                        break;
                    case 0x18:
                        chip8.soundTimer = chip8.V[x];
                        break;
                    case 0x1E:
                        chip8.I += chip8.V[x];
                        break;
                    case 0x0A:  // tusa basilmasini bekle
                    {
                        int found = 0;
                        for (int i = 0; i < 16; i++)
                        {
                            if (chip8.keys[i] == 1)
                            {
                                chip8.V[x] = i;
                                found = 1;
                                break;
                            }
                        }
                        if (found == 0)
                        {
                            chip8.pc -= 2;  // tus yok, bu opcode'u tekrarla (bekle)
                        }
                        break;
                    }
                    case 0x33:
                        chip8.memory[chip8.I] = chip8.V[x] / 100;
                        chip8.memory[chip8.I + 1] = (chip8.V[x] / 10) % 10;
                        chip8.memory[chip8.I + 2] = chip8.V[x] % 10;
                        break;
                    case 0x55:
                        for (int i = 0; i <= x; i++)
                        {
                            chip8.memory[chip8.I + i] = chip8.V[i];
                        }
                        break;
                    case 0x65:
                        for (int i = 0; i <= x; i++)
                        {
                            chip8.V[i] = chip8.memory[chip8.I + i];
                        }
                        break;
                    case 0x29:
                        chip8.I = 0x50 + (chip8.V[x] * 5); //Bütün sayılar 5 byte olduğu için 5 ile çarpıp ekliyorum
                        break;

                }
                break;
            }
        }

        timer++;
        Sleep(2);
    }

    return 0;
}

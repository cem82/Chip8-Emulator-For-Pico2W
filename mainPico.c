#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include <stdint.h>
#include <stdlib.h>

#ifdef CYW43_WL_GPIO_LED_PIN
#include "pico/cyw43_arch.h"
#endif

#define I2C_SDA 0
#define I2C_SCL 1
#define BTN_LEFT  14  
#define BTN_RIGHT 15   

uint8_t rom[] = {
    0x6E, 0x05, 0x65, 0x00, 0x6B, 0x06, 0x6A, 0x00, 0xA3, 0x0C, 0xDA, 0xB1,
    0x7A, 0x04, 0x3A, 0x40, 0x12, 0x08, 0x7B, 0x02, 0x3B, 0x12, 0x12, 0x06,
    0x6C, 0x20, 0x6D, 0x1F, 0xA3, 0x10, 0xDC, 0xD1, 0x22, 0xF6, 0x60, 0x00,
    0x61, 0x00, 0xA3, 0x12, 0xD0, 0x11, 0x70, 0x08, 0xA3, 0x0E, 0xD0, 0x11,
    0x60, 0x40, 0xF0, 0x15, 0xF0, 0x07, 0x30, 0x00, 0x12, 0x34, 0xC6, 0x0F,
    0x67, 0x1E, 0x68, 0x01, 0x69, 0xFF, 0xA3, 0x0E, 0xD6, 0x71, 0xA3, 0x10,
    0xDC, 0xD1, 0x60, 0x04, 0xE0, 0xA1, 0x7C, 0xFE, 0x60, 0x06, 0xE0, 0xA1,
    0x7C, 0x02, 0x60, 0x3F, 0x8C, 0x02, 0xDC, 0xD1, 0xA3, 0x0E, 0xD6, 0x71,
    0x86, 0x84, 0x87, 0x94, 0x60, 0x3F, 0x86, 0x02, 0x61, 0x1F, 0x87, 0x12,
    0x47, 0x1F, 0x12, 0xAC, 0x46, 0x00, 0x68, 0x01, 0x46, 0x3F, 0x68, 0xFF,
    0x47, 0x00, 0x69, 0x01, 0xD6, 0x71, 0x3F, 0x01, 0x12, 0xAA, 0x47, 0x1F,
    0x12, 0xAA, 0x60, 0x05, 0x80, 0x75, 0x3F, 0x00, 0x12, 0xAA, 0x60, 0x01,
    0xF0, 0x18, 0x80, 0x60, 0x61, 0xFC, 0x80, 0x12, 0xA3, 0x0C, 0xD0, 0x71,
    0x60, 0xFE, 0x89, 0x03, 0x22, 0xF6, 0x75, 0x01, 0x22, 0xF6, 0x45, 0x60,
    0x12, 0xDE, 0x12, 0x46, 0x69, 0xFF, 0x80, 0x60, 0x80, 0xC5, 0x3F, 0x01,
    0x12, 0xCA, 0x61, 0x02, 0x80, 0x15, 0x3F, 0x01, 0x12, 0xE0, 0x80, 0x15,
    0x3F, 0x01, 0x12, 0xEE, 0x80, 0x15, 0x3F, 0x01, 0x12, 0xE8, 0x60, 0x20,
    0xF0, 0x18, 0xA3, 0x0E, 0x7E, 0xFF, 0x80, 0xE0, 0x80, 0x04, 0x61, 0x00,
    0xD0, 0x11, 0x3E, 0x00, 0x12, 0x30, 0x12, 0xDE, 0x78, 0xFF, 0x48, 0xFE,
    0x68, 0xFF, 0x12, 0xEE, 0x78, 0x01, 0x48, 0x02, 0x68, 0x01, 0x60, 0x04,
    0xF0, 0x18, 0x69, 0xFF, 0x12, 0x70, 0xA3, 0x14, 0xF5, 0x33, 0xF2, 0x65,
    0xF1, 0x29, 0x63, 0x37, 0x64, 0x00, 0xD3, 0x45, 0x73, 0x05, 0xF2, 0x29,
    0xD3, 0x45, 0x00, 0xEE, 0xF0, 0x00, 0x80, 0x00, 0xFC, 0x00, 0xAA, 0x00,
    0x00, 0x00, 0x00, 0x00,
};
unsigned int rom_len = 280;

typedef struct {
    uint8_t memory[4096];
    uint8_t V[16];
    uint16_t pc;
    uint16_t stack[16];
    uint8_t stackLocation;
    uint16_t I;
    uint8_t screen[32][64];
    uint8_t delayTimer;
    uint8_t soundTimer;
    uint8_t keys[16];
} Chip8;

uint8_t font[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, 0x20, 0x60, 0x20, 0x20, 0x70,
    0xF0, 0x10, 0xF0, 0x80, 0xF0, 0xF0, 0x10, 0xF0, 0x10, 0xF0,
    0x90, 0x90, 0xF0, 0x10, 0x10, 0xF0, 0x80, 0xF0, 0x10, 0xF0,
    0xF0, 0x80, 0xF0, 0x90, 0xF0, 0xF0, 0x10, 0x20, 0x40, 0x40,
    0xF0, 0x90, 0xF0, 0x90, 0xF0, 0xF0, 0x90, 0xF0, 0x10, 0xF0,
    0xF0, 0x90, 0xF0, 0x90, 0x90, 0xE0, 0x90, 0xE0, 0x90, 0xE0,
    0xF0, 0x80, 0x80, 0x80, 0xF0, 0xE0, 0x90, 0x90, 0x90, 0xE0,
    0xF0, 0x80, 0xF0, 0x80, 0xF0, 0xF0, 0x80, 0xF0, 0x80, 0x80
};

ssd1306_t disp;

void drawScreen(Chip8 *chip8) {
    ssd1306_clear(&disp);
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            if (chip8->screen[y][x]) {
                int px = x * 2;
                int py = y * 2;
                ssd1306_draw_pixel(&disp, px,     py);
                ssd1306_draw_pixel(&disp, px + 1, py);
                ssd1306_draw_pixel(&disp, px,     py + 1);
                ssd1306_draw_pixel(&disp, px + 1, py + 1);
            }
        }
    }
    ssd1306_show(&disp);
}

void handleKeys(Chip8 *chip8) {
    chip8->keys[4] = !gpio_get(BTN_LEFT);  
    chip8->keys[6] = !gpio_get(BTN_RIGHT);  
}

int main() {
    stdio_init_all();

#ifdef CYW43_WL_GPIO_LED_PIN
    cyw43_arch_init();
#endif

    gpio_init(BTN_LEFT);
    gpio_set_dir(BTN_LEFT, GPIO_IN);
    gpio_pull_up(BTN_LEFT);
    gpio_init(BTN_RIGHT);
    gpio_set_dir(BTN_RIGHT, GPIO_IN);
    gpio_pull_up(BTN_RIGHT);

    i2c_init(i2c0, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    disp.external_vcc = false;
    ssd1306_init(&disp, 128, 64, 0x3C, i2c0);
    ssd1306_clear(&disp);
    ssd1306_show(&disp);

    Chip8 chip8 = {0};
    for (int i = 0; i < 80; i++) chip8.memory[0x50 + i] = font[i];
    for (unsigned int i = 0; i < rom_len; i++) chip8.memory[0x200 + i] = rom[i];
    chip8.pc = 0x200;

    int timer = 0;

    while (true) {
        handleKeys(&chip8);

        if (timer >= 10) {
            if (chip8.delayTimer > 0) chip8.delayTimer--;
            if (chip8.soundTimer > 0) chip8.soundTimer--;
            timer = 0;
        }

        uint16_t opcode = (chip8.memory[chip8.pc] << 8) | chip8.memory[chip8.pc + 1];
        chip8.pc += 2;

        uint8_t first = (opcode & 0xF000) >> 12;

        switch (first) {
            case 0x6: {
                uint8_t x = (opcode & 0x0F00) >> 8;
                uint8_t nn = opcode & 0x00FF;
                chip8.V[x] = nn;
                break;
            }
            case 0x7: {
                uint8_t x = (opcode & 0x0F00) >> 8;
                uint8_t nn = opcode & 0x00FF;
                chip8.V[x] += nn;
                break;
            }
            case 0x3: {
                uint8_t x = (opcode & 0x0F00) >> 8;
                uint8_t nn = opcode & 0x00FF;
                if (chip8.V[x] == nn) chip8.pc += 2;
                break;
            }
            case 0x4: {
                uint8_t x = (opcode & 0x0F00) >> 8;
                uint8_t nn = opcode & 0x00FF;
                if (chip8.V[x] != nn) chip8.pc += 2;
                break;
            }
            case 0x5: {
                uint8_t x = (opcode & 0x0F00) >> 8;
                uint8_t y = (opcode & 0x00F0) >> 4;
                if (chip8.V[x] == chip8.V[y]) chip8.pc += 2;
                break;
            }
            case 0x9: {
                uint8_t x = (opcode & 0x0F00) >> 8;
                uint8_t y = (opcode & 0x00F0) >> 4;
                if (chip8.V[x] != chip8.V[y]) chip8.pc += 2;
                break;
            }
            case 0x1: {
                uint16_t nnn = opcode & 0x0FFF;
                chip8.pc = nnn;
                break;
            }
            case 0x2: {
                uint16_t nnn = opcode & 0x0FFF;
                chip8.stack[chip8.stackLocation] = chip8.pc;
                chip8.stackLocation++;
                chip8.pc = nnn;
                break;
            }
            case 0xB: {
                uint16_t nnn = opcode & 0x0FFF;
                chip8.pc = nnn + chip8.V[0];
                break;
            }
            case 0xC: {
                uint8_t x = (opcode & 0x0F00) >> 8;
                uint8_t nn = opcode & 0x00FF;
                chip8.V[x] = (rand() % 256) & nn;
                break;
            }
            case 0x8: {
                uint8_t x = (opcode & 0x0F00) >> 8;
                uint8_t y = (opcode & 0x00F0) >> 4;
                uint8_t last = opcode & 0x000F;
                switch (last) {
                    case 0x0: chip8.V[x] = chip8.V[y]; break;
                    case 0x1: chip8.V[x] = chip8.V[x] | chip8.V[y]; break;
                    case 0x2: chip8.V[x] = chip8.V[x] & chip8.V[y]; break;
                    case 0x3: chip8.V[x] = chip8.V[x] ^ chip8.V[y]; break;
                    case 0x4: {
                        uint16_t sum = chip8.V[x] + chip8.V[y];
                        chip8.V[0xF] = (sum > 255) ? 1 : 0;
                        chip8.V[x] = sum & 0xFF;
                        break;
                    }
                    case 0x5: {
                        chip8.V[0xF] = (chip8.V[x] >= chip8.V[y]) ? 1 : 0;
                        chip8.V[x] = chip8.V[x] - chip8.V[y];
                        break;
                    }
                    case 0x6:
                        chip8.V[0xF] = chip8.V[x] & 0x1;
                        chip8.V[x] >>= 1;
                        break;
                    case 0x7:
                        chip8.V[0xF] = (chip8.V[y] >= chip8.V[x]) ? 1 : 0;
                        chip8.V[x] = chip8.V[y] - chip8.V[x];
                        break;
                    case 0xE:
                        chip8.V[0xF] = (chip8.V[x] >> 7) & 1;
                        chip8.V[x] <<= 1;
                        break;
                }
                break;
            }
            case 0x0: {
                if (opcode == 0x00E0) {
                    for (int y = 0; y < 32; y++)
                        for (int x = 0; x < 64; x++)
                            chip8.screen[y][x] = 0;
                } else if (opcode == 0x00EE) {
                    chip8.stackLocation--;
                    chip8.pc = chip8.stack[chip8.stackLocation];
                }
                break;
            }
            case 0xA: {
                uint16_t nnn = opcode & 0x0FFF;
                chip8.I = nnn;
                break;
            }
            case 0xD: {
                uint8_t x = (opcode & 0x0F00) >> 8;
                uint8_t y = (opcode & 0x00F0) >> 4;
                uint8_t n = opcode & 0x000F;
                uint8_t Row = chip8.V[y];
                uint8_t Column = chip8.V[x];
                chip8.V[0xF] = 0;

                for (int row = 0; row < n; row++) {
                    uint8_t pixel = chip8.memory[chip8.I + row];
                    for (int bit = 0; bit < 8; bit++) {
                        if ((pixel & (0x80 >> bit)) != 0) {
                            int screenX = (Column + bit) % 64;
                            int screenY = (Row + row) % 32;
                            if (chip8.screen[screenY][screenX] == 1)
                                chip8.V[0xF] = 1;
                            chip8.screen[screenY][screenX] ^= 1;
                        }
                    }
                }
                drawScreen(&chip8);
                break;
            }
            case 0xE: {
                uint8_t x = (opcode & 0x0F00) >> 8;
                uint8_t last = opcode & 0x00FF;
                switch (last) {
                    case 0x9E:
                        if (chip8.keys[chip8.V[x]] == 1) chip8.pc += 2;
                        break;
                    case 0xA1:
                        if (chip8.keys[chip8.V[x]] == 0) chip8.pc += 2;
                        break;
                }
                break;
            }
            case 0xF: {
                uint8_t x = (opcode & 0x0F00) >> 8;
                uint8_t last = opcode & 0x00FF;
                switch (last) {
                    case 0x07: chip8.V[x] = chip8.delayTimer; break;
                    case 0x15: chip8.delayTimer = chip8.V[x]; break;
                    case 0x18: chip8.soundTimer = chip8.V[x]; break;
                    case 0x1E: chip8.I += chip8.V[x]; break;
                    case 0x0A: {
                        int found = 0;
                        for (int i = 0; i < 16; i++) {
                            if (chip8.keys[i] == 1) {
                                chip8.V[x] = i;
                                found = 1;
                                break;
                            }
                        }
                        if (found == 0) chip8.pc -= 2;
                        break;
                    }
                    case 0x33:
                        chip8.memory[chip8.I] = chip8.V[x] / 100;
                        chip8.memory[chip8.I + 1] = (chip8.V[x] / 10) % 10;
                        chip8.memory[chip8.I + 2] = chip8.V[x] % 10;
                        break;
                    case 0x55:
                        for (int i = 0; i <= x; i++)
                            chip8.memory[chip8.I + i] = chip8.V[i];
                        break;
                    case 0x65:
                        for (int i = 0; i <= x; i++)
                            chip8.V[i] = chip8.memory[chip8.I + i];
                        break;
                    case 0x29:
                        chip8.I = 0x50 + (chip8.V[x] * 5);
                        break;
                }
                break;
            }
        }

        timer++;
        sleep_ms(2);
    }

    return 0;
}

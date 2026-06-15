#include "kernel.hpp"

/* --- Constantes de Hardware --- */
#define VGA_CTRL_PORT 0x3D4
#define VGA_DATA_PORT 0x3D5
#define VIDEO_COLOR_WHITE_ON_BLACK 0x0F
#define VIDEO_COLOR_GRAY_ON_BLACK 0x07

#ifdef __INTELLISENSE__
#define __asm__ asm
#define __volatile__
#endif

IronKernelEngine::IronKernelEngine() {
    cursor_x = 0;
    cursor_y = 0;
    clear_screen();
}

void IronKernelEngine::clear_screen() {
    unsigned short* VideoMemory = reinterpret_cast<unsigned short*>(VGA_MEMORY);

    // Preenche a tela com espaços vazios e cor padrão
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; ++i) {
        VideoMemory[i] = (VIDEO_COLOR_GRAY_ON_BLACK << 8) | ' ';
    }

    cursor_x = 0;
    cursor_y = 0;
    update_hardware_cursor();
}

void IronKernelEngine::update_hardware_cursor() {
    uint16_t pos = cursor_y * VGA_WIDTH + cursor_x;

    // Envia o comando para o controlador VGA para mover o cursor piscante
    outb(VGA_CTRL_PORT, 0x0F);
    outb(VGA_DATA_PORT, (uint8_t)(pos & 0xFF));
    outb(VGA_CTRL_PORT, 0x0E);
    outb(VGA_DATA_PORT, (uint8_t)((pos >> 8) & 0xFF));
}

void IronKernelEngine::scroll() {
    unsigned short* VideoMemory = reinterpret_cast<unsigned short*>(VGA_MEMORY);

    // Move todas as linhas uma posição para cima
    for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++) {
        VideoMemory[i] = VideoMemory[i + VGA_WIDTH];
    }

    // Limpa a última linha que ficou repetida
    for (int i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; i++) {
        VideoMemory[i] = (VIDEO_COLOR_GRAY_ON_BLACK << 8) | ' ';
    }

    cursor_y = VGA_HEIGHT - 1;
}

void IronKernelEngine::put_char(char c) {
    unsigned short* VideoMemory = reinterpret_cast<unsigned short*>(VGA_MEMORY);

    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    }
    else if (c == '\r') {
        cursor_x = 0;
    }
    else {
        int index = cursor_y * VGA_WIDTH + cursor_x;
        // Atributo 0x0F: Texto branco brilhante (F) no fundo preto (0)
        VideoMemory[index] = (VIDEO_COLOR_WHITE_ON_BLACK << 8) | (c & 0xFF);
        cursor_x++;
    }

    // Wrap-around (quebra de linha automática ao fim da largura)
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }

    // Se ultrapassar a altura, rola a tela
    if (cursor_y >= VGA_HEIGHT) {
        scroll();
    }

    update_hardware_cursor();
}

void IronKernelEngine::printf(const char* str) {
    for (int i = 0; str[i] != '\0'; ++i) {
        put_char(str[i]);
    }
}

uint8_t IronKernelEngine::read_key_scancode() {
    // Espera até que o buffer do teclado tenha um caractere pronto (bit 0 do status)
    while ((inb(KEYBOARD_STATUS_PORT) & 0x01) == 0);
    return inb(KEYBOARD_DATA_PORT);
}

void IronKernelEngine::reboot_system() {
    // Envia o comando de reset para o controlador 8042
    outb(KEYBOARD_STATUS_PORT, 0xFE);
}

inline uint8_t IronKernelEngine::inb(uint16_t port) {
    uint8_t result;
    __asm__ __volatile__("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

inline void IronKernelEngine::outb(uint16_t port, uint8_t value) {
    __asm__ __volatile__("outb %0, %1" : : "a"(value), "Nd"(port));
}

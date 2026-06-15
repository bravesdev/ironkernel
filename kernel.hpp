#pragma once

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

class TerminalRootOS {
private:
    // Constantes de Hardware
    static const int VGA_WIDTH = 80;
    static const int VGA_HEIGHT = 25;
    static const unsigned int VGA_MEMORY = 0xB8000;

    static const int KEYBOARD_DATA_PORT = 0x60;
    static const int KEYBOARD_STATUS_PORT = 0x64;

    int cursor_x, cursor_y;

    // Métodos internos de manipulação de tela
    void update_hardware_cursor(); // Atualiza o cursor piscante do BIOS
    void scroll();                 // Rola a tela quando chega no fim

    // Funções de comunicação com portas (I/O)
    inline uint8_t inb(uint16_t port);
    inline void outb(uint16_t port, uint8_t value);

public:
    TerminalRootOS();

    // Scancodes úteis
    static const int ENTER_SCANCODE = 0x1C;

    // Métodos públicos para o Kernel
    void clear_screen();
    void printf(const char* str);
    void put_char(char c);         // Movido para cá para permitir ecoar caracteres do prompt
    uint8_t read_key_scancode();
    void reboot_system();
};
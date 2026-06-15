#include "kernel.hpp"

// Manual function to compare two identical strings
bool strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 == *(unsigned char*)s2;
}

// Function to check if a command starts with a specific prefix (e.g., "echo ")
bool strstarts(const char* str, const char* prefix) {
    while (*prefix) {
        if (*str != *prefix) {
            return false;
        }
        str++;
        prefix++;
    }
    return true;
}

// Converts keyboard scancodes to ASCII characters
char scancode_to_ascii(uint8_t scancode) {
    if (scancode & 0x80) {
        return 0; // Ignore key release events
    }

    static const char kbd_layout[] = {
        0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
      '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0,  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 
        0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0,
      '*',   0, ' '
    };

    if (scancode < sizeof(kbd_layout)) {
        return kbd_layout[scancode];
    }
    return 0;
}

extern "C" void kmain(void* multiboot_structure, unsigned int magicnumber) {

    TerminalRootOS terminal;

 // Initial Splash Header (Optimized for 80-column VGA Text Mode)
    terminal.printf("  __________________________________________________________________________\n");
    terminal.printf("           ___                     _  __                         _ \n");
    terminal.printf("          |_ _| _ __  ___   _ __  | |/ / ___  _ __  _ __    ___ | |\n");
    terminal.printf("           | | | '__|/ _ \\ | '_ \\ | ' / / _ \\| '__|| '_ \\  / _ \\| |\n");
    terminal.printf("           | | | |  | (_) || | | || . \\|  __/| |   | | | ||  __/| |\n");
    terminal.printf("          |___||_|   \\___/ |_| |_||_|\\_\\\\___||_|   |_| |_| \\___||_|\n");
    terminal.printf("  __________________________________________________________________________\n\n");
    terminal.printf("                  Version: 0.1-ab (Instruction Mode 32-bit)\n");
    terminal.printf("                            Code for BravesDev\n");
    terminal.printf("                     Open Source GNU GPL v3.0 License\n");
    terminal.printf("  __________________________________________________________________________\n\n");
    terminal.printf("   Type 'help' to display the list of available commands.\n\n");

    char cmd_buffer[64];
    int cmd_idx = 0;

    terminal.printf("root@machine#> ");

    while (1) {
        uint8_t scancode = terminal.read_key_scancode();
        char c = scancode_to_ascii(scancode);

        if (c == 0) continue;

        if (c == '\n') {
            cmd_buffer[cmd_idx] = '\0'; 
            terminal.printf("\n");      

            // 1. HELP Command
            if (strcmp(cmd_buffer, "help")) {
                terminal.printf("Available commands:\n");
                terminal.printf("  help - Show this list\n");
                terminal.printf("  about - System information\n");
                terminal.printf("  echo <text> - Repeats the typed message\n");
                terminal.printf("  clear - Clears the terminal\n");
                terminal.printf("  reboot - Restarts the computer\n");
            } 
            // 2. ABOUT Command
            else if (strcmp(cmd_buffer, "about")) {
                terminal.printf("=========================================\n");
                terminal.printf(" Iron Kernel - Version 0.0.2 (Protected Mode)\n");
                terminal.printf(" Architecture: x86 (i386 ELF)\n");
                terminal.printf(" Driver Video: VGA Text Mode 80x25\n");
                terminal.printf("=========================================\n");
            }
            // 3. ECHO Command (Uses the strstarts function)
            else if (strstarts(cmd_buffer, "echo ")) {
                // Advance 5 characters to skip "echo " and print only the argument
                terminal.printf(cmd_buffer + 5);
                terminal.printf("\n");
            }
            // 4. CLEAR Command
            else if (strcmp(cmd_buffer, "clear")) {
                terminal.clear_screen();
            } 
            // 5. REBOOT Command
            else if (strcmp(cmd_buffer, "reboot")) {
                terminal.printf("Restarting the system...\n");
                terminal.reboot_system();
            } 
            // Invalid/Unknown Command
            else if (cmd_idx > 0) {
                terminal.printf("Unknown command: ");
                terminal.printf(cmd_buffer);
                terminal.printf("\n");
            }

            cmd_idx = 0;
            terminal.printf("root@machine# ");
        } 
        else if (c == '\b') {
            if (cmd_idx > 0) {
                cmd_idx--;              
                terminal.put_char('\b'); 
            }
        } 
        else {
            if (cmd_idx < 63) { 
                cmd_buffer[cmd_idx++] = c;
                terminal.put_char(c);
            }
        }
    }
}
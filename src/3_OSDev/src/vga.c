#include <vga.h>
#include <libc/stdarg.h>

uint16_t coloumn = 0;
uint16_t line = 0;
uint16_t* const vga = (uint16_t* const) 0xB8000;
const uint16_t default_colour = (COLOUR_WHITE << 8) | (COLOUR_BLACK << 12);
uint16_t current_colour = default_colour;

void Reset() {
    line = 0;
    coloumn = 0;
    current_colour = default_colour;

    for (uint16_t y = 0; y < height; y++) {
        for (uint16_t x = 0; x < width; x++) {
            vga[y * width + x] = ' ' | default_colour;
        }
    }
}

void newLine() {
    if (line < height -1) {
        line++;
        coloumn = 0;
    } else {
        scrollup();
        coloumn = 0;
    }
}

void scrollup() {
    // Copy each line to the line above it (starting from the second line)
    for (uint16_t y = 1; y < height; y++) {
        for (uint16_t x = 0; x < width; x++) {
            vga[(y-1) * width + x] = vga[y * width + x];
        }  
    }
    // Clear the last line
    for (uint16_t x = 0; x < width; x++) {
        vga[(height-1) * width + x] = ' ' | current_colour;
    }
}

void print(int colour, const char* s, ...) {
    va_list args;
    va_start(args, s);
    
    if (colour != 0) {
        current_colour = (colour << 8) | (COLOUR_BLACK << 12);
    } else {
        current_colour = default_colour;
    }
    
    while (*s) {
        if (*s == '%' && *(s + 1) != '\0') {
            s++;
            switch(*s) {
                case 'd': {
                    int val = va_arg(args, int);
                    char buffer[12]; // Enough for 32-bit integers with sign
                    int i = 0;
                    int is_negative = 0;
                    
                    // Handle negative numbers
                    if (val < 0) {
                        is_negative = 1;
                        val = -val;
                    }
                    
                    // Convert integer to string (reversed)
                    do {
                        buffer[i++] = '0' + (val % 10);
                        val /= 10;
                    } while (val > 0);
                    
                    // Add negative sign if needed
                    if (is_negative) {
                        buffer[i++] = '-';
                    }
                    
                    // Print in correct order
                    while (i > 0) {
                        i--;
                        if (coloumn == width) {
                            newLine();
                        }
                        vga[line * width + (coloumn++)] = buffer[i] | current_colour;
                    }
                    break;
                }
                case 'u': {
                    unsigned int val = va_arg(args, unsigned int);
                    char buffer[12]; // Enough for 32-bit unsigned integers
                    int i = 0;
                    
                    // Handle special case for 0
                    if (val == 0) {
                        buffer[i++] = '0';
                    }
                    
                    // Convert integer to string (reversed)
                    while (val > 0) {
                        buffer[i++] = '0' + (val % 10);
                        val /= 10;
                    }
                    
                    // Print in correct order
                    while (i > 0) {
                        i--;
                        if (coloumn == width) {
                            newLine();
                        }
                        vga[line * width + (coloumn++)] = buffer[i] | current_colour;
                    }
                    break;
                }
                case 'x': {
                    unsigned int val = va_arg(args, unsigned int);
                    char buffer[12]; // Enough for 32-bit integers in hex
                    int i = 0;
                    const char hex_chars[] = "0123456789abcdef";
                    
                    // Handle special case for 0
                    if (val == 0) {
                        buffer[i++] = '0';
                    }
                    
                    // Convert integer to hex string (reversed)
                    while (val > 0) {
                        buffer[i++] = hex_chars[val % 16];
                        val /= 16;
                    }
                    
                    // Print in correct order
                    while (i > 0) {
                        i--;
                        if (coloumn == width) {
                            newLine();
                        }
                        vga[line * width + (coloumn++)] = buffer[i] | current_colour;
                    }
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int); // char is promoted to int in varargs
                    if (coloumn == width) {
                        newLine();
                    }
                    vga[line * width + (coloumn++)] = c | current_colour;
                    break;
                }
                case 's': {
                    const char* str = va_arg(args, const char*);
                    while (*str) {
                        if (coloumn == width) {
                            newLine();
                        }
                        vga[line * width + (coloumn++)] = *str++ | current_colour;
                    }
                    break;
                }
                case '%':
                    if (coloumn == width) {
                        newLine();
                    }
                    vga[line * width + (coloumn++)] = '%' | current_colour;
                    break;
                default:
                    if (coloumn == width) {
                        newLine();
                    }
                    vga[line * width + (coloumn++)] = '%' | current_colour;
                    if (coloumn == width) {
                        newLine();
                    }
                    vga[line * width + (coloumn++)] = *s | current_colour;
                    break;
            }
        } else {
            switch(*s) {
                case '\n':
                    newLine();
                    break;
                case '\r':
                    coloumn = 0;
                    break;
                case '\t':
                    if (coloumn == width) {
                        newLine();
                    }
                    uint16_t tabLen = 4 - (coloumn % 4);
                    while (tabLen != 0) {
                        vga[line * width + (coloumn++)] = ' ' | current_colour;
                        tabLen--;
                    }
                    break;
                case '\b':
                    if (coloumn > 0) {
                        coloumn--;
                        vga[line * width + coloumn] = ' ' | current_colour;
                    } else if (line > 0) {
                        line--;
                        coloumn = width - 1;
                        vga[line * width + coloumn] = ' ' | current_colour;
                    }
                    break;
                default:
                    if (coloumn == width) {
                        newLine();
                    }
                    vga[line * width + (coloumn++)] = *s | current_colour;
                    break;
            }
        }
        s++;
    }
    va_end(args);
}


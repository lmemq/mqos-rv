#include "serial.h"
#include <stdarg.h> 

void put_char(uint8_t ch) {
    *((volatile uint64_t*)SERIAL_MMIO) = ch;
}

void kprint(uint8_t *print_string) {
    uint64_t i = 0;
    while(1) {
        if (print_string[i] == 0) {
            break;
        }
        put_char(print_string[i]);
        i++;
    }
}
 
void reverse(uint8_t str[], int length) {
    int start = 0;
    int end = length - 1;
    while (start < end) {
        uint8_t temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

void kprint_ui(uint64_t inp) {
    uint8_t str[24];
    uitoa(str, inp, 10);
    kprint(str);
}

uint8_t* uitoa(uint8_t *str, uint64_t num, int base) {
    int i = 0;
 
    /* Handle 0 explicitly, otherwise empty string is printed for 0 */
    if (num == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }
 
    // Process individual digits
    while (num != 0)
    {
        int rem = num % base;
        str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
        num = num/base;
    }
 
    str[i] = '\0';
 
    reverse(str, i);
 
    return str;
}

void kprintf(const char *format, ...) {
    va_list args;
    va_start(args, format);

    uint64_t i = 0;
    uint8_t buffer[24];

    while (format[i] != '\0') {
        if (format[i] != '%') {
            put_char(format[i]);
            i++;
            continue;
        }
        i++; 
        switch (format[i]) {
            case 'c': {
                char c = (char)va_arg(args, int);
                put_char(c);
                break;
            }
            case 's': {
                uint8_t *s = va_arg(args, uint8_t*);
                if (s == 0) s = (uint8_t*)"(null)";
                kprint(s);
                break;
            }
            case 'd':
            case 'u': {
                uint64_t num = va_arg(args, uint64_t);
                uitoa(buffer, num, 10);
                kprint(buffer);
                break;
            }
            case 'p':
            case 'x': {
                uint64_t num = va_arg(args, uint64_t);
                kprint((uint8_t*)"0x"); 
                uitoa(buffer, num, 16);
                kprint(buffer);
                break;
            }
            case '%': { 
                put_char('%');
                break;
            }
            default: { 
                put_char('%');
                put_char(format[i]);
                break;
            }
        }
        i++;
    }

    va_end(args);
}

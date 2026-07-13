#include "serial.h"

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


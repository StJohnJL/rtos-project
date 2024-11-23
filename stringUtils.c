
#include <stdint.h>
#include <string.h>

void itoa(uint32_t num, char numString[]) {
    char temp;
    uint8_t i = 0;
    uint8_t j = 0;

    do {
        numString[i++] = num%10 + '0';
    } while((num /= 10) > 0);

    numString[i] = '\0';

    for(i = 0, j = strlen(numString)-1; i < j; i++, j--) {
        temp = numString[i];
        numString[i] = numString[j];
        numString[j] = temp;
    }
}

void itoh(uint32_t num, char numString[]) {
    char temp;
    uint8_t i = 0;
    uint8_t j = 0;

    do {
        if(num%16 < 10) {
            numString[i++] = num%16 + '0';
        }
        else {
            numString[i++] = num%16 + '7';
        }
    } while((num /= 16) > 0);

    numString[i++] = 'x';
    numString[i++] = '0';

    numString[i] = '\0';

    for(i = 0, j = strlen(numString)-1; i < j; i++, j--) {
        temp = numString[i];
        numString[i] = numString[j];
        numString[j] = temp;
    }
}

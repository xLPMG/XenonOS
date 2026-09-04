#include "string.h"

int strcmp(const char *a, const char *b)
{
    while (*a && *a == *b)
    {
        a++;
        b++;
    }

    return (unsigned char)*a - (unsigned char)*b;
}

size_t strlen(const char *str)
{
    size_t length = 0;

    while (str[length])
        length++;

    return length;
}

void *memcpy(void *dest, const void *src, size_t n)
{
    void *d = dest;
    size_t words = n / 4;
    size_t remainder = n % 4;

    // cld ensures the direction flag is cleared so that string operations increment the pointers
    // rep movsl = copy 4 bytes at a time
    // rep movsb = copy the remaining 0-3 bytes
    __asm__ volatile("cld\n\t"
                     "rep movsl"
                     : "+D"(d), "+S"(src), "+c"(words)
                     :
                     : "memory");
    __asm__ volatile("cld\n\t"
                     "rep movsb"
                     : "+D"(d), "+S"(src), "+c"(remainder)
                     :
                     : "memory");

    return dest;
}

void *memset(void *dest, int value, size_t n)
{
    void *d = dest;
    unsigned char byte = (unsigned char)value;
    uint32_t word = byte * 0x01010101u;
    size_t words = n / 4;
    size_t remainder = n % 4;

    // cld ensures the direction flag is cleared so that string operations increment the pointers
    // rep stosl = fill 4 bytes at a time
    // rep stosb = fill the remaining 0-3 bytes
    __asm__ volatile("cld\n\t"
                     "rep stosl"
                     : "+D"(d), "+c"(words)
                     : "a"(word)
                     : "memory");
    __asm__ volatile("cld\n\t"
                     "rep stosb"
                     : "+D"(d), "+c"(remainder)
                     : "a"(byte)
                     : "memory");

    return dest;
}

int memcmp(const void *a, const void *b, size_t n)
{
    if (n == 0)
        return 0;

    const unsigned char *x = a;
    const unsigned char *y = b;
    size_t count = n;

    // repe cmpsb stops as soon as a mismatch is found (or n bytes match);
    // either way x/y end up one past the last byte compared.
    __asm__ volatile("cld\n\t"
                     "repe cmpsb"
                     : "+D"(x), "+S"(y), "+c"(count)
                     :
                     : "memory", "cc");

    return x[-1] - y[-1];
}

void itoa(unsigned int value, char *buffer)
{
    char temp[16];
    int i = 0;
    int j = 0;

    if (value == 0)
    {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    while (value > 0)
    {
        temp[i++] = '0' + (value % 10);
        value /= 10;
    }

    while (i > 0)
        buffer[j++] = temp[--i];

    buffer[j] = '\0';
}

void itoa_hex(unsigned int value, char *buffer)
{
    static const char digits[] = "0123456789abcdef";
    char temp[8];
    int i = 0;
    int j = 0;

    buffer[j++] = '0';
    buffer[j++] = 'x';

    if (value == 0)
    {
        buffer[j++] = '0';
        buffer[j] = '\0';
        return;
    }

    while (value > 0)
    {
        temp[i++] = digits[value % 16];
        value /= 16;
    }

    while (i > 0)
        buffer[j++] = temp[--i];

    buffer[j] = '\0';
}
#include "cpu.h"

static void cpuid(uint32_t leaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx,
                  uint32_t *edx)
{
    __asm__ volatile("cpuid"
                     : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                     : "a"(leaf));
}

void cpu_get_vendor(char *vendor)
{
    uint32_t eax, ebx, ecx, edx;

    cpuid(0, &eax, &ebx, &ecx, &edx);

    // CPUID vendor string is EBX ECX EDX
    vendor[0] = ebx & 0xFF;
    vendor[1] = (ebx >> 8) & 0xFF;
    vendor[2] = (ebx >> 16) & 0xFF;
    vendor[3] = (ebx >> 24) & 0xFF;

    vendor[4] = ecx & 0xFF;
    vendor[5] = (ecx >> 8) & 0xFF;
    vendor[6] = (ecx >> 16) & 0xFF;
    vendor[7] = (ecx >> 24) & 0xFF;

    vendor[8] = edx & 0xFF;
    vendor[9] = (edx >> 8) & 0xFF;
    vendor[10] = (edx >> 16) & 0xFF;
    vendor[11] = (edx >> 24) & 0xFF;

    vendor[12] = '\0';
}

void cpu_get_version(unsigned int *family, unsigned int *model)
{
    uint32_t eax, ebx, ecx, edx;

    cpuid(1, &eax, &ebx, &ecx, &edx);

    unsigned int base_family = (eax >> 8) & 0xF;
    unsigned int base_model = (eax >> 4) & 0xF;

    unsigned int extended_family = (eax >> 20) & 0xFF;
    unsigned int extended_model = (eax >> 16) & 0xF;

    if (base_family == 0xF)
        *family = base_family + extended_family;
    else
        *family = base_family;

    if (base_family == 0x6 || base_family == 0xF)
        *model = base_model | (extended_model << 4);
    else
        *model = base_model;
}
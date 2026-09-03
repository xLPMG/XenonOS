#ifndef CPU_H
#define CPU_H

#include "types.h"

void cpu_get_vendor(char *vendor);
void cpu_get_version(uint32_t *family, uint32_t *model);

#endif // CPU_H
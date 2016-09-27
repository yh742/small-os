#ifndef COMMON_H_
#define COMMON_H_

#define UNUSED(x) (void)(x)
#include "constants.h"

#define PHYSICAL_TO_VIRTUAL(addr) ((addr) + KERNEL_START_VADDR)

#endif

#ifndef TYPES_H
#define TYPES_H

#include <stdexcept>

typedef enum {
    LO = 1,
    HI = 2,
} Criticality;

static inline Criticality int2crit(int value) {
    if (1 == value) return LO;
    if (2 == value) return HI;

    throw std::runtime_error("Unknown given value for criticality");
}

#endif /* TYPES_H */

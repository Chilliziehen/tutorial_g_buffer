/*
 * filename：IdentificationGenerator
 * arthur：Chilliziehen
 * time created：2026/1/14
 * description：
 * Identification Generator implementation.
 */


#include "IdentificationGenerator.h"

namespace se {
    IdentificationGenerator::IdentificationGenerator() {
        this->currentID = 0;
    }
    IdentificationGenerator::~IdentificationGenerator() = default;
    uint32_t IdentificationGenerator::generateID() {
        return currentID++;
    }
} // se
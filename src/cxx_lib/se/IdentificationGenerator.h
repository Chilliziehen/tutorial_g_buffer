/*
 * filename：IdentificationGenerator
 * arthur：Chilliziehen
 * time created：2026/1/14
 * description：
 * A Meyers-Singleton Class for ID generation.
 * Could be extended.
 * Thread safe.
 */
#ifndef IMMEDIATERENDERERVK_IDENTIFICATIONGENERATOR_H
#define IMMEDIATERENDERERVK_IDENTIFICATIONGENERATOR_H
#include "Singleton.h"
#include <glm.hpp>
namespace se {
    /**
     * @author Chilliziehen scream303677483@gmail.com
     * @brief A Meyers-Singleton Class for ID generation.
     * @note Could be extended. Thread safe
     * @attention Every call to generateID() will return a unique uint32_t ID.
     * Never duplicate.
     */
    class IdentificationGenerator:public Singleton<IdentificationGenerator> {
        friend class Singleton<IdentificationGenerator>;
    private:
        uint32_t currentID;
        IdentificationGenerator();
        ~IdentificationGenerator();
    public:
        uint32_t generateID();
    };
} // se

#endif //IMMEDIATERENDERERVK_IDENTIFICATIONGENERATOR_H
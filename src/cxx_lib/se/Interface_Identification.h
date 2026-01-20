/*
 * filename：Interface_Identificated
 * arthur：Chilliziehen
 * time created：2026/1/14
 * description：
 * A template for identification objects.
 */
#ifndef IMMEDIATERENDERERVK_INTERFACE_IDENTIFICATED_H
#define IMMEDIATERENDERERVK_INTERFACE_IDENTIFICATED_H

#include "IdentificationGenerator.h"
namespace se {
    /**
     * @author Chilliziehen scream303677483@gmail.com
     * @brief A template for identification objects.
     * @tparam T The type of the identification object.
     * @note Inherit from this class to give your class a unique ID.
     * Usage:
     * MyClass : public Identification<MyClass> {
     *  friend class Identification<MyClass>;
     * public:
     *  MyClass();
     *  ~MyClass();
     *  void someMethod();
     * };
     */
    template<class T>
    class Identification {
    private:
        uint32_t id;
    public:
        explicit Identification() {
            this->id = IdentificationGenerator::getInstance().generateID();
        }
        ~Identification() = default;
        [[nodiscard]] uint32_t getID() const {
            return id;
        }
    };
}
#endif //IMMEDIATERENDERERVK_INTERFACE_IDENTIFICATED_H
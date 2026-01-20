/*
 * filename：Interface_ExternalResourcePoolAccessor
 * arthur：Chilliziehen
 * time created：2026/1/15
 * description：
 * A resource pool accessor to access external resource pools.
 * Designed to abstract access to resource pools inside other modules.
 */

#ifndef IMMEDIATERENDERERVK_INTERFACE_EXTERNALRESOURCEPOOLACCESSOR_H
#define IMMEDIATERENDERERVK_INTERFACE_EXTERNALRESOURCEPOOLACCESSOR_H
#include <vector>

namespace resources {
    /**
     * @tparam T The type of the resource. e.g. Vertex.
     * @author Chilliziehen scream303677483@gmail.com
     * @brief A resource pool accessor to access external resource pools.
     * @note Designed to abstract access to resource pools inside other modules.
     */
    template<class T>
    class Interface_ExternalResourcePoolAccessor {
    public:
        Interface_ExternalResourcePoolAccessor() = default;
        virtual ~Interface_ExternalResourcePoolAccessor() = default;
        [[nodiscard]] virtual bool hasResourcePool()=0;
        [[nodiscard]]virtual T* getResourcePoolPtr() = 0;
        [[nodiscard]] virtual size_t getResourceCount() const = 0;
        [[nodiscard]] virtual bool bindExternalResourcePool(T* externalPoolPtr, size_t resourceCount)=0;
        [[nodiscard]] virtual T* getResourceAtOffset(size_t offset)=0;
        [[nodiscard]] virtual size_t getResourceSize() const=0;
        [[nodiscard]] virtual T* copyBoundResourcePool() const=0;
        [[nodiscard]] virtual bool bindExternalResourcePool(std::vector<T>& externalPool)=0;
        [[nodiscard]] virtual std::vector<T>& getBoundResourcePoolVector()=0;
    };
}

#endif //IMMEDIATERENDERERVK_INTERFACE_EXTERNALRESOURCEPOOLACCESSOR_H
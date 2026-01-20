/*
 * filename：IndexPoolAccessor
 * arthur：Chilliziehen
 * time created：2026/1/15
 * description：
 * Data structure for accessing index pools.
 * Could be extended.
 */

#ifndef IMMEDIATERENDERERVK_INDEXPOOLACCESSOR_H
#define IMMEDIATERENDERERVK_INDEXPOOLACCESSOR_H

#include <cstdint>
#include <vector>

#include "Interface_ExternalResourcePoolAccessor.h"

namespace resources {
    /**
     * @author Chilliziehen scream303677483@gmail.com
     * @brief Data structure for accessing index pools.
     * @note Could be extended.
     * @attention index pool is a reference to external index pool.
     */
    class IndexPoolAccessor : public Interface_ExternalResourcePoolAccessor<uint32_t> {
    private:
        std::vector<uint32_t>& indices;
        uint32_t* indicesDataPtr = nullptr;
        size_t indexCount = -1;
        bool isBound = false;
        bool boundByVector = false;

    public:
        IndexPoolAccessor();
        ~IndexPoolAccessor() override;

        [[nodiscard]] bool hasResourcePool() override;
        [[nodiscard]] uint32_t* getResourcePoolPtr() override;
        [[nodiscard]] size_t getResourceCount() const override;
        [[nodiscard]] bool bindExternalResourcePool(uint32_t* externalPoolPtr, size_t resourceCount) override;
        [[nodiscard]] uint32_t* getResourceAtOffset(size_t offset) override;
        [[nodiscard]] size_t getResourceSize() const override;
        [[nodiscard]] uint32_t* copyBoundResourcePool() const override;
        [[nodiscard]] bool bindExternalResourcePool(std::vector<uint32_t>& externalPool) override;
        [[nodiscard]] std::vector<uint32_t>& getBoundResourcePoolVector() override;
    };
} // resources

#endif //IMMEDIATERENDERERVK_INDEXPOOLACCESSOR_H


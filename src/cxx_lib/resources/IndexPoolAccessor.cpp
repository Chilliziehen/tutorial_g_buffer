/*
 * filename：IndexPoolAccessor
 * arthur：Chilliziehen
 * time created：2026/1/15
 * description：
 * Implementation for IndexPoolAccessor.
 */

#include "IndexPoolAccessor.h"
#include <cstring>
#include <stdexcept>

namespace resources {
    IndexPoolAccessor::IndexPoolAccessor() : indices(*(reinterpret_cast<std::vector<uint32_t> *>(this))) {
        this->isBound = false;
        this->indexCount = -1;
        this->indicesDataPtr = nullptr;
    }

    IndexPoolAccessor::~IndexPoolAccessor() {
        this->isBound = false;
    }

    bool IndexPoolAccessor::bindExternalResourcePool(uint32_t *externalPoolPtr, size_t resourceCount) {
        if (externalPoolPtr == nullptr)
            throw std::runtime_error("IndexPoolAccessor::bindExternalResourcePool: externalPoolPtr is null");
        if (resourceCount == 0)
            throw std::runtime_error("IndexPoolAccessor::bindExternalResourcePool: resourceCount is zero");
        this->indexCount = resourceCount;
        this->indicesDataPtr = externalPoolPtr;
        this->isBound = true;
        this->boundByVector = false;
        return true;
    }

    bool IndexPoolAccessor::bindExternalResourcePool(std::vector<uint32_t> &externalPool) {
        this->isBound = true;
        this->boundByVector = true;
        this->indices = externalPool;
        this->indexCount = externalPool.size();
        this->indicesDataPtr = externalPool.data();
        return true;
    }

    bool IndexPoolAccessor::hasResourcePool() {
        return this->isBound;
    }

    uint32_t *IndexPoolAccessor::getResourcePoolPtr() {
        return this->indicesDataPtr;
    }

    size_t IndexPoolAccessor::getResourceCount() const {
        return this->indexCount;
    }

    uint32_t *IndexPoolAccessor::getResourceAtOffset(const size_t offset) {
        return this->indicesDataPtr + offset;
    }

    size_t IndexPoolAccessor::getResourceSize() const {
        return sizeof(uint32_t) * this->indexCount;
    }

    uint32_t *IndexPoolAccessor::copyBoundResourcePool() const {
        auto *newPool = new uint32_t[this->indexCount];
        std::memcpy(newPool, this->indicesDataPtr, sizeof(uint32_t) * this->indexCount);
        return newPool;
    }

    std::vector<uint32_t> &IndexPoolAccessor::getBoundResourcePoolVector() {
        if (!this->boundByVector)
            throw std::runtime_error("IndexPoolAccessor::getBoundResourcePoolVector: Not bound by vector.");
        return this->indices;
    }
} // resources
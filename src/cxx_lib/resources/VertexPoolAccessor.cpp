/*
 * filename：VertexPoolAccessor
 * arthur：Chilliziehen
 * time created：2026/1/15
 * description：
 * Implementation for VertexPoolAccessor.
 */


#include "VertexPoolAccessor.h"

#include <cstring>
#include <stdexcept>

namespace resources {
    VertexPoolAccessor::VertexPoolAccessor() : vertices(*(reinterpret_cast<std::vector<scene::Vertex> *>(this))) {
        this->isBound = false;
        this->vertexCount = -1;
        this->verticesDataPtr = nullptr;
    }
    VertexPoolAccessor::~VertexPoolAccessor() {
        this->isBound = false;
    }
    bool VertexPoolAccessor::bindExternalResourcePool(scene::Vertex *externalPoolPtr, size_t resourceCount) {
        if (externalPoolPtr==nullptr)
            throw std::runtime_error("VertexPoolAccessor::bindExternalResourcePool: externalPoolPtr is null");
        if (resourceCount==0)
            throw std::runtime_error("VertexPoolAccessor::bindExternalResourcePool: resourceCount is zero");
        this->vertexCount = resourceCount;
        this->verticesDataPtr = externalPoolPtr;
        this->isBound = true;
        this->boundByVector = false;
        return true;
    }
    bool VertexPoolAccessor::bindExternalResourcePool(std::vector<scene::Vertex> &externalPool) {
        this->isBound = true;
        this->boundByVector = true;
        this->vertices = externalPool;
        this->vertexCount = externalPool.size();
        this->verticesDataPtr = externalPool.data();
        return true;
    }
    bool VertexPoolAccessor::hasResourcePool() {
        return this->isBound;
    }
    scene::Vertex* VertexPoolAccessor::getResourcePoolPtr() {
        return this->verticesDataPtr;
    }
    size_t VertexPoolAccessor::getResourceCount() const{
        return this->vertexCount;
    }
    scene::Vertex *VertexPoolAccessor::getResourceAtOffset(const size_t offset) {
        return this->verticesDataPtr + offset;
    }
    size_t VertexPoolAccessor::getResourceSize() const {
        return sizeof(scene::Vertex)*this->vertexCount;
    }
    scene::Vertex *VertexPoolAccessor::copyBoundResourcePool() const {
        scene::Vertex* newPool = new scene::Vertex[this->vertexCount];
        std::memcpy(newPool, this->verticesDataPtr, sizeof(scene::Vertex)*this->vertexCount);
        return newPool;
    }
    std::vector<scene::Vertex> &VertexPoolAccessor::getBoundResourcePoolVector() {
        if (!this->boundByVector)
            throw std::runtime_error("VertexPoolAccessor::getBoundResourcePoolVector: Not bound by vector.");
        return this->vertices;
    }
} // resources
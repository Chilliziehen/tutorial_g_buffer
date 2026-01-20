/*
 * filename：VertexPoolAccessor
 * arthur：Chilliziehen
 * time created：2026/1/15
 * description：
 * Data structure for accessing vertex pools.
 * Could be extended.
 */

#ifndef IMMEDIATERENDERERVK_VERTEXPOOLACCESSOR_H
#define IMMEDIATERENDERERVK_VERTEXPOOLACCESSOR_H
#include <optional>

#include "Interface_ExternalResourcePoolAccessor.h"
#include "../scene/data_structures/Vertex.h"

namespace resources {
    /**
     * @author Chilliziehen scream303677483@gmail.com
     * @brief Data structure for accessing vertex pools.
     * @note Could be extended.
     * @attention vertex pool is a reference to external vertex pool.
     */
    class VertexPoolAccessor : public Interface_ExternalResourcePoolAccessor<scene::Vertex>{
    private:
        std::vector<scene::Vertex>& vertices;
        scene::Vertex* verticesDataPtr = nullptr;
        size_t vertexCount = -1;
        bool isBound = false;
        bool boundByVector = false;
    public:
        VertexPoolAccessor();
        ~VertexPoolAccessor() override;
        [[nodiscard]] bool hasResourcePool() override;
        [[nodiscard]]scene::Vertex* getResourcePoolPtr() override;
        [[nodiscard]] size_t getResourceCount() const override;
        [[nodiscard]] bool bindExternalResourcePool(scene::Vertex* externalPoolPtr, size_t resourceCount) override;
        [[nodiscard]] scene::Vertex* getResourceAtOffset(size_t offset) override;
        [[nodiscard]] size_t getResourceSize() const override;
        [[nodiscard]] scene::Vertex* copyBoundResourcePool() const override;
        [[nodiscard]] bool bindExternalResourcePool(std::vector<scene::Vertex>& externalPool) override;
        [[nodiscard]] std::vector<scene::Vertex>& getBoundResourcePoolVector() override;
    };
} // resources

#endif //IMMEDIATERENDERERVK_VERTEXPOOLACCESSOR_H
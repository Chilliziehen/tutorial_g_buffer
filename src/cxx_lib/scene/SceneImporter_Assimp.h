/*
 * filename：SceneImporter_Assimp
 * arthur：Chilliziehen
 * time created：2026/1/15
 * description：
 * Scene importer implementation using Assimp library.
 */


#ifndef IMMEDIATERENDERERVK_SCENEIMPORTER_ASSIMP_H
#define IMMEDIATERENDERERVK_SCENEIMPORTER_ASSIMP_H
#include <optional>

#include "Interface_SceneImporter.h"
#include "data_structures/RenderScene.h"

namespace scene {
    /**
     * @author Chilliziehen scream303677483@gmail.com
     * @brief Scene importer implementation using Assimp library.
     * @attention getImportedScene() should be called after loadScene().
     * Any invalid operation will lead to exception throw.
     */
    class SceneImporter_Assimp :public Interface_SceneImporter{
    private:
        std::optional<RenderScene> importedScene;
    public:
        SceneImporter_Assimp() = default;
        ~SceneImporter_Assimp() override;

        bool loadScene(const std::string& filePath) override;   ///@brief Load scene from file using Assimp.
        [[nodiscard]] RenderScene& getImportedScene() override; ///@brief Get the imported scene. Should be called after loadScene().
    };
} // scene

#endif //IMMEDIATERENDERERVK_SCENEIMPORTER_ASSIMP_H
/*
 * filename：Interface_SceneImporter
 * arthur：Chilliziehen
 * time created：2026/1/14
 * description：
 * ADAPTOR
 * Implement should be SINGLETON.(using se::Singleton)
 * Scene importing base class.
 * Could be implemented.
 */

#ifndef IMMEDIATERENDERERVK_INTERFACE_SCENEIMPORTER_H
#define IMMEDIATERENDERERVK_INTERFACE_SCENEIMPORTER_H
#include <string>

#include "data_structures/RenderScene.h"
#include "../se/Singleton.h"
namespace scene {
    /**
     * @author Chilliziehen scream303677483@gmail.com
     * @attention Implement should be SINGLETON.(using se::Singleton)
     * @note ADAPTOR
     * @brief Scene importing base class.
     */
    class Interface_SceneImporter {
    private:
    public:
        virtual ~Interface_SceneImporter() = default;
        /**
         * @author Chilliziehen scream303677483@gmail.com
         * @brief Load a scene from a file.
         * @param path The path to the scene file.
         * @return true if the scene was loaded successfully, false otherwise.
         */
        virtual bool loadScene(const std::string& path) = 0;
        virtual RenderScene& getImportedScene() = 0;
    };
} // scene

#endif //IMMEDIATERENDERERVK_SCENEIMPORTER_H
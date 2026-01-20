/*
 * filename：Material
 * arthur：Chilliziehen
 * time created：2026/1/14
 * description：
 * Data structure for Material.
 * Currently supporting [glTF 2.0 PBR Metallic-Roughness] model.
 * Could be extended.
 */
#ifndef IMMEDIATERENDERERVK_MATERIAL_H
#define IMMEDIATERENDERERVK_MATERIAL_H
#include <string>
#include <glm.hpp>
namespace scene {
    /**
     * @author Chilliziehen scream303677483@gmail.com
     * @brief Data structure for PBR Material.
     * @attention Currently supporting [glTF 2.0 PBR Metallic-Roughness] model.
     * @note Could be extended.
     */
    struct Material {
        std::string name;
        // PBR Factors
        glm::vec4 baseColorFactor = glm::vec4(1.0f);
        float metallicFactor = 1.0f;
        float roughnessFactor = 1.0f;

        // Texture map indices (-1 means no texture)
        int albedoMapIndex = -1;
        int normalMapIndex = -1;
        int metallicRoughnessMapIndex = -1; ///@attention glTF uses a combined metallic-roughness map
        int emissiveMapIndex = -1;
        int aoMapIndex = -1;

        // Render status flag.
        bool doubleSided = false;
        std::string alphaMode = "OPAQUE"; // OPAQUE, MASK, BLEND
    };
}
#endif //IMMEDIATERENDERERVK_MATERIAL_H
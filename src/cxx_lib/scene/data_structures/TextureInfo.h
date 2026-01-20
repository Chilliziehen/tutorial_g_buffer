/*
 * filename：TextureInfo
 * arthur：Chilliziehen
 * time created：2026/1/14
 * description：
 * Data structure for texture information.
 * Could be extended.(I have no idea it should have been extended lol.)
 * Texture should be loaded to GPU, so I won't put image data here or in this directory.
 */
#ifndef IMMEDIATERENDERERVK_TEXTUREINFO_H
#define IMMEDIATERENDERERVK_TEXTUREINFO_H
#include <string>
#include "Interface_Identification.h"

namespace scene {
    /**
     * @author Chilliziehen scream303677483@gmail.com
     * @brief Data structure for texture information.
     * @note Could be extended.(I have no idea it should have been extended lol.)
     * @attention Not included image data. Texture should be loaded to GPU using other modules.
     * This struct has implemented se::Identification for unique identification.
     * Identification for future resource management.
     */
    struct TextureInfo :public se::Identification<TextureInfo> {
        std::string path; // File path to the texture image
        std::string type; // e.g., "diffuse", "specular", "normal", etc.
    };
}

#endif //IMMEDIATERENDERERVK_TEXTUREINFO_H
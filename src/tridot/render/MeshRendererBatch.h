//
// Copyright (c) 2021 Julian Hinxlage. All rights reserved.
//

#ifndef TRIDOT_MESHRENDERERBATCH_H
#define TRIDOT_MESHRENDERERBATCH_H

#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"
#include <unordered_map>

namespace tridot {

    class MeshRendererBatch {
    public:
        MeshRendererBatch();

        void init(uint32_t elementSize, uint32_t maxInstanceCount, uint32_t maxTextureCount, Mesh *mesh, Shader *shader, std::vector<Attribute> layout);
        void *next();
        void updateBuffer();
        void submit();
        void reset();
        uint32_t getTextureUnit(Texture *texture);
        void resetTextures();

        Shader *shader;
        Mesh *mesh;
        uint32_t meshId;
        Ref<VertexArray> vertexArray;
        Ref<Buffer> instanceBuffer;
        std::unordered_map<Texture*, int> textures;
        std::vector<uint8_t> data;
        uint32_t elementSize;
        uint32_t instanceIndex;
        uint32_t updateIndex;
        uint32_t maxTextureCount;
        uint32_t maxInstanceCount;
    };

}

#endif //TRIDOT_MESHRENDERERBATCH_H
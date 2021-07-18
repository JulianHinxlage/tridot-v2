//
// Copyright (c) 2021 Julian Hinxlage. All rights reserved.
//

#ifndef TRIDOT_VERTEXARRAY_H
#define TRIDOT_VERTEXARRAY_H

#include "tridot/util/Ref.h"
#include "Buffer.h"
#include "enum.h"
#include <vector>

namespace tridot {

    class Attribute {
    public:
        Type type;
        int count;
        bool normalized;
        int size;
        int offset;

        Attribute(Type type = FLOAT, int count = 1, bool normalized = false);
    };

    class VertexArray {
    public:
        VertexArray();
        explicit VertexArray(const VertexArray &vertexArray);
        ~VertexArray();

        void bind();
        static void unbind();
        uint32_t getId();

        void addIndexBuffer(const Ref<Buffer> &indexBuffer, Type type);
        void addVertexBuffer(const Ref<Buffer> &vertexBuffer, std::vector<Attribute> layout, int divisor = 0);
        void submit(int vertexCount = -1, int instanceCount = -1);
        void setPrimitive(Primitive primitive);
        void clear();

    private:
        uint32_t id;
        uint32_t nextAttribute;
        Primitive primitive;

        class IBuffer{
        public:
            Ref<Buffer> indexBuffer;
            Type type;
        };
        std::vector<IBuffer> indexBuffer;

        class VBuffer{
        public:
            Ref<Buffer> vertexBuffer;
            std::vector<Attribute> layout;
            int divisor;
        };
        std::vector<VBuffer> vertexBuffer;
    };

}

#endif //TRIDOT_VERTEXARRAY_H

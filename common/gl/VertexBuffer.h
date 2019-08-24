#ifndef VIDREVOLT_GL_VERTEXBUFFER_H_
#define VIDREVOLT_GL_VERTEXBUFFER_H_

#include "GLUtil.h"

namespace vidrevolt {
    namespace gl {
        class VertexBuffer {
            public:
                VertexBuffer(const void* data, unsigned int size);
                ~VertexBuffer();

                void bind() const;
                void unbind() const;

            private:
                unsigned int glID_ = 0;
        };
    }
}

#endif

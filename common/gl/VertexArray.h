#ifndef VIDREVOLT_GL_VERTEXARRAY_H_
#define VIDREVOLT_GL_VERTEXARRAY_H_

#include "gl/GLUtil.h"

namespace vidrevolt {
    namespace gl {
        class VertexArray {
            public:
                VertexArray();
                ~VertexArray();

                void bind() const;
                void unbind() const;

            private:
                unsigned int glID_ = GL_FALSE;
        };
    }
}

#endif

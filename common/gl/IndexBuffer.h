#ifndef VIDREVOLT_GL_INDEXBUFFER_H_
#define VIDREVOLT_GL_INDEXBUFFER_H_

#include "GLUtil.h"

namespace vidrevolt {
    namespace gl {
        class IndexBuffer {
            public:
                IndexBuffer(const void* data, unsigned int count);
                ~IndexBuffer();

                void bind() const;
                void unbind() const;

                unsigned int getCount() const;

            private:
                unsigned int glID_ = GL_FALSE;
                unsigned int count_ = 0;
        };
    }
}

#endif


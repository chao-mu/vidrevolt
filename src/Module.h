#ifndef VIDREVOLT_MODULE_H_
#define VIDREVOLT_MODULE_H_

// STL
#include <string>

// Ours
#include "gl/GLUtil.h"
#include "gl/ShaderProgram.h"
#include "gl/RenderOut.h"
#include "AddressOrValue.h"
#include "Address.h"
#include "Value.h"
#include "ValueStore.h"

namespace vidrevolt {
    class Module {
        public:
            struct Param {
                public:
                    AddressOrValue value;
                    AddressOrValue amp;
                    AddressOrValue shift;
            };

            Module(const std::string& output, const std::string& path, const Resolution& res);

            void setParam(const std::string& input, Param src);
            const std::string& getOutput() const;
            const std::string& getPath() const;
            std::shared_ptr<gl::ShaderProgram> getShaderProgram();
            GLuint getFBO();
            GLenum getReadableBuf();
            std::shared_ptr<gl::Texture> getLastOutTex();
            Resolution getResolution();

            void setValues(std::shared_ptr<ValueStore> store, bool first_pass);

            void bind();
            void unbind();

            void compile(std::shared_ptr<ValueStore> store);

        private:
            const std::string output_;
            const std::string path_;
            std::map<std::string, Param> params_;
            std::map<std::string, AddressOrValue> uniforms_;
            std::shared_ptr<gl::RenderOut> render_out_;
            const Resolution resolution_;
            std::shared_ptr<gl::ShaderProgram> program_;

            static std::string toInputName(const std::string& name);
    };
}

#endif

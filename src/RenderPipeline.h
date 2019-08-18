#ifndef VIDREVOLT_RENDER_PIPELINE_H_
#define VIDREVOLT_RENDER_PIPELINE_H_

// STL
#include <memory>
#include <vector>
#include <map>

// Ours
#include "GLUtil.h"
#include "Media.h"
#include "ValueStore.h"
#include "Module.h"
#include "Resolution.h"

namespace vidrevolt {
    class RenderPipeline {
        public:
            RenderPipeline(
                    const Resolution& resolution,
                    std::shared_ptr<ValueStore> store,
                    std::vector<std::shared_ptr<Module>> steps);


            std::shared_ptr<Module> getLastStep();

            void load();
            void render();

        private:
            const Resolution resolution_;
            std::shared_ptr<ValueStore> store_;
            std::vector<std::shared_ptr<Module>> steps_;
            std::map<Address, std::shared_ptr<Media>> media_;
            bool first_pass_ = true;
    };
}

#endif


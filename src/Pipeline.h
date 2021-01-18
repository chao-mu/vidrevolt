#ifndef VIDREVOLT_PATCH_H_
#define VIDREVOLT_PATCH_H_

// STL
#include <memory>
#include <random>

// SFML
#include <SFML/Audio.hpp>

// Ours
#include "Video.h"
#include "Webcam.h"
#include "Image.h"
#include "Controller.h"
#include "Keyboard.h"
#include "BPMSync.h"
#include "gl/Renderer.h"
#include "RenderResult.h"

namespace vidrevolt {
    struct RenderStep {
        const std::string target;
        const std::string path;
    };

    class Pipeline {
        public:
            using ObjID = std::string;

            Pipeline();

            void load(const Resolution& resolution);

            Resolution getResolution();

            std::vector<RenderStep> getRenderSteps();

            RenderResult render(std::function<void()> f);
            void reconnectControllers();

            ObjID addVideo(const std::string& path, bool auto_reset, Video::Playback pb);
            ObjID addWebcam(int device);
            ObjID addKeyboard();
            ObjID addImage(const std::string& path);
            ObjID addOSC(int port, const std::string& path);
            ObjID addMidi(const std::string& path);
            ObjID addBPMSync();

            void playAudio(const std::string& path);
            void restartAudio();

            void setFPS(const std::string& id, double fps);
            void flipPlayback(const std::string& id);
            void tap(const std::string& sync_id);

            void addRenderStep(const std::string& target, const std::string& path, gl::ParamSet params, std::vector<Address> video_deps);

            std::map<std::string, std::shared_ptr<Controller>> getControllers() const;

            float rand();

        private:
            ObjID next_id(const std::string& comment);

            void setVideo(const std::string& key, std::unique_ptr<Video> vid);
            void setWebcam(const std::string& key, std::unique_ptr<Webcam> vid);
            void setBPMSync(const std::string& key, std::shared_ptr<BPMSync> vid);
            void setController(const std::string& key, std::shared_ptr<Controller> controller);

            std::map<std::string, std::shared_ptr<BPMSync>> bpm_syncs_;
            std::map<Address, std::unique_ptr<Video>> videos_;
            std::map<Address, std::unique_ptr<Webcam>> webcams_;
            std::map<std::string, std::shared_ptr<Controller>> controllers_;
            Resolution resolution_;
            std::unique_ptr<gl::Renderer> renderer_ = std::make_unique<gl::Renderer>();

            std::map<Address, bool> in_use_;
            std::map<Address, bool> last_in_use_;

            size_t obj_id_cursor_ = 0;

            std::vector<RenderStep> render_steps_;
            sf::Music music_;

            std::random_device rand_dev_;
            std::mt19937 rand_gen_;
    };
}

#endif

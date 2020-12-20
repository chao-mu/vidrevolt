#include "Pipeline.h"

// Ours
#include "midi/Device.h"
#include "KeyboardManager.h"
#include "osc/Server.h"
#include "gl/ParamSet.h"

// STL
#include <stdexcept>

namespace vidrevolt {
    void Pipeline::load(const Resolution& resolution) {
        resolution_ = resolution;
        renderer_->setResolution(resolution_);

        for (auto& vid_kv : videos_) {
            vid_kv.second->waitForLoaded();
        }
    }

    void Pipeline::restartAudio() {
        music_.stop();
        music_.play();
    }

    void Pipeline::playAudio(const std::string& path) {
        if (!music_.openFromFile(path)) {
            throw std::runtime_error("Unable to load audio file: " + path);
        }

        music_.play();
    }

    void Pipeline::setBPMSync(const std::string& key, std::shared_ptr<BPMSync> sync) {
        setController(key, sync);
        bpm_syncs_[key] = sync;
    }

    Pipeline::ObjID Pipeline::addKeyboard() {
        ObjID id = next_id("keyboard");

        setController(id, KeyboardManager::makeKeyboard());

        return id;
    }

    Pipeline::ObjID Pipeline::addBPMSync() {
        ObjID id = next_id("bpm_sync");

        setBPMSync(id, std::make_shared<BPMSync>());

        return id;
    }

    Pipeline::ObjID Pipeline::addOSC(int port, const std::string& path) {
        ObjID id = next_id(path);

        auto osc = std::make_shared<osc::Server>(port, path);
        osc->start();

        setController(id, std::move(osc));

        return id;

    }

    Pipeline::ObjID Pipeline::addVideo(const std::string& path, bool auto_reset, Video::Playback pb) {
        ObjID id = next_id(path);
        auto vid =  std::make_unique<Video>(path, auto_reset, pb);
        vid->start();
        setVideo(id, std::move(vid));

        return id;
    }

    Pipeline::ObjID Pipeline::addWebcam(int device) {
        ObjID id = next_id("webcam(" + std::to_string(device) + ")");
        auto vid =  std::make_unique<Webcam>(device);
        vid->start();
        setWebcam(id, std::move(vid));

        return id;
    }


    Pipeline::ObjID Pipeline::addMidi(const std::string& path) {
        ObjID id = next_id(path);
        auto dev = std::make_shared<midi::Device>(path);
        dev->start();

        setController(id, dev);

        return id;
    }

    Pipeline::ObjID Pipeline::addImage(const std::string& path) {
        ObjID id = next_id(path);

        cv::Mat frame = Image::load(path);
        renderer_->render(id, frame);

        return id;
    }

    void Pipeline::tap(const std::string& sync_id) {
        if (!bpm_syncs_.count(sync_id)) {
            throw std::runtime_error("Attempt to tap non-existent BPM sync");
        }

        bpm_syncs_.at(sync_id)->tap();
    }

    void Pipeline::reconnectControllers() {
        for (auto& kv : controllers_) {
            kv.second->reconnect();
        }
    }

    void Pipeline::addRenderStep(const std::string& target, const std::string& path, gl::ParamSet params, std::vector<Address> video_deps) {
        for (const auto& addr : video_deps) {
            in_use_[addr] = true;
            FrameSource* source = nullptr;
            if (videos_.count(addr) > 0) {
                source = videos_.at(addr).get();
            } else if (webcams_.count(addr) > 0) {
                source = webcams_.at(addr).get();
            }

            if (source != nullptr) {
                auto frame_opt = source->nextFrame();
                if (frame_opt) {
                    renderer_->render(addr, frame_opt.value());
                }
            }
        }

        renderer_->render(target, path, params);
        render_steps_.push_back(RenderStep{target, path});
    }

    void Pipeline::setFPS(const std::string& id, double fps) {
        if (!videos_.count(id)) {
            throw std::runtime_error("Attempt to set fps on non-existent video");
        }

        videos_.at(id)->setFPS(fps);
    }

    void Pipeline::flipPlayback(const std::string& id) {
        if (!videos_.count(id)) {
            throw std::runtime_error("Attempt to flip non-existent video");
        }

        videos_.at(id)->flipPlayback();
    }

    std::map<std::string, std::shared_ptr<Controller>> Pipeline::getControllers() const {
       return controllers_;
    }

    std::shared_ptr<gl::RenderOut> Pipeline::render(std::function<void()> f) {
        last_in_use_ = in_use_;
        in_use_.clear();

        for (const auto& kv : controllers_) {
            kv.second->poll();
        }

        render_steps_.clear();

        // Perform render
        f();

        // Trigger out/in focus
        for (const auto& kv : videos_) {
            const auto& addr = kv.first;
            auto& vid = kv.second;
            bool was_in_use = last_in_use_.count(addr) > 0 ? last_in_use_.at(addr) : false;
            bool is_in_use = in_use_.count(addr) > 0 ? in_use_.at(addr) : false;

            if (was_in_use && !is_in_use) {
                vid->outFocus();
            } else if (!was_in_use && is_in_use) {
                vid->inFocus();
            }
        }

        return renderer_->getLast();
    }

    void Pipeline::setWebcam(const std::string& key, std::unique_ptr<Webcam> vid) {
        webcams_[key] = std::move(vid);
    }

    void Pipeline::setVideo(const std::string& key, std::unique_ptr<Video> vid) {
        videos_[key] = std::move(vid);
    }

    void Pipeline::setController(const std::string& key, std::shared_ptr<Controller> controller) {
        controllers_[key] = controller;
    }

    Pipeline::ObjID Pipeline::next_id(const std::string& comment) {
        obj_id_cursor_++;
        return "ObjID:" + std::to_string(obj_id_cursor_) + ":" + comment;
    }

    Resolution Pipeline::getResolution() {
        return resolution_;
    }

    std::vector<RenderStep> Pipeline::getRenderSteps() {
        return render_steps_;
    }
}

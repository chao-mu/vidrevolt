#include "Patch.h"

// Ours
#include "midi/Device.h"
#include "KeyboardManager.h"
#include "osc/Server.h"
#include "gl/ParamSet.h"

namespace vidrevolt {
    sol::table toTable(sol::state& lua, Value v) {
        auto vec = v.getVec4();
        auto tab = lua.create_table(4);
        for (size_t i = 0; i < vec.size(); i++) {
            tab[i] = vec[i];
        }

        return tab;
    }

    Patch::Patch(const std::string& path) : path_(path) {}

    AddressOrValue Patch::toAOV(sol::object obj) {
        if (obj.is<float>()) {
            return Value(obj.as<float>());
        } else if (obj.is<sol::table>()) {
            auto obj_arr = obj.as<std::vector<sol::object>>();
            if (obj_arr.size() == 2 && obj_arr[1].is<std::string>()) {
                Address addr(obj_arr[0].as<std::string>());
                addr.setSwiz(obj_arr[1].as<std::string>());
                return addr;
            } else {
                return Value(obj.as<std::vector<float>>());
            }
        }  else if (obj.is<std::string>()) {
            return Address(obj.as<std::string>());
        }

        throw std::runtime_error("Unsupported lua value type");
    }

    void Patch::luafunc_flipPlayback(const std::string& id) {
        if (!videos_.count(id)) {
            throw std::runtime_error("Attempt to tap non-existent video");
        }

        videos_.at(id)->flipPlayback();
    }

    Patch::ObjID Patch::luafunc_Image(const std::string& path) {
        ObjID id = next_id(path);

        cv::Mat frame = Image::load(path);
        renderer_->render(id, frame);

        return id;
    }

    Patch::ObjID Patch::next_id(const std::string& comment) {
        obj_id_cursor_++;
        return "ObjID:" + std::to_string(obj_id_cursor_) + ":" + comment;
    }

    void Patch::luafunc_preload(sol::table shaders) {
        for (const auto& kv : shaders) {
            renderer_->preloadModule(kv.second.as<std::string>());
        }
    }

    Patch::ObjID Patch::luafunc_Keyboard() {
        ObjID id = next_id("keyboard");

        setController(id, KeyboardManager::makeKeyboard());

        return id;
    }

    Patch::ObjID Patch::luafunc_BPM() {
        ObjID id = next_id("bpm_sync");

        setBPMSync(id, std::make_shared<BPMSync>());

        return id;
    }

    void Patch::setBPMSync(const std::string& key, std::shared_ptr<BPMSync> sync) {
        setController(key, sync);
        bpm_syncs_[key] = sync;
    }

    Patch::ObjID Patch::luafunc_OSC(const std::string& path, int port) {
        ObjID id = next_id(path);

        auto osc = std::make_shared<osc::Server>(port, path);
        osc->start();

        setController(id, std::move(osc));

        return id;
    }

    void Patch::luafunc_tap(const std::string& sync_id) {
        if (!bpm_syncs_.count(sync_id)) {
            throw std::runtime_error("Attempt to tap non-existent BPM sync");
        }

        bpm_syncs_.at(sync_id)->tap();
    }

    void Patch::load() {
        lua_.open_libraries(
            sol::lib::base,
            sol::lib::package,
            sol::lib::coroutine,
            sol::lib::string,
            sol::lib::os,
            sol::lib::math,
            sol::lib::table,
            sol::lib::bit32,
            sol::lib::io
        );

        // Our custom functions
        lua_.set_function("Video", &Patch::luafunc_Video, this);
        lua_.set_function("Image", &Patch::luafunc_Image, this);
        lua_.set_function("OSC", &Patch::luafunc_OSC, this);
        lua_.set_function("BPM", &Patch::luafunc_BPM, this);
        lua_.set_function("Keyboard", &Patch::luafunc_Keyboard, this);
        lua_.set_function("Midi", &Patch::luafunc_Midi, this);
        lua_.set_function("rend", &Patch::luafunc_rend, this);
        lua_.set_function("getControlValues", &Patch::luafunc_getControlValues, this);
        lua_.set_function("tap", &Patch::luafunc_tap, this);
        lua_.set_function("preload", &Patch::luafunc_preload, this);
        lua_.set_function("flipPlayback", &Patch::luafunc_flipPlayback, this);

        auto time = std::chrono::high_resolution_clock::now();
        auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch());
        lua_["time_ms"] = time_ms.count();
        lua_["time_delta"] = 0;

        lua_.script_file(path_);

        resolution_.width = lua_.get_or("width", 1920);
        resolution_.height = lua_.get_or("height", 1920);
        renderer_->setResolution(resolution_);

        for (auto& vid_kv : videos_) {
            vid_kv.second->waitForLoaded();
        }
    }

    Patch::ObjID Patch::luafunc_Video(const std::string& path, sol::table args) {
        Video::Playback pb = Video::Forward;
        bool auto_reset = false;

        if (args) {
            for (const auto& arg : args) {
                auto arg_s = arg.second.as<std::string>();
                if (arg_s == "reverse") {
                    pb = Video::Reverse;
                } else if (arg_s == "forward") {
                    pb = Video::Forward;
                } else if (arg_s == "mirror") {
                    pb = Video::Mirror;
                } else if (arg_s == "reset") {
                    auto_reset = true;
                } else {
                    throw std::runtime_error("Unexpected Video argument " + arg_s);
                }
            }
        }

        ObjID id = next_id(path);
        auto vid =  std::make_unique<Video>(path, auto_reset, pb);
        vid->start();
        setVideo(id, std::move(vid));

        return id;
    }

    void Patch::reconnectControllers() {
        for (auto& kv : controllers_) {
            kv.second->reconnect();
        }
    }

    sol::table Patch::luafunc_getControlValues(const ObjID& controller_id) {
        if (controllers_.count(controller_id) == 0) {
            throw std::runtime_error("Control values requested for non-existent controller");
        }
        sol::table ret = lua_.create_table_with();

        std::vector<std::string> keys;
        for (const auto& kv : controllers_.at(controller_id)->getValues()) {
            ret[kv.first] = kv.second.at(0);
        }

        return ret;
    }

    Patch::ObjID Patch::luafunc_Midi(const std::string& path) {
        ObjID id = next_id(path);
        auto dev = std::make_shared<midi::Device>(path);
        dev->start();

        setController(id, dev);

        return id;
    }

    void Patch::luafunc_rend(const std::string& target, const std::string& path, sol::table inputs){
        gl::ParamSet params;
        if (inputs) {
            for (const auto& input_kv : inputs) {
                const std::string key = input_kv.first.as<std::string>();
                sol::object value = input_kv.second;

                gl::Param param;
                if (value.is<sol::table>()) {
                    auto tab = value.as<sol::table>();

                    if (tab["input"]) {
                        param.value = toAOV(tab.get<sol::object>("input"));
                    }

                    if (tab["amp"]) {
                        param.amp = toAOV(tab.get<sol::object>("amp"));
                    }

                    if (tab["shift"]) {
                        param.shift = toAOV(tab.get<sol::object>("shift"));
                    }

                    if (tab["pow"]) {
                        param.pow = toAOV(tab.get<sol::object>("pow"));
                    }
                } else {
                    param.value = toAOV(value);
                }

                params[key] = param;

                // Mark dependencies as used and render if need be
                for (const AddressOrValue& aov : {param.value, param.amp, param.shift, param.pow}) {
                    if (std::holds_alternative<Address>(aov)) {
                        auto addr = std::get<Address>(aov);

                        in_use_[addr] = true;
                        if (videos_.count(addr) > 0) {
                            auto frame_opt = videos_.at(addr)->nextFrame();
                            if (frame_opt) {
                                renderer_->render(addr, frame_opt.value());
                            }
                        }
                    }
                }
            }
        }

        renderer_->render(target, path, params);
    }

    std::shared_ptr<gl::RenderOut> Patch::render() {
        last_in_use_ = in_use_;
        in_use_.clear();

        // Calculate time delta (fractions of seconds)
        if (!last_time_) {
            last_time_ = std::chrono::high_resolution_clock::now();
        }

        auto time = std::chrono::high_resolution_clock::now();
        float time_delta = std::chrono::duration<float>(time - last_time_.value()).count();
        last_time_ = time;
        lua_["time_delta"] = time_delta;

        auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch());
        lua_["time_ms"] = time_ms.count();

        for (const auto& kv : controllers_) {
            kv.second->poll();
        }

        sol::function renderFunc = lua_["render"];
        if (!renderFunc) {
            throw std::runtime_error("Expected a function 'render'");
        }

        // Perform render
        renderFunc();

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

    void Patch::setVideo(const std::string& key, std::unique_ptr<Video> vid) {
        videos_[key] = std::move(vid);
    }

    void Patch::setController(const std::string& key, std::shared_ptr<Controller> controller) {
        controller->connect([key, this](const std::string& control, Value v) {
            auto listener = lua_.get<sol::function>("onControl");
            if (lua_["onControl"]) {
                listener(key, control, v.at(0));
            }
        });

        controllers_[key] = controller;
    }

    Resolution Patch::getResolution() {
        return resolution_;
    }
}

#include "LuaFrontend.h"

namespace vidrevolt {
    sol::table toTable(sol::state& lua, Value v) {
        auto vec = v.getVec4();
        auto tab = lua.create_table(4);
        for (size_t i = 0; i < vec.size(); i++) {
            tab[i] = vec[i];
        }

        return tab;
    }

    LuaFrontend::LuaFrontend(const std::string& path, std::shared_ptr<Pipeline> pipeline) :
        path_(path), pipeline_(pipeline) {}

    void LuaFrontend::load() {
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
        lua_.set_function("Video", &LuaFrontend::luafunc_Video, this);
        lua_.set_function("Image", &LuaFrontend::luafunc_Image, this);
        lua_.set_function("OSC", &LuaFrontend::luafunc_OSC, this);
        lua_.set_function("BPM", &LuaFrontend::luafunc_BPM, this);
        lua_.set_function("Keyboard", &LuaFrontend::luafunc_Keyboard, this);
        lua_.set_function("Midi", &LuaFrontend::luafunc_Midi, this);
        lua_.set_function("rend", &LuaFrontend::luafunc_rend, this);
        lua_.set_function("getControlValues", &LuaFrontend::luafunc_getControlValues, this);
        lua_.set_function("tap", &LuaFrontend::luafunc_tap, this);
        //lua_.set_function("preload", &LuaFrontend::luafunc_preload, this);
        lua_.set_function("flipPlayback", &LuaFrontend::luafunc_flipPlayback, this);
        lua_.set_function("setFPS", &LuaFrontend::luafunc_setFPS, this);

        auto time = std::chrono::high_resolution_clock::now();
        auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch());
        lua_["time_ms"] = time_ms.count();
        lua_["time_delta"] = 0;

        lua_.script_file(path_);

        Resolution resolution;
        resolution.width = lua_.get_or("width", 1920);
        resolution.height = lua_.get_or("height", 1920);

        pipeline_->load(resolution);
    }

    std::shared_ptr<gl::RenderOut> LuaFrontend::render() {
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

        sol::function renderFunc = lua_["render"];
        if (!renderFunc) {
            throw std::runtime_error("Expected a function 'render'");
        }

        return pipeline_->render(renderFunc);
    }

    LuaFrontend::ObjID LuaFrontend::luafunc_Video(const std::string& path, const sol::table& args) {
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

        return pipeline_->addVideo(path, auto_reset, pb);
    }

    sol::table LuaFrontend::luafunc_getControlValues(const ObjID& controller_id) {
        auto controllers = pipeline_->getControllers();

        if (controllers.count(controller_id) == 0) {
            throw std::runtime_error("Control values requested for non-existent controller");
        }
        sol::table ret = lua_.create_table_with();

        std::vector<std::string> keys;
        for (const auto& kv : controllers.at(controller_id)->getValues()) {
            ret[kv.first] = kv.second.at(0);
        }

        return ret;
    }

    std::string LuaFrontend::luafunc_rend(const std::string& target, const std::string& path, sol::table inputs) {
        gl::ParamSet params;
        std::vector<Address> video_deps;

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
                        video_deps.push_back(std::get<Address>(aov));
                    }
                }
            }
        }

        pipeline_->addRenderStep(target, path, params, video_deps);

        return target;
    }

    AddressOrValue LuaFrontend::toAOV(sol::object obj) {
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

    void LuaFrontend::luafunc_setFPS(const std::string& id, double fps) {
        pipeline_->setFPS(id, fps);
    }

    void LuaFrontend::luafunc_flipPlayback(const std::string& id) {
        pipeline_->flipPlayback(id);
    }

    /* Commented out because I didn't feel like puting in the effort to make it work with
     * the refactor.
    void LuaFrontend::luafunc_preload(sol::table shaders) {
        for (const auto& kv : shaders) {
            renderer_->preloadModule(kv.second.as<std::string>());
        }
    }
    */

    LuaFrontend::ObjID LuaFrontend::luafunc_Keyboard() {
        return connect(pipeline_->addKeyboard());
    }

    LuaFrontend::ObjID LuaFrontend::luafunc_BPM() {
        return connect(pipeline_->addBPMSync());
    }

    LuaFrontend::ObjID LuaFrontend::luafunc_OSC(const std::string& path, int port) {
        return connect(pipeline_->addOSC(port, path));
    }

    LuaFrontend::ObjID LuaFrontend::luafunc_Image(const std::string& path) {
        return connect(pipeline_->addImage(path));
    }

    LuaFrontend::ObjID LuaFrontend::luafunc_Midi(const std::string& path) {
        return connect(pipeline_->addMidi(path));
    }


    void LuaFrontend::luafunc_tap(const std::string& sync_id) {
        pipeline_->tap(sync_id);
    }

    LuaFrontend::ObjID LuaFrontend::connect(const std::string& key) {
        pipeline_->getControllers().at(key)->connect([key, this](const std::string& control, Value v) {
            auto listener = lua_.get<sol::function>("onControl");
            if (lua_["onControl"]) {
                listener(key, control, v.at(0));
            }
        });

        return key;
    };
}

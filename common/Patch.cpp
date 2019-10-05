#include "Patch.h"

// Ours
#include "cmd/OverwriteGroup.h"
#include "cmd/OverwriteVar.h"
#include "cmd/Reverse.h"
#include "cmd/Rotate.h"
#include "cmd/TapTempo.h"
#include "midi/Device.h"



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

    bool Patch::hasRenderStep(const std::string& label) {
        for (const auto& step : render_steps_) {
            if (step->getOutput() == label) {
                return true;
            }
        }

        return false;
    }

    AddressOrValue Patch::toAOV(sol::object obj) {
        if (obj.is<float>()) {
            return Value(obj.as<float>());
        } else if (obj.is<std::vector<float>>()) {
            return Value(obj.as<std::vector<float>>());
        }  else if (obj.is<std::string>()) {
            std::string id = obj.as<std::string>();
            if (videos_.count(id) || images_.count(id) || hasRenderStep(id)) {
                return Address(obj.as<std::string>());
            } else {
                throw std::runtime_error("Unrecognized address specified");
            }
        } else {
            throw std::runtime_error("Unsupported lua value type");
        }
    }


    void Patch::load() {
        lua_.open_libraries(sol::lib::base, sol::lib::package);

        // Our custom functions
        lua_.set_function("Video", &Patch::luafunc_Video, this);
        lua_.set_function("Midi", &Patch::luafunc_Midi, this);
        lua_.set_function("rend", &Patch::luafunc_rend, this);
        lua_.set_function("getControlValues", &Patch::luafunc_getControlValues, this);

        lua_.script_file(path_);

        resolution_.width = lua_.get_or("width", 1920);
        resolution_.height = lua_.get_or("height", 1920);

        populateRenderSteps();
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

        ObjID id = path;
        videos_[id] = std::make_unique<Video>(path, auto_reset, pb);
        videos_[id]->start();

        return id;
    }

    sol::table Patch::luafunc_getControlValues(const ObjID& controller_id) {
        if (controllers_.count(controller_id) == 0) {
            throw std::runtime_error("Control values requested for non-existent controller");
        }
        sol::table ret = lua_.create_table_with();

        std::vector<std::string> keys;
        for (const auto& kv : controllers_.at(controller_id)->getValues()) {
            ret[kv.first] = toTable(lua_, kv.second);
        }

        return ret;
    }


    Patch::ObjID Patch::luafunc_Midi(const std::string& path) {
        ObjID id = path;
        auto dev = std::make_shared<midi::Device>(path);
        dev->start();

        dev->connect([id, this](const std::string& control, Value v) {
            auto listener = lua_.get<sol::function>("onControl");
            if (lua_["onControl"]) {
                listener(id, control, toTable(lua_, v));
            }
        });

        controllers_[id] = dev;

        return id;
    }

    Patch::ObjID Patch::luafunc_rend(const std::string& label, const std::string& path, sol::table inputs){
        ObjID id = label;

        auto step = std::make_unique<RenderStep>(id, path, getResolution());
        if (inputs) {
            for (const auto& input_kv : inputs) {
                const std::string key = input_kv.first.as<std::string>();
                sol::object value = input_kv.second;

                RenderStep::Param param;
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

                step->setParam(key, param);
            }
        }

        addRenderStep(std::move(step));

        return id;
    }


    void Patch::addCommand(const Trigger& trigger, std::unique_ptr<cmd::Command> c) {
        commands_.push_back(std::move(c));
        auto cptr = commands_.back().get();

        std::string dev = trigger.getFront();
        std::string ctrl = trigger.getBack();
        if (controllers_.count(dev) <= 0) {
            throw std::runtime_error("Non-existent controller referenced in trigger " + trigger.str());
        }

        controllers_.at(dev)->connect(ctrl, [cptr, this](Value v) {
            if (v.getBool()) {
                interpretCommand(cptr);
            }
        });
    }

    void Patch::visitGroupMember(const Address& addr, std::function<void(Group&, const std::string&)> f) const {
        if (addr.getDepth() == 1) {
            std::string key = addr.str();
            if (aovs_.count(key) > 0) {
                AddressOrValue aov = aovs_.at(key);
                if (std::holds_alternative<Address>(aov)) {
                    visitGroupMember(std::get<Address>(aov), f);
                    return;
                }
            }
        } else if (addr.getDepth() == 2) {
            std::string front = addr.getFront();
            std::string back = addr.getBack();

            if (groups_.count(front) > 0) {
                auto& group = groups_.at(front);
                f(*group.get(), back);
                return;
            }
        }

        throw std::runtime_error("Expected '" + addr.str() + "' to point to a group member");
    }

    void Patch::visitReferable(const Address& addr, std::function<void(const std::string&, Referable)> f, const Address& tail) const {
        if (addr.getDepth() == 1 && tail.getDepth() == 0) {
            std::string key = addr.str();

            if (videos_.count(key) > 0) {
                f(key, videos_.at(key).get());
            } else if (images_.count(key) > 0) {
                f(key, images_.at(key).get());
            } else if (controllers_.count(key) > 0) {
                f(key, controllers_.at(key).get());
            } else if (groups_.count(key) > 0) {
                f(key, groups_.at(key).get());
            } else if (aovs_.count(key) > 0) {
                auto& aov = aovs_.at(key);
                if (std::holds_alternative<Value>(aov)) {
                    f(key, std::get<Value>(aov));
                } else if (std::holds_alternative<Address>(aov)) {
                    visitReferable(std::get<Address>(aov), f);
                }
            } else if (render_step_resolutions_.count(key) > 0) {
                 f(key, key);
            } else {
                throw std::runtime_error("Invalid address " + addr.str());
            }
        } else if (tail.str() == "resolution") {
            Resolution res;
            visitReferable(addr, [&res, addr, this](const std::string&, Referable r) {
                if (std::holds_alternative<Media*>(r)) {
                    res = std::get<Media*>(r)->getResolution();
                } else if (std::holds_alternative<RenderStep::Label>(r)) {
                    res = render_step_resolutions_.at(std::get<RenderStep::Label>(r));
                } else {
                    throw std::runtime_error("Expected address '" + addr.str() + "' to refer to media");
                }
            });

            f((addr + tail).str(),
                Value({static_cast<float>(res.width), static_cast<float>(res.height)}));
        } else if (addr.getDepth() == 1 && tail.getDepth() == 1) {
            std::string head = addr.str();
            std::string back = tail.str();

            if (groups_.count(head) > 0) {
                auto& group = groups_.at(head);

                auto aov_opt = group->get(back);
                if (!aov_opt) {
                    throw std::runtime_error("Group '" + head + "' did not have member '" + back + "'");
                }

                if (std::holds_alternative<Value>(*aov_opt)) {
                    f((addr + tail).str(), std::get<Value>(*aov_opt));
                } else if (std::holds_alternative<Address>(*aov_opt)) {
                    visitReferable(std::get<Address>(*aov_opt), f);
                }
            } else if (controllers_.count(head) > 0) {
                f((addr + tail).str(), controllers_.at(head)->getValue(back));
            } else {
                throw std::runtime_error("Invalid address " + head + "." + back);
            }
        } else if (addr.getDepth() == 0) {
            f((addr + tail).str(), tail.str());
        } else {
            std::string back = addr.getBack();
            visitReferable(addr.withoutBack(), f, tail.withHead(back));
        }
    }

    void Patch::startRender() {
        auto f = [](Media* m) {
            m->resetInUse();
        };

        for (const auto& kv : videos_) {
            f(kv.second.get());
        }

        for (const auto& kv : images_) {
            f(kv.second.get());
        }

        // Calculate time delta (fractions of seconds)
        if (!last_time_) {
            last_time_ = std::chrono::high_resolution_clock::now();
        }

        auto time = std::chrono::high_resolution_clock::now();
        float time_delta = std::chrono::duration<float>(time - last_time_.value()).count();
        last_time_ = time;
        setAOV("time_delta", Value(time_delta));

        // Set function arguments for our lua controller functions
        for (const auto& kv : lua_controllers_) {
            std::shared_ptr<lua::Controller> lua = kv.second;

            // Set arguments for each function of this controller
            for (const auto& kv : lua->getControls()) {
                const auto& control_name = kv.first;
                const std::vector<AddressOrValue> params = kv.second.params;

                // Convert our parameter list into resolved arguments.
                std::vector<Value> args;
                for (const auto& param : params) {
                    if (std::holds_alternative<Value>(param)) {
                        args.push_back(std::get<Value>(param));
                        continue;
                    }


                    auto addr = std::get<Address>(param);

                    bool found = false;
                    visitReferable(addr, [&found, &args](const std::string& /*name*/, Referable r) {
                        if (std::holds_alternative<Value>(r)) {
                            args.push_back(std::get<Value>(r));
                            found = true;
                        }
                    });

                    if (!found) {
                        throw std::runtime_error(addr.str() + " is not a Value address");
                    }
                }

                // Set the arguments
                lua->setControlArgs(control_name, args);
            }
        }

        for (const auto& kv : controllers_) {
            kv.second->poll();
        }

        populateRenderSteps();
    }

    void Patch::populateRenderSteps() {
        // Populate our render steps
        render_steps_.clear();
        sol::function renderFunc = lua_["render"];
        if (!renderFunc) {
            throw std::runtime_error("Expected a function 'render'");
        }

        renderFunc();
    }

    void Patch::endRender() {
        auto f = [](Media* m) {
            if (m->wasInUse() && !m->isInUse()) {
                m->outFocus();
            } else if (!m->wasInUse() && m->isInUse()) {
                m->inFocus();
            }
        };

         for (const auto& kv : videos_) {
            f(kv.second.get());
         }

         for (const auto& kv : images_) {
             f(kv.second.get());
         }
    }

    const std::vector<std::unique_ptr<RenderStep>>& Patch::getRenderSteps() const {
        return render_steps_;
    }

    const std::map<std::string, std::unique_ptr<Video>>& Patch::getVideos() const {
        return videos_;
    }

    const std::map<std::string, std::unique_ptr<Image>>& Patch::getImages() const {
        return images_;
    }

    const std::map<std::string, std::shared_ptr<Controller>>& Patch::getControllers() const {
        return controllers_;
    }

    const std::map<std::string, std::unique_ptr<Group>>& Patch::getGroups() const {
        return groups_;
    }

    void Patch::setVideo(const std::string& key, std::unique_ptr<Video> vid) {
        videos_[key] = std::move(vid);
    }

    void Patch::setImage(const std::string& key, std::unique_ptr<Image> image) {
        images_[key] = std::move(image);
    }

    void Patch::setController(const std::string& key, std::shared_ptr<Controller> controller) {
        controllers_[key] = controller;
    }

    void Patch::setLuaController(const std::string& key, std::shared_ptr<lua::Controller> lua) {
        setController(key, lua);
        lua_controllers_[key] = lua;
    }

    void Patch::setGroup(const std::string& key, std::unique_ptr<Group> group) {
        groups_[key] = std::move(group);
    }

    void Patch::addRenderStep(std::unique_ptr<RenderStep> step) {
        render_step_resolutions_[step->getOutput()] = step->getResolution();
        render_steps_.push_back(std::move(step));
    }

    bool Patch::isMedia(const Address& addr) const {
        bool is = false;
        visitReferable(addr, [&is](const std::string& /*name*/, Referable r) {
            if (std::holds_alternative<Media*>(r) || std::holds_alternative<RenderStep::Label>(r)) {
                is = true;
            }
        });

        return is;
    }

    bool Patch::isSwizzable(const Address& addr) const {
        bool is = false;
        visitReferable(addr, [&is](const std::string& /*name*/, Referable r) {
            is = std::holds_alternative<Media*>(r) || std::holds_alternative<RenderStep::Label>(r) || std::holds_alternative<Value>(r);
        });

        return is;
    }

    void Patch::setResolution(const Resolution& res) {
        resolution_ = res;
    }

    void Patch::setAOV(const std::string& key, const AddressOrValue& aov) {
        aovs_[key] = aov;
    }

    void Patch::interpretCommand(cmd::Command* c) {
        auto overwrite_group = dynamic_cast<cmd::OverwriteGroup*>(c);
        if (overwrite_group != nullptr) {
            visitGroupMember(overwrite_group->target, [&overwrite_group](Group& g, const std::string& s) {
                g.overwrite(s, overwrite_group->replacement);
            });
            return;
        }

        auto overwrite_var = dynamic_cast<cmd::OverwriteVar*>(c);
        if (overwrite_var != nullptr) {
            std::string key = overwrite_var->target.str();
            if (aovs_.count(key) <= 0) {
                throw std::runtime_error("overwrite-var command attempted to overwrite an undeclared variable");
            }

            aovs_[key] = overwrite_var->replacement;
            return;
        }

        auto reverse = dynamic_cast<cmd::Reverse*>(c);
        if (reverse != nullptr) {
            visitReferable(reverse->target, [&reverse](const std::string& /*name*/, Referable r) {
                if (std::holds_alternative<Media*>(r)) {
                    auto vid = dynamic_cast<Video*>(std::get<Media*>(r));
                    if (vid != nullptr) {
                        vid->flipPlayback();
                    }
                } else {
                    throw std::runtime_error("Attempted to reverse non-video '" + reverse->target.str() + "'");
                }
            });
            return;
        }

        auto rotate = dynamic_cast<cmd::Rotate*>(c);
        if (rotate != nullptr) {
            std::cerr << "TODO: Implement rotate" << std::endl;
            return;
        }

        auto tap_tempo = dynamic_cast<cmd::TapTempo*>(c);
        if (tap_tempo != nullptr) {
            std::cerr << "TODO: Implement TapTempo" << std::endl;
            return;
        }
    }

    Resolution Patch::getResolution() {
        return resolution_;
    }
}

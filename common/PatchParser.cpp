#include "PatchParser.h"

// STL
#include <stdexcept>
#include <optional>

// OpenCV
#include <opencv2/opencv.hpp>

// Ours
#include "Video.h"
#include "Image.h"
#include "cmd/OverwriteGroup.h"
#include "cmd/OverwriteVar.h"
#include "cmd/Reverse.h"
#include "cmd/Rotate.h"
#include "cmd/TapTempo.h"
#include "fileutil.h"
#include "Keyboard.h"

#define KEY_RESET "reset"
#define KEY_MEMBERS "members"
#define KEY_ARGS "args"
#define KEY_INDEX "index"
#define KEY_COMMANDS "commands"
#define KEY_COMMAND "command"
#define KEY_TRIGGER_AND_ARGS "trigger-and-args"
#define KEY_TRIGGER "trigger"
#define KEY_MEDIAS "media"
#define KEY_GROUPS "groups"
#define KEY_RENDER "render"
#define KEY_PLAYBACK "playback"
#define KEY_TYPE "type"
#define KEY_OUTPUT "output"
#define KEY_INPUTS "inputs"
#define KEY_PATH "path"
#define KEY_RESOLUTION "resolution"
#define KEY_WIDTH "width"
#define KEY_HEIGHT "height"
#define KEY_REPEAT "repeat"
#define KEY_INPUT "input"
#define KEY_AMP "amp"
#define KEY_SHIFT "shift"
#define KEY_POW "pow"
#define KEY_SCALE_FILTER "sizeFilter"
#define KEY_CONTROLLERS "controllers"
#define CONTROLLER_TYPE_MIDI "midi"
#define CONTROLLER_TYPE_KEYBOARD "keyboard"
#define CONTROLLER_TYPE_BPM_SYNC "bpm-sync"
#define MEDIA_TYPE_IMAGE "image"
#define MEDIA_TYPE_VIDEO "video"
#define KEY_VARS "vars"

namespace vidrevolt {
    PatchParser::PatchParser(const std::string& path) : Parser(), path_(path) {}

    void PatchParser::parse() {
        const YAML::Node patch = YAML::LoadFile(path_);

        parseMedia(patch);
        parseVars(patch);
        parseGroups(patch);
        parseControllers(patch);
        parseModules(patch);
        parseCommands(patch);
    }

    std::shared_ptr<ValueStore> PatchParser::getValueStore() {
        return store_;
    }

    std::map<std::string, std::shared_ptr<Video>> PatchParser::getVideos() {
        return videos_;
    }

    std::map<std::string, std::shared_ptr<Image>> PatchParser::getImages() {
        return images_;
    }

    std::map<std::string, std::shared_ptr<Controller>> PatchParser::getControllers() {
        return controllers_;
    }

    std::map<std::string, std::shared_ptr<Group>> PatchParser::getGroups() {
        return groups_;
    }

    std::vector<std::shared_ptr<cmd::Command>> PatchParser::getCommands() {
        return commands_;
    }

    std::shared_ptr<cmd::Command> PatchParser::loadCommand(int num, const std::string& name, Trigger trigger, std::vector<AddressOrValue> args) {
        std::shared_ptr<cmd::Command> command;
        if (name == "reverse") {
            command = std::make_shared<cmd::Reverse>(name, trigger, args);
        } else if (name == "overwrite-var") {
            command = std::make_shared<cmd::OverwriteVar>(name, trigger, args);
        } else if (name == "overwrite-group") {
            command = std::make_shared<cmd::OverwriteGroup>(name, trigger, args);
        } else if (name == "rotate") {
            command = std::make_shared<cmd::Rotate>(name, trigger, args);
        } else if (name == "tap-tempo") {
            if (args.size() != 1 || !isAddress(args.at(0)) || bpm_syncs_.count(std::get<Address>(args.at(0))) <= 0) {
                throw std::runtime_error("command #" + std::to_string(num) + " (" + name + ") expects 1 argument; a bpm-sync");
            }

            std::shared_ptr<BPMSync> sync = bpm_syncs_.at(std::get<Address>(args.at(0)));
            command = std::make_shared<cmd::TapTempo>(name, trigger, args, sync);
        } else {
            throw std::runtime_error(
                    "command #" + std::to_string(num) + " has unrecognized command name '" +
                    name + "'");
        }

        command->validate();

        return command;
    }

    void PatchParser::parseVars(const YAML::Node& patch) {
        if (!patch[KEY_VARS]) {
            return;
        }

        for (const auto& kv : patch[KEY_VARS]) {
            const YAML::Node& addr_n = kv.first;
            const YAML::Node& value_n = kv.second;

            Address addr = readAddress(addr_n, false);
            AddressOrValue aov = readAddressOrValue(value_n, true);

            if (isAddress(aov)) {
                store_->set(addr, std::get<Address>(aov));
            } else if (isValue(aov)) {
                store_->set(addr, std::get<Value>(aov));
            }
        }
    }

    void PatchParser::connectCommand(std::shared_ptr<cmd::Command> c) {
        Trigger trigger = c->getTrigger();
        std::string dev = trigger.getFront();
        std::string control = trigger.getBack();

        if (controllers_.count(dev) == 0) {
            throw std::runtime_error("Controller '" + dev + "' not found");
        }

        std::shared_ptr<ValueStore> store = store_;
        // TODO: Why are we doing the lookup when we're given the value?
        controllers_.at(dev)->connect(control, [c, store](Value /*v*/) {
            std::optional<Value> v_opt = store->getValue(c->getTrigger());
            if (v_opt.has_value() && v_opt.value().getBool()) {
                c->run(store);
            }
        });
    }

    void PatchParser::parseCommands(const YAML::Node& patch) {
        if (!patch[KEY_COMMANDS]) {
            return;
        }

        int i = 0;
        for (const auto& settings : patch[KEY_COMMANDS]) {
            i++;

            std::string command_name = requireNode(
                settings,
                KEY_COMMAND,
                "expected command #" + std::to_string(i) + " to have key '" + KEY_COMMAND + "'"
            ).as<std::string>();

            if (settings[KEY_TRIGGER_AND_ARGS]) {
                for (const auto& trig_args : settings[KEY_TRIGGER_AND_ARGS]) {
                    Trigger trigger;
                    std::vector<AddressOrValue> args;
                    int j = 0;
                    for (const auto& arg : trig_args) {
                        if (j++ == 0) {
                            trigger = readTrigger(arg);
                        } else {
                            args.push_back(readAddressOrValue(arg, true));
                        }
                    }

                    std::shared_ptr<cmd::Command> c = loadCommand(i, command_name, trigger, args);
                    connectCommand(c);
                    commands_.push_back(c);
                }
            } else {
                Trigger trigger = requireTrigger(
                    settings,
                    KEY_TRIGGER,
                    "expected command #" + std::to_string(i) + " to have key '" + KEY_TRIGGER + "'"
                );

                std::vector<AddressOrValue> args;
                if (settings[KEY_ARGS]) {
                    for (const auto& arg : settings[KEY_ARGS]) {
                        args.push_back(readAddressOrValue(arg, true));
                    }
                }

                std::shared_ptr<cmd::Command> c = loadCommand(i, command_name, trigger, args);
                connectCommand(c);
                commands_.push_back(c);
            }

        }
    }

    void PatchParser::parseGroups(const YAML::Node& patch) {
        if (!patch[KEY_GROUPS]) {
            return;
        }

        for (const auto& kv : patch[KEY_GROUPS]) {
            const std::string& name = kv.first.as<std::string>();
            const YAML::Node& settings = kv.second;

            auto group = std::make_shared<Group>();

            if (kv.second.IsSequence()) {
                for (const auto el : settings) {
                    group->add(readAddressOrValue(el, true));
                }
            } else {
                const YAML::Node elements = requireNode(
                    settings,
                    KEY_MEMBERS,
                    "expected group '" + name + "' to have field '" + KEY_MEMBERS + "'"
                );

                size_t el_size = 0;
                for (const auto el : elements) {
                    group->add(readAddressOrValue(el, true));
                    el_size++;
                }

                if (settings[KEY_INDEX]) {
                    size_t i = 0;
                    for (const auto idx_name : settings[KEY_INDEX]) {
                        if (i > el_size - 1) {
                            throw std::runtime_error("Index specification for group '" + name + "' is longer than members");
                        }
                        group->setMapping(idx_name.as<std::string>(), i);
                        i++;
                    }
                }
            }

            groups_[kv.first.as<std::string>()] = group;
            store_->set(Address(name), group);
        }
    }

    void PatchParser::parseControllers(const YAML::Node& patch) {
        if (!patch[KEY_CONTROLLERS]) {
            return;
        }

        for (const auto& kv : patch[KEY_CONTROLLERS]) {
            const std::string name = kv.first.as<std::string>();
            const YAML::Node& settings = kv.second;

            if (!settings[KEY_TYPE]) {
                throw std::runtime_error("controller '" + name + "' is missing type");
            }

            const std::string type = settings[KEY_TYPE].as<std::string>();
            if (type == CONTROLLER_TYPE_MIDI) {
                controllers_[name] = loadMidiDevice(name, settings);
            } else if (type == CONTROLLER_TYPE_KEYBOARD) {
                controllers_[name] = std::make_shared<Keyboard>();
            } else if (type == CONTROLLER_TYPE_BPM_SYNC) {
                auto sync = std::make_shared<BPMSync>();
                controllers_[name] = sync;
                bpm_syncs_[Address(name)] = sync;
            } else {
                throw std::runtime_error("unsupported controller type " + type);
            }

            store_->set(Address(name), controllers_.at(name));
        }
    }

    void PatchParser::parseMedia(const YAML::Node& patch) {
        if (!patch[KEY_MEDIAS]) {
            return;
        }

        for (const auto& kv : patch[KEY_MEDIAS]) {
            const std::string name = kv.first.as<std::string>();
            const YAML::Node& settings = kv.second;
            Address addr = Address(name);

            std::string path;
            if (settings.IsMap() && settings[KEY_PATH]) {
                path = settings[KEY_PATH].as<std::string>();
            } else if (settings.IsScalar()) {
                path = settings.as<std::string>();
            } else {
                throw std::runtime_error("media '" + name + "' is missing path");
            }

            std::string type;
            if (settings.IsMap() && settings[KEY_TYPE]) {
                type = settings[KEY_TYPE].as<std::string>();
            } else {
                if (Image::isImage(path)) {
                    type = MEDIA_TYPE_IMAGE;
                } else if (Video::isVideo(path)) {
                    type = MEDIA_TYPE_VIDEO;
                } else {
                    throw std::runtime_error("could not detect type of media '" + name + "' from path");
                }
            }

            if (type == MEDIA_TYPE_IMAGE) {
                images_[name] = loadImage(name, path, settings);
                store_->set(addr, images_[name]);
            } else if (type == MEDIA_TYPE_VIDEO) {
                videos_[name] = loadVideo(name, path, settings);
                store_->set(addr, videos_[name]);
            } else {
                throw std::runtime_error("unsupported media type " + type);
            }
        }
    }

    std::string PatchParser::getBuiltinShader(const std::string& path) {
        return fileutil::join("shaders", path);
    }

    void PatchParser::parseModules(const YAML::Node& patch) {
        if (!patch[KEY_RENDER]) {
            return;
        }

        Resolution res = getResolution();

        for (const auto& settings : patch[KEY_RENDER]) {
            if (!settings[KEY_OUTPUT]) {
                throw std::runtime_error("A render step is missing output");
            }

            const std::string output = settings[KEY_OUTPUT].as<std::string>();

            if (settings[KEY_INPUT]) {
                Address addr = readAddress(settings[KEY_INPUT], true);
                Module::Param param;
                param.value = addr;

                auto mod = std::make_shared<Module>(output, getBuiltinShader("pass.glsl"), res);

                mod->setParam("img0", param);

                modules_.push_back(mod);

                continue;
            }

            if (!settings[KEY_PATH]) {
                throw std::runtime_error("A render step is missing its path");
            }

            const std::string path = settings[KEY_PATH].as<std::string>();

            auto mod = std::make_shared<Module>(output, path, res);

            if (settings[KEY_INPUTS]) {
                for (const auto& input_kv : settings[KEY_INPUTS]) {
                    const std::string key = input_kv.first.as<std::string>();
                    const YAML::Node& value = input_kv.second;

                    Module::Param param;

                    if (value.Type() == YAML::NodeType::Map) {
                        param.value = readAddressOrValue(value[KEY_INPUT], true);

                        if (value[KEY_AMP]) {
                            param.amp = readAddressOrValue(value[KEY_AMP], true);
                        }

                        if (value[KEY_SHIFT]) {
                            param.shift = readAddressOrValue(value[KEY_SHIFT], true);
                        }

                        if (value[KEY_POW]) {
                            param.pow = readAddressOrValue(value[KEY_POW], true);
                        }
                    } else {
                        param.value = readAddressOrValue(value, true);
                    }

                    mod->setParam(key, param);
                }
            }

            int repeat = 1;
            if (settings[KEY_REPEAT]) {
                repeat = settings[KEY_REPEAT].as<int>();
            }

            for (int i = 0; i < repeat; i++) {
                modules_.push_back(mod);
            }
        }

        for (const auto& mod : modules_) {
            store_->setIsMedia(Address(mod->getOutput()), true);
        }
    }

    std::vector<std::shared_ptr<Module>> PatchParser::getModules() {
        return modules_;
    }

    Resolution PatchParser::getResolution() const {
        const YAML::Node patch = YAML::LoadFile(path_);
        Resolution res;

        if (!patch[KEY_RESOLUTION]) {
            res.width = 1280;
            res.height = 960;
            return res;
        }

        const YAML::Node& res_node = patch[KEY_RESOLUTION];

        if (!res_node[KEY_WIDTH] || !res_node[KEY_HEIGHT]) {
            throw std::runtime_error("resolution section missing width or height");
        }

        res.width = res_node[KEY_WIDTH].as<int>();
        res.height = res_node[KEY_HEIGHT].as<int>();

        store_->set(Address("resolution"),
                Value({static_cast<float>(res.width), static_cast<float>(res.height)}));

        return res;
    }

    std::shared_ptr<midi::Device> PatchParser::loadMidiDevice(const std::string& name, const YAML::Node& settings) const {
        if (!settings[KEY_PATH]) {
            throw std::runtime_error("controller '" + name + "' is missing path");
        }

        const std::string path = settings[KEY_PATH].as<std::string>();

        auto dev = std::make_shared<midi::Device>(path);
        dev->start();

        return dev;
    }

    std::shared_ptr<Video> PatchParser::loadVideo(const std::string& name, const std::string& path, const YAML::Node& settings) const {
        bool auto_reset = false;
        if (settings.IsMap() && settings[KEY_RESET]) {
            auto_reset = settings[KEY_RESET].as<bool>();
        }

        Video::Playback pb = Video::Forward;
        if (settings.IsMap() && settings[KEY_PLAYBACK]) {
            const std::string playback = settings[KEY_PLAYBACK].as<std::string>();
            if (playback == "reverse") {
                pb = Video::Reverse;
            } else if (playback == "mirror") {
                pb = Video::Mirror;
            }
        }

        auto vid = std::make_shared<Video>(Address(name), path, auto_reset, pb);

        vid->start();

        return vid;
    }

    std::shared_ptr<Image> PatchParser::loadImage(const std::string& name, const std::string& path, const YAML::Node& /*settings*/) const {
        auto image = std::make_shared<vidrevolt::Image>(Address(name), path);

        image->load();

        return image;
    }
}
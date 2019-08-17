#include "PatchParser.h"

// STL
#include <stdexcept>
#include <optional>

// OpenCV
#include <opencv2/opencv.hpp>

// Ours
#include "Texture.h"
#include "Video.h"
#include "Image.h"
#include "cmd/Overwrite.h"
#include "cmd/Reverse.h"
#include "cmd/Rotate.h"
#include "fileutil.h"

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
#define KEY_SCALE_FILTER "sizeFilter"
#define KEY_CONTROLLERS "controllers"
#define CONTROLLER_TYPE_MIDI "midi"
#define MEDIA_TYPE_IMAGE "image"
#define MEDIA_TYPE_VIDEO "video"
#define KEY_VARS "vars"

namespace frag {
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

    std::map<std::string, std::shared_ptr<midi::Device>> PatchParser::getControllers() {
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
        } else if (name == "overwrite") {
            command = std::make_shared<cmd::Overwrite>(name, trigger, args);
        } else if (name == "rotate") {
            command = std::make_shared<cmd::Rotate>(name, trigger, args);
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

                    commands_.push_back(loadCommand(i, command_name, trigger, args));
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

                commands_.push_back(loadCommand(i, command_name, trigger, args));
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

                int el_size = 0;
                for (const auto el : elements) {
                    group->add(readAddressOrValue(el, true));
                    el_size++;
                }

                if (settings[KEY_INDEX]) {
                    int i = 0;
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
                store_->set(Address(name), controllers_.at(name));
            } else {
                throw std::runtime_error("unsupported controller type " + type);
            }
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

        auto vid = std::make_shared<Video>(path, auto_reset, pb);
        if (settings.IsMap() && settings[KEY_SCALE_FILTER]) {
            const std::string filter = settings[KEY_SCALE_FILTER].as<std::string>();
            if (filter == "nearest") {
                vid->setScaleFilter(GL_NEAREST, GL_NEAREST);
            } else {
                throw std::runtime_error("Invalid scale filter for media '" + name + "'");
            }
        }

        vid->start();

        return vid;
    }

    std::shared_ptr<Image> PatchParser::loadImage(const std::string& /*name*/, const std::string& path, const YAML::Node& /*settings*/) const {
        auto image = std::make_shared<frag::Image>(path);

        image->load();

        return image;
    }
}

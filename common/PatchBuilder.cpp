#include "PatchBuilder.h"

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
#include "KeyboardManager.h"
#include "Patch.h"
#include "Trigger.h"
#include "midi/Device.h"
#include "BPMSync.h"
#include "OscServer.h"

#define KEY_PORT "port"
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
#define CONTROLLER_TYPE_OSC "osc"
#define CONTROLLER_TYPE_KEYBOARD "keyboard"
#define CONTROLLER_TYPE_BPM_SYNC "bpm-sync"
#define MEDIA_TYPE_IMAGE "image"
#define MEDIA_TYPE_VIDEO "video"
#define KEY_VARS "vars"

namespace vidrevolt {
    std::shared_ptr<Patch> PatchBuilder::getPatch() const {
        return patch_;
    }

    void PatchBuilder::build(const std::string& path) {
        const YAML::Node patch = YAML::LoadFile(path);

        buildResolution(patch);
        buildMedia(patch);
        buildGroups(patch);
        buildVars(patch);
        buildControllers(patch);
        buildModules(patch);
        buildCommands(patch);
    }

    void PatchBuilder::addCommand(int num, const std::string& name, Trigger trigger, std::vector<AddressOrValue> args) {
        std::unique_ptr<cmd::Command> c;
        std::string expects_str = "command #" + std::to_string(num) + " (" + name + ") expects ";
        if (name == "reverse") {
            if (args.size() != 1 || !std::holds_alternative<Address>(args.at(0))) {
                throw std::runtime_error(expects_str + " 1 argument; an address to a video");
            }

            auto rev = std::make_unique<cmd::Reverse>();
            rev->target = std::get<Address>(args.at(0));;

            c = std::move(rev);
        } else if (name == "overwrite-var") {
            if (args.size() != 2 || !std::holds_alternative<Address>(args.at(0))) {
                throw std::runtime_error(expects_str + " 2 arguments; the address of a variable and the address or value to set it to");
            }

            auto over = std::make_unique<cmd::OverwriteVar>();
            over->target = std::get<Address>(args.at(0));;
            over->replacement = args.at(1);

            c = std::move(over);
        } else if (name == "overwrite-group") {
            if (args.size() != 2 || !std::holds_alternative<Address>(args.at(0))) {
                throw std::runtime_error(expects_str + " 2 arguments; the address of a group member and the address or value to set it to");
            }

            auto over = std::make_unique<cmd::OverwriteGroup>();
            over->target = std::get<Address>(args.at(0));;
            over->replacement = args.at(1);

            c = std::move(over);
        } else if (name == "rotate") {
            if (args.size() != 1 || !std::holds_alternative<Address>(args.at(0))) {
                throw std::runtime_error(expects_str + " 1 argument; an address to a group");
            }

            auto rot = std::make_unique<cmd::Reverse>();
            rot->target = std::get<Address>(args.at(0));;

            c = std::move(rot);
        } else if (name == "tap-tempo") {
            if (args.size() != 1 || !isAddress(args.at(0))) {
                throw std::runtime_error(expects_str + "1 argument; a bpm-sync");
            }
            auto tap = std::make_unique<cmd::Reverse>();
            tap->target = std::get<Address>(args.at(0));
            c = std::move(tap);
        } else {
            throw std::runtime_error(
                    "command #" + std::to_string(num) + " has unrecognized command name '" +
                    name + "'");
        }

        patch_->addCommand(trigger, std::move(c));
    }

    void PatchBuilder::buildVars(const YAML::Node& patch) {
        if (!patch[KEY_VARS]) {
            return;
        }

        for (const auto& kv : patch[KEY_VARS]) {
            const YAML::Node& addr_n = kv.first;
            const YAML::Node& value_n = kv.second;

            Address addr = readAddress(addr_n, false);
            AddressOrValue aov = readAddressOrValue(value_n, true);

            patch_->setAOV(kv.first.as<std::string>(), readAddressOrValue(kv.second, true));
        }
    }

    void PatchBuilder::buildCommands(const YAML::Node& patch) {
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

                    addCommand(i, command_name, trigger, args);
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

                addCommand(i, command_name, trigger, args);
            }

        }
    }

    void PatchBuilder::buildGroups(const YAML::Node& patch) {
        if (!patch[KEY_GROUPS]) {
            return;
        }

        for (const auto& kv : patch[KEY_GROUPS]) {
            const std::string& name = kv.first.as<std::string>();
            const YAML::Node& settings = kv.second;

            auto group = std::make_unique<Group>();

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

            patch_->setGroup(kv.first.as<std::string>(), std::move(group));
        }
    }

    void PatchBuilder::buildControllers(const YAML::Node& patch) {
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
                addMidiDevice(name, settings);
            } else if (type == CONTROLLER_TYPE_OSC) {
                addOSCServer(name, settings);
            } else if (type == CONTROLLER_TYPE_KEYBOARD) {
                patch_->setController(name, KeyboardManager::makeKeyboard());
            } else if (type == CONTROLLER_TYPE_BPM_SYNC) {
                patch_->setController(name, std::make_unique<BPMSync>());
            } else {
                throw std::runtime_error("unsupported controller type " + type);
            }
        }
    }

    void PatchBuilder::buildMedia(const YAML::Node& patch) {
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
                if (fileutil::hasImageExt(path)) {
                    type = MEDIA_TYPE_IMAGE;
                } else if (fileutil::hasVideoExt(path)) {
                    type = MEDIA_TYPE_VIDEO;
                } else {
                    throw std::runtime_error("could not detect type of media '" + name + "' from path");
                }
            }

            if (type == MEDIA_TYPE_IMAGE) {
                addImage(name, path, settings);
            } else if (type == MEDIA_TYPE_VIDEO) {
                addVideo(name, path, settings);
            } else {
                throw std::runtime_error("unsupported media type " + type);
            }
        }
    }

    std::string PatchBuilder::getBuiltinShader(const std::string& path) {
        return fileutil::join("shaders", path);
    }

    void PatchBuilder::buildModules(const YAML::Node& patch) {
        if (!patch[KEY_RENDER]) {
            return;
        }

        Resolution res = patch_->getResolution();

        for (const auto& settings : patch[KEY_RENDER]) {
            if (!settings[KEY_OUTPUT]) {
                throw std::runtime_error("A render step is missing output");
            }

            const std::string output = settings[KEY_OUTPUT].as<std::string>();

            if (settings[KEY_INPUT]) {
                Address addr = readAddress(settings[KEY_INPUT], true);
                Module::Param param;
                param.value = addr;

                auto mod = std::make_unique<Module>(output, getBuiltinShader("pass.glsl"), res);

                mod->setParam("img0", param);

                patch_->addModule(std::move(mod));

                continue;
            }

            if (!settings[KEY_PATH]) {
                throw std::runtime_error("A render step is missing its path");
            }

            const std::string path = settings[KEY_PATH].as<std::string>();

            auto mod = std::make_unique<Module>(output, path, res);

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

            patch_->addModule(std::move(mod));
        }
    }

    void PatchBuilder::buildResolution(const YAML::Node& patch) {
        Resolution res;

        if (!patch[KEY_RESOLUTION]) {
            res.width = 1280;
            res.height = 960;
        } else {
            const YAML::Node& res_node = patch[KEY_RESOLUTION];

            if (!res_node[KEY_WIDTH] || !res_node[KEY_HEIGHT]) {
                throw std::runtime_error("resolution section missing width or height");
            }

            res.width = res_node[KEY_WIDTH].as<int>();
            res.height = res_node[KEY_HEIGHT].as<int>();
        }

        patch_->setResolution(res);
    }

    void PatchBuilder::addOSCServer(const std::string& name, const YAML::Node& settings) {
        if (!settings[KEY_PORT]) {
            throw std::runtime_error("controller '" + name + "' is missing port");
        }

        int port = settings[KEY_PORT].as<int>();

        if (!settings[KEY_PATH]) {
            throw std::runtime_error("controller '" + name + "' is missing path");
        }

        const std::string path = settings[KEY_PATH].as<std::string>();

        auto osc = std::make_unique<OscServer>(port, path);
        osc->start();

        patch_->setController(name, std::move(osc));
    }


    void PatchBuilder::addMidiDevice(const std::string& name, const YAML::Node& settings) {
        if (!settings[KEY_PATH]) {
            throw std::runtime_error("controller '" + name + "' is missing path");
        }

        const std::string path = settings[KEY_PATH].as<std::string>();

        auto dev = std::make_unique<midi::Device>(path);
        dev->start();

        patch_->setController(name, std::move(dev));
    }

    void PatchBuilder::addVideo(const std::string& name, const std::string& path, const YAML::Node& settings) {
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

        auto vid = std::make_unique<Video>(path, auto_reset, pb);

        vid->start();

        patch_->setVideo(name, std::move(vid));
    }

    void PatchBuilder::addImage(const std::string& name, const std::string& path, const YAML::Node& /*settings*/) {
        auto image = std::make_unique<vidrevolt::Image>(path);

        image->load();
        patch_->setImage(name, std::move(image));
    }

    AddressOrValue PatchBuilder::readAddressOrValue(const YAML::Node& node, bool parse_swiz) {
        if (node.IsSequence()) {
            std::vector<float> v = {};

            for (const auto& el : node) {
                v.push_back(el.as<float>());
            }

            return Value(v);
        }

        const std::string str = node.as<std::string>();

        bool b;
        float f;
        if (YAML::convert<bool>::decode(node, b) && str != "n" && str != "y") {
            return Value(b);
        }

        if (YAML::convert<float>::decode(node, f)) {
            return Value(f);
        }

        return readAddress(node, parse_swiz);
    }

    Address PatchBuilder::readAddress(const YAML::Node& node, bool parse_swiz) {
        std::string str = node.as<std::string>();
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream iss(str);

        while (std::getline(iss, token, '.')) {
            tokens.push_back(token);
        }

        std::string swiz;
        if (parse_swiz) {
            if (tokens.size() > 1) {
                std::regex nonswiz_re("[^xyzwrgb]");
                if (!std::regex_search(tokens.back(), nonswiz_re)) {
                    Address addr = Address(tokens);
                    Address no_swiz_addr = addr.withoutBack();

                    if (!patch_->isGroup(no_swiz_addr)) {
                        swiz = tokens.back();
                        tokens.pop_back();
                    }
                }
            }
        }

        Address addr(tokens);
        addr.setSwiz(swiz);

        return addr;
    }

    const YAML::Node PatchBuilder::requireNode(const YAML::Node& parent, const std::string& key, const std::string& err) {
        if (!parent[key]) {
            throw std::runtime_error(err);
        }

        return parent[key];
    }


    Address PatchBuilder::requireAddress(const YAML::Node& parent, const std::string& key, const std::string& err, bool parse_swiz) {
        return readAddress(requireNode(parent, key, err), parse_swiz);
    }

    Trigger PatchBuilder::readTrigger(const YAML::Node& node) {
        /*Trigger trig;
        if (node.IsSequence()) {
            for (const auto& addr_node : node) {
                trig.push_back(readAddress(addr_node, false));
            }
        } else {
            trig.push_back(readAddress(node, false));
        }

        return trig;
        */
        return readAddress(node, false);
    }

    Trigger PatchBuilder::requireTrigger(
            const YAML::Node& parent,
            const std::string& key,
            const std::string& err) {
        if (!parent[key]) {
            throw std::runtime_error(err);
        }

        return readTrigger(parent[key]);
    }
}

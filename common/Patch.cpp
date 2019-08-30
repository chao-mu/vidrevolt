#include "Patch.h"

// TODO: Rename Address to Reference. Everything simplifies with that reframing

#include "cmd/OverwriteGroup.h"
#include "cmd/OverwriteVar.h"
#include "cmd/Reverse.h"
#include "cmd/Rotate.h"
#include "cmd/TapTempo.h"

namespace vidrevolt {

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

    void Patch::visitReferable(const Address& addr, std::function<void(Referable)> f) const {
        visitReferable(addr, f, Address());
    }

    void Patch::visitReferable(const Address& addr, std::function<void(Referable)> f, const Address& tail) const {
        if (addr.getDepth() == 1 && tail.getDepth() == 0) {
            std::string key = addr.str();

            if (videos_.count(key) > 0) {
                f(videos_.at(key).get());
            } else if (images_.count(key) > 0) {
                f(images_.at(key).get());
            } else if (groups_.count(key) > 0) {
                f(groups_.at(key).get());
            } else if (aovs_.count(key) > 0) {
                auto& aov = aovs_.at(key);
                if (std::holds_alternative<Value>(aov)) {
                    f(std::get<Value>(aov));
                } else if (std::holds_alternative<Address>(aov)) {
                    visitReferable(std::get<Address>(aov), f);
                }
            } else if (module_resolutions_.count(key) > 0) {
                 f(key);
            } else {
                throw std::runtime_error("Invalid address " + addr.str());
            }
        } else if (tail.str() == "resolution") {
            Resolution res;
            visitReferable(addr, [&res, addr, this](Referable r) {
                if (std::holds_alternative<Media*>(r)) {
                    res = std::get<Media*>(r)->getResolution();
                } else if (std::holds_alternative<ModuleOutputLabel>(r)) {
                    res = module_resolutions_.at(std::get<ModuleOutputLabel>(r));
                } else {
                    throw std::runtime_error("Expected address '" + addr.str() + "' to refer to media");
                }
            });

            f(Value({static_cast<float>(res.width), static_cast<float>(res.height)}));
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
                    f(std::get<Value>(*aov_opt));
                } else if (std::holds_alternative<Address>(*aov_opt)) {
                    visitReferable(std::get<Address>(*aov_opt), f);
                }
            } else if (controllers_.count(head) > 0) {
                f(controllers_.at(head)->getValue(back));
            } else {
                throw std::runtime_error("Invalid address " + head + "." + back);
            }
        } else if (addr.getDepth() == 0) {
            f(tail.str());
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
    }

    const std::vector<std::unique_ptr<Module>>& Patch::getModules() const {
        return modules_;
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

    void Patch::setGroup(const std::string& key, std::unique_ptr<Group> group) {
        groups_[key] = std::move(group);
    }

    void Patch::addModule(std::unique_ptr<Module> mod) {
        module_resolutions_[mod->getOutput()] = mod->getResolution();
        modules_.push_back(std::move(mod));
    }

    bool Patch::isMedia(const Address& addr) const {
        bool is = false;
        visitReferable(addr, [&is](Referable r) {
            if (std::holds_alternative<Media*>(r) || std::holds_alternative<ModuleOutputLabel>(r)) {
                is = true;
            }
        });

        return is;
    }

    bool Patch::isGroup(const Address& addr) const {
        bool is = false;
        visitReferable(addr, [&is](Referable r) {
            if (std::holds_alternative<Group*>(r)) {
                is = true;
            }
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
            visitReferable(reverse->target, [&reverse](Referable r) {
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

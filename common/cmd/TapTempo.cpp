#include "TapTempo.h"

namespace vidrevolt {
    namespace cmd {
        TapTempo::TapTempo(const std::string& name, const Trigger& trigger, std::vector<AddressOrValue> args, std::shared_ptr<BPMSync> sync) : Command(name, trigger, args), sync_(sync) {}

        void TapTempo::run(std::shared_ptr<ValueStore> /*store*/) const {
            sync_->tap();
        }
    }
}


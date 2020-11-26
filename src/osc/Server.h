#ifndef VIDREVOLT_OSC_SERVER_H_
#define VIDREVOLT_OSC_SERVER_H_

// STL
#include <string>

// LibLo (OSC)
#include <lo/lo.h>
#include <lo/lo_cpp.h>

// Ours
#include "../Controller.h"

namespace vidrevolt::osc {
    class Server : public Controller {
        public:
            Server(int port, const std::string& path);

            void start();

        private:
            int port_;
            std::string path_;
            lo::ServerThread server_;

    };
}
#endif

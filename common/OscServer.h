#ifndef VIDREVOLT_OSCSERVER_H_
#define VIDREVOLT_OSCSERVER_H_

// STL
#include <string>

// LibLo (OSC)
#include <lo/lo.h>
#include <lo/lo_cpp.h>

// Ours
#include "Controller.h"

namespace vidrevolt {
    class OscServer : public Controller {
        public:
            OscServer(int port, const std::string& path);

            void start();

        private:
            int port_;
            std::string path_;
            lo::ServerThread server_;
    };
}

#endif


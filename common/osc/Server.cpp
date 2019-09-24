#include "Server.h"

// STL
#include "fstream"

// Ours
#include "../Value.h"

namespace vidrevolt {
    namespace osc {
        Server::Server(int port, const std::string& path) : port_(port), path_(path), server_(port) {}

        void Server::start() {
            if (!server_.is_valid()) {
                throw std::runtime_error("Failed to start osc server on port " + std::to_string(port_));
            }


            std::ifstream controls_file(path_);
            for (std::string ctrl; getline(controls_file, ctrl);) {
                addControlName(ctrl);
                // We pass ctrl by copy to the lambda because add_method appears to
                // modify its contents at some point later.
                server_.add_method(ctrl, "f", [ctrl, this](lo_arg **argv, int argc) {
                    if (argc >= 1) {
                        addValue(ctrl, Value(argv[0]->f));
                    }
                });
            }

            server_.start();
        }
    }
}

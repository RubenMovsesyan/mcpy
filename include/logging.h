#ifndef LOGGING
#define LOGGING

#include <iostream>

namespace {
    void log(const std::string &message) {
        std::cout << message << std::endl;
    }

    void exitWithError(const std::string &error_message) {
        log("ERROR: " + error_message);
        exit(1);
    }
}

#endif
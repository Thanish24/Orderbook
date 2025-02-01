#pragma once

#include <string>

std::string makeRed(std::string inp) {
    return "\033[31m" + inp + "\033[0m";
}

std::string makeGreen(std::string inp) {
    return "\033[32m" + inp + "\033[0m";
}
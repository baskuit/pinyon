#pragma once

#include "tree/node.hh"

struct SearchSession {
    prng& device;
    SearchSession (prng& device) : device(device) {}
};
#include "AutoGroupState.hpp"

namespace ta {

bool& autoGroupEnabled() {
    static bool enabled = false;
    return enabled;
}

}
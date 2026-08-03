#include "FormulaLibrary.hpp"

namespace ta {

const std::vector<Formula>& formulaLibrary() {
    static const std::vector<Formula> table = {
        {
            "flip_3d_x",
            "3D Flip (X axis)",
            "Makes the selected group appear to flip like a card/door using synced Scale + Move triggers."
        },
        {
            "parallax_layers",
            "Auto Parallax (up to 4 layers)",
            "Generates Advanced Follow Triggers with decreasing modifiers for tagged background layers."
        },
        {
            "beat_pulse",
            "Manual Beat Pulse",
            "Places a tunable Pulse Trigger template on the target group (manual timing for now)."
        },
    };
    return table;
}

const Formula* findFormula(std::string const& id) {
    for (auto& f : formulaLibrary()) if (f.id == id) return &f;
    return nullptr;
}

} // namespace ta
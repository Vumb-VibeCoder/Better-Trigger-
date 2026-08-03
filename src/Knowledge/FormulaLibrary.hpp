#pragma once
// ============================================================================
// FORMULA LIBRARY (explain-only)
// ----------------------------------------------------------------------------
// Descriptions of known trigger combos, used purely for lookup text in the
// Trigger Explainer. This mod no longer generates triggers, so this table
// only carries id/displayName/description - no params, no generate function.
// ============================================================================

#include <string>
#include <vector>

namespace ta {

struct Formula {
    std::string id;
    std::string displayName;
    std::string description;
};

// All registered formulas.
std::vector<Formula> const& formulaLibrary();
const Formula* findFormula(std::string const& id);

} // namespace ta
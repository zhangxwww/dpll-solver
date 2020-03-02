//
// DPLL algorithm.
//

#include "DPLL.h"

DPLL::DPLL(const formula& phi) 
    :phi(phi), 
     literalInfoMap(phi.num_variable, nullptr),
     clauseValue(phi.clauses.size(), UNDEF),
     unassignedCount(phi.clauses.size(), 0) {}

bool DPLL::check_sat() {
    // TODO: your code here, or in the header file
    init();
    return dfs();
}

model DPLL::get_model() {
    // TODO: your code here, or in the header file
    return model();
}

void DPLL::init() {
    int n = phi.num_variable;
    for (int i = 1; i <= n; ++i) {
        AtomInfo info = AtomInfo();
        info.index = i;
        unassignedAtoms.push_back(info);
    }
    auto cs = phi.clauses;
    int n_clause = cs.size();
    for (int i = 0; i < n_clause; ++i) {
        auto c = cs[i];
        int n_literal = c.size();
        for (int j = 0; j < n_literal; ++j) {
            literal l = c[j];
            Sign s = POSITIVE(l) ? POS : NEG;
            LiteralInfo info = LiteralInfo();
            info.sign = s;
            info.clause_index = i;
            int atom = VAR(l);
            if (literalInfoMap[atom] == nullptr) {
                literalInfoMap[atom] = new LiteralInfoList();
            }
            literalInfoMap[atom]->push_back(info);
        }
        unassignedCount[i] = n_literal;
        if (n_literal == 1) {
            clauseValue[i] = UNIT;
        }
    }
}

bool DPLL::dfs() {
    // TODO
    return false;
}

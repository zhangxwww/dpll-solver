//
// DPLL algorithm.
//

#include "DPLL.h"

DPLL::DPLL(const formula& phi) 
    :phi(phi), 
     literalInfoMap(phi.num_variable + 1, nullptr),
     clauseValue(phi.clauses.size(), UNDEF),
     unassignedCount(phi.clauses.size(), 0),
     usedAtom(phi.num_variable + 1, false) {}

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
    auto & cs = phi.clauses;
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
    decideChain.push_back(0);
}

bool DPLL::dfs() {
    while (true) {
        while (false) {
            // propagate
        }
        decide();
        if (conflict()) {
            if (hasDecision()) {
                if (!use_backjump) {
                    //backtrack();
                }
                else {
                    //backjump();
                }
            }
            else {
                return false;
            }
        }
        else if (sat()) {
            // generate model
            return true;
        }
    }
    return false;
}

bool DPLL::sat() const {
    for (auto iter = clauseValue.begin(); 
        iter != clauseValue.end(); 
        ++iter) {
        if (*iter != TRUE) {
            return false;
        }
    }
    return true;
}

bool DPLL::conflict() {
    int n = clauseValue.size();
    for (int i = 0; i < n; ++i) {
        if (clauseValue[i] == FALSE) {
            conflictClause = i;
            return true;
        }
    }
    return false;
}

void DPLL::decide() {
    int atom = findNextUnusedAtom();
    decideLiteral(atom);
}

bool DPLL::hasDecision() const {
    return decidedCount != 0;
}

int DPLL::findNextUnusedAtom() {
    if (backtrackResult != 0) {
        int res = backtrackResult;
        backtrackResult = 0;
        return res;
    }
    int last = *decideChain.end();
    if (last == 0) {
        return 1;
    }
    while (true) {
        last = last > 0 ? -last : -last + 1;
        if (!usedAtom[VAR(last)]) {
            return last;
        }
    }
    return 0;
}

void DPLL::decideLiteral(int liter) {
    int idx = VAR(liter);
    bool sign = POSITIVE(liter);

    updateInterpretations(idx, sign);

    LiteralInfoList* infoList = literalInfoMap[idx];
    for (auto iter = infoList->begin();
        iter != infoList->end();
        ++iter) {
        Sign literalSign = iter->sign;
        int cidx = iter->clause_index;

        updateClauseValue(liter, cidx, literalSign);
    }

    usedAtom[idx] = true;
    decideChain.push_back(liter);
    ++decidedCount;
}

void DPLL::updateInterpretations(int idx, bool sign) {
    AtomInfo info;
    info.index = idx;
    info.is_decide = true;
    info.value = sign ? TRUE : FALSE;
    interpretations.push_back(info);
}

void DPLL::updateClauseValue(int liter, int cidx, Sign literalSign) {
    bool sign = POSITIVE(liter);
    if (clauseValue[cidx] == UNDEF) {
        if (sameSign(literalSign, sign)) {
            clauseValue[cidx] = TRUE;
        }
    }
    else if (clauseValue[cidx] == UNIT) {
        if (sameSign(literalSign, sign)) {
            clauseValue[cidx] = TRUE;
        }
        else {
            clauseValue[cidx] = FALSE;
        }
    }
    --unassignedCount[cidx];
    if (clauseValue[cidx] == UNDEF
        && unassignedCount[cidx] == 1) {
        clauseValue[cidx] = UNIT;
    }
}

bool DPLL::sameSign(Sign s, bool pos) {
    bool cond1 = s == POS;
    bool cond2 = pos;
    return cond1 == cond2;
}

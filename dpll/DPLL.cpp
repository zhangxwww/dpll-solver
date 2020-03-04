//
// DPLL algorithm.
//

#include "DPLL.h"

DPLL::DPLL(const formula& phi) 
    :phi(phi), 
     interpretations(phi.num_variable + 1, UNDEF),
     literalInfoMap(phi.num_variable + 1, nullptr),
     clauseValue(phi.clauses.size(), UNDEF),
     unassignedCount(phi.clauses.size(), 0) {}

DPLL::~DPLL() {
    int n = phi.num_variable + 1;
    for (int i = 0; i < n; ++i) {
        if (literalInfoMap[i] != nullptr) {
            delete literalInfoMap[i];
        }
    }
}

bool DPLL::check_sat() {
    // TODO: your code here, or in the header file
    init();
    return dfs();
}

model DPLL::get_model() {
    // TODO: your code here, or in the header file
    return finalModel;
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
        while (propagate());
        decide();
        if (conflict()) {
            if (hasDecision()) {
                if (!use_backjump) { backtrack(); }
                else { /*backjump();*/ }
            }
            else { return false; }
        }
        if (sat()) {
            generateModel();
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

bool DPLL::propagate() {
    int n_clause = phi.clauses.size();
    int unitClauseIndex = -1;
    bool found = false;
    for (int i = 0; i < n_clause; ++i) {
        if (clauseValue[i] == UNIT) { 
            unitClauseIndex = i;
            found = true;
            break;
        }
    }
    if (!found) { return false; }
    clause c = phi.clauses[unitClauseIndex];
    for (auto liter = c.begin();
        liter != c.end();
        ++liter) {
        int atom = VAR(*liter);
        bool sign = POSITIVE(*liter);
        if (interpretations[atom] == UNDEF) {
            updateInterpretations(*liter);
        }
    }
    return true;
}

void DPLL::decide() {
    int atom = findNextUnusedAtom();
    if (atom == phi.num_variable + 1) { return; }
    decideAtom(atom);
}

bool DPLL::hasDecision() const {
    return decideChain.size() > 1;
}

void DPLL::backtrack() {
    bool should_break = false;
    int n_clause = phi.clauses.size();
    std::vector<bool> markedClause(n_clause, false);
    std::list<int> relatedClauses;
    while (decideChain.size() > 1) {
        auto last = --runningChain.end();
        int aidx = last->index;
        bool unassign = false;
        if (!last->is_decide) {
            // pop all p given by unit propagation
            runningChain.pop_back();
            interpretations[aidx] = UNDEF;
            unassign = true;
        }
        else {
            last->is_decide = false;
            last->value = FALSE;
            decideChain.pop_back();
            should_break = true;
            interpretations[aidx] = FALSE;
        }
        LiteralInfoList* infoList = literalInfoMap[aidx];
        for (auto iter = infoList->begin();
            iter != infoList->end();
            ++iter) {
            int cidx = iter->clause_index;
            if (unassign) { ++unassignedCount[cidx]; }
            if (!markedClause[cidx]) {
                markedClause[cidx] = true;
                relatedClauses.push_back(cidx);
            }
        }
        if (should_break) { break; }
    }
    updateClauseValue(relatedClauses);
}

void DPLL::generateModel() {
    int n = phi.num_variable;
    for (int i = 1; i <= n; ++i) {
        finalModel[i] = interpretations[i] == TRUE;
    }
}

int DPLL::findNextUnusedAtom() const {
    int last = *(--decideChain.end());
    int n_atoms = phi.num_variable;
    while (last++ < n_atoms
        && interpretations[last] != UNDEF);
    return last;
}

void DPLL::decideAtom(int atom) {
    updateInterpretations(atom);
    decideChain.push_back(atom);
}

void DPLL::updateInterpretations(int liter) {
    int idx = VAR(liter);
    bool sign = POSITIVE(liter);
    LiteralInfoList* infoList = literalInfoMap[idx];
    for (auto iter = infoList->begin();
        iter != infoList->end();
        ++iter) {
        Sign literalSign = iter->sign;
        int cidx = iter->clause_index;

        updateClauseValue(liter, cidx, literalSign);
    }
    interpretations[idx] = sign ? TRUE : FALSE;
    AtomInfo info;
    info.index = idx;
    info.is_decide = true;
    info.value = sign ? TRUE : FALSE;
    runningChain.push_back(info);
}

void DPLL::updateClauseValue(int liter, int cidx, Sign literalSign) {
    bool sign = POSITIVE(liter);
    if (clauseValue[cidx] == UNDEF) {
        if (sameSign(literalSign, sign)) {
            clauseValue[cidx] = TRUE;
        }
    }
    else if (clauseValue[cidx] == UNIT) {
        clauseValue[cidx] = sameSign(literalSign, sign) ?
            TRUE : FALSE;
    }
    --unassignedCount[cidx];
    if (clauseValue[cidx] == UNDEF
        && unassignedCount[cidx] == 1) {
        clauseValue[cidx] = UNIT;
    }
}

void DPLL::updateClauseValue(const std::list<int>& cs) {
    for (auto c = cs.begin();
        c != cs.end();
        ++c) {

        clauseValue[*c] = evalClause(*c);
    }
}

TrueValue DPLL::evalClause(int cidx) const {
    clause cls = phi.clauses[cidx];
    int n_undef = 0;
    for (auto l = cls.begin();
        l != cls.end();
        ++l) {

        int atom = VAR(*l);
        TrueValue inter = interpretations[atom];
        if (inter == TRUE && POSITIVE(*l)) {
            return TRUE;
        }
        else if (inter == FALSE && NEGATIVE(*l)) {
            return TRUE;
        }
        else if (inter == UNDEF) {
            ++n_undef;
            if (n_undef == 2) {
                return UNDEF;
            }
        }
    }
    return n_undef == 1 ? UNIT : FALSE;
}

bool DPLL::sameSign(Sign s, bool pos) {
    bool cond1 = s == POS;
    bool cond2 = pos;
    return cond1 == cond2;
}

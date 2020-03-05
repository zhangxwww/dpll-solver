//
// DPLL algorithm.
//

#include "DPLL.h"

DPLL::DPLL(const formula& phi) 
    :phi(phi), 
     interpretations(phi.num_variable + 1, UNDEF),
     literalInfoMap(phi.num_variable + 1, nullptr),
     clauseValue(phi.clauses.size(), UNDEF),
     unassignedCount(phi.clauses.size(), 0),
     conflictGraph(phi.num_variable + 1) {}

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
                else { backjump(); }
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
        if (*iter != TRUE) { return false; }
    }
    return true;
}

bool DPLL::conflict() const {
    if (conflictNodeIndex != 0) { return true; }
    int n = clauseValue.size();
    for (int i = 0; i < n; ++i) {
        if (clauseValue[i] == FALSE) {
            return true;
        }
    }
    return false;
}

bool DPLL::propagate() {
    int n_clause = phi.clauses.size();
    bool success = false;
    for (int i = 0; i < n_clause; ++i) {
        if (clauseValue[i] == UNIT) { 
            success = propagateClause(i);
            break;
        }
    }
    return success;
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
        for (auto iter : *infoList) {
            int cidx = iter.clause_index;
            if (unassign) { ++unassignedCount[cidx]; }
            if (!markedClause[cidx]) {
                markedClause[cidx] = true;
                relatedClauses.push_back(cidx);
            }
        }
        if (should_break) { break; }
    }
    updateClauseValue(relatedClauses);
    conflictNodeIndex = 0;
    conflictClause = -1;
}

void DPLL::backjump() {
    std::set<int> conflictDecisions;
    conflictDecisions.insert(
        conflictGraph[conflictNodeIndex].decideNodes.begin(),
        conflictGraph[conflictNodeIndex].decideNodes.end()
    );
    for (int liter : phi.clauses[conflictClause]) {
        if (liter != conflictNodeIndex) {
            conflictDecisions.insert(
                conflictGraph[liter].decideNodes.begin(),
                conflictGraph[liter].decideNodes.end()
            );
        }
    }

    addNewClause(conflictDecisions);

    unsigned int minLevel = decideChain.size();
    for (int decide : conflictDecisions) {
        if (conflictGraph[decide].decideLevel < minLevel) {
            minLevel = conflictGraph[decide].decideLevel;
        }
    }

    int n_clause = phi.clauses.size();
    std::vector<bool> markedClause(n_clause, false);
    std::list<int> relatedClauses;

    int lastDecide = 0;
    while (decideChain.size() >= minLevel) {
        auto last = --decideChain.end();
        if (decideChain.size() == minLevel) {
            lastDecide = *last;
        }
        decideChain.pop_back();
    }
    bool should_break = false;;
    while (true) {
        auto last = --runningChain.end();
        int aidx = last->index;
        bool unassign = false;
        if (last->index != lastDecide) {
            runningChain.pop_back();
            interpretations[aidx] = UNDEF;
            unassign = true;
        }
        else {
            last->is_decide = false;
            last->value = FALSE;
            should_break = true;
            interpretations[aidx] = FALSE;
        }
        LiteralInfoList* infoList = literalInfoMap[aidx];
        for (auto info : *infoList) {
            int cidx = info.clause_index;
            if (unassign) { ++unassignedCount[cidx]; }
            if (!markedClause[cidx]) {
                markedClause[cidx] = true;
                relatedClauses.push_back(cidx);
            }
        }
        if (should_break) { break; }
    }
    updateClauseValue(relatedClauses);
    destroyGraph(conflictDecisions, lastDecide);
    conflictNodeIndex = 0;
    conflictClause = -1;
}

void DPLL::generateModel() {
    int n = phi.num_variable;
    for (int i = 1; i <= n; ++i) {
        finalModel[i] = interpretations[i] == TRUE;
    }
}

bool DPLL::propagateClause(int cidx) {
    clause c = phi.clauses[cidx];
    bool success = false;
    int propagated = 0;
    for (auto liter : c) {
        int atom = VAR(liter);
        if (interpretations[atom] == UNDEF) {
            propagated = atom;
            success = updateInterpretations(liter, false);
        }
    }
    if (use_backjump) {
        for (auto liter : c) {
            if (liter == propagated) { continue; }
            conflictGraph[liter].propagations.insert(propagated);
            conflictGraph[propagated].decideNodes.insert(
                conflictGraph[liter].decideNodes.begin(),
                conflictGraph[liter].decideNodes.end()
            );
            conflictGraph[propagated].parents.insert(liter);
        }
    }
    return success;
}

int DPLL::findNextUnusedAtom() const {
    int last = *(--decideChain.end());
    int n_atoms = phi.num_variable;
    while (last++ < n_atoms
        && interpretations[last] != UNDEF);
    return last;
}

void DPLL::decideAtom(int atom) {
    updateInterpretations(atom, true);
    decideChain.push_back(atom);
    if (use_backjump) {
        conflictGraph[atom].isDecide = true;
        conflictGraph[atom].decideLevel = decideChain.size();
        conflictGraph[atom].decideNodes.insert(atom);
    }
}

bool DPLL::updateInterpretations(int liter, bool is_decide) {
    int idx = VAR(liter);
    bool sign = POSITIVE(liter);
    interpretations[idx] = sign ? TRUE : FALSE;
    LiteralInfoList* infoList = literalInfoMap[idx];
    bool updateResult = true;
    for (auto iter = infoList->begin();
        iter != infoList->end();
        ++iter) {
        Sign literalSign = iter->sign;
        int cidx = iter->clause_index;

        updateResult = updateClauseValue(liter, cidx, literalSign)
            && updateResult;
    }
    AtomInfo info;
    info.index = idx;
    info.is_decide = is_decide;
    info.value = sign ? TRUE : FALSE;
    runningChain.push_back(info);
    return updateResult;
}

bool DPLL::updateClauseValue(int liter, int cidx, Sign literalSign) {
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
    bool updateResult = clauseValue[cidx] != FALSE;
    if (!updateResult) {
        conflictNodeIndex = VAR(liter);
        conflictClause = cidx;
    }
    return updateResult;
}

bool DPLL::updateClauseValue(const std::list<int>& cs) {
    for (auto c = cs.begin();
        c != cs.end();
        ++c) {

        clauseValue[*c] = evalClause(*c);
    }
    return true;
}

TrueValue DPLL::evalClause(int cidx) const {
    clause cls = phi.clauses[cidx];
    int n_undef = 0;
    for (auto l = cls.begin();
        l != cls.end();
        ++l) {

        int atom = VAR(*l);
        TrueValue inter = interpretations[atom];
        if (inter == TRUE && POSITIVE(*l)) { return TRUE; }
        else if (inter == FALSE && NEGATIVE(*l)) { return TRUE; }
        else if (inter == UNDEF) { ++n_undef; }
    }
    if (n_undef == 1) { return UNIT; }
    else if (n_undef > 1) { return UNDEF; }
    else { return FALSE; }
}

void DPLL::addNewClause(const std::set<int>& conflicts) {
    int n_liter = conflicts.size();
    clause c(n_liter);
    int cidx = phi.clauses.size();
    for (int atom : conflicts) {
        c.push_back(-atom);
        LiteralInfo info;
        info.sign = NEG;
        info.clause_index = cidx;
        literalInfoMap[atom]->push_back(info);
        clauseValue.push_back(n_liter == 1 ? UNIT : UNDEF);
        unassignedCount.push_back(n_liter);
        GraphNode node;
        conflictGraph.push_back(node);
    }
}

void DPLL::destroyGraph(const std::set<int>& conflicts) {
    for (int atom : conflicts) {
        conflictGraph[atom].isDecide = false;
        conflictGraph[atom].decideLevel = 0xffffffff;
        for (int p : conflictGraph[atom].parents) {
            if (conflictGraph[p].propagations.find(atom)
                != conflictGraph[p].propagations.end()) {
                conflictGraph[p].propagations.erase(atom);
            }
        }
        conflictGraph[atom].parents.clear();
        conflictGraph[atom].decideNodes.clear();
        destroyGraph(conflictGraph[atom].propagations, -1);
        conflictGraph[atom].propagations.clear();
    }
}

bool DPLL::sameSign(Sign s, bool pos) {
    bool cond1 = s == POS;
    bool cond2 = pos;
    return cond1 == cond2;
}

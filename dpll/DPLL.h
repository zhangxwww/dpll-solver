//
// DPLL algorithm.
//

#ifndef DPLL_DPLL_H
#define DPLL_DPLL_H

#include <list>

#include "common.h"

class DPLL {
public:
    /**
     * Constructor.
     *
     * @param phi the formula to be checked
     * @note Please DON'T CHANGE this signature because the grading script will directly call this function!
     */
    DPLL(const formula& phi);

    /**
     * Check if the formula is satisfiable.
     *
     * @return true if satisfiable, and false if unsatisfiable
     * @note Please DON'T CHANGE this signature because the grading script will directly call this function!
     */
    bool check_sat();

    /**
     * Get a satisfying model (interpretation) of the formula, the model must be *complete*, that is,
     * it must assign every variable a truth value.
     * This function will be called if and only if `check_sat()` returns true.
     *
     * @return an arbitrary (if there exist many) satisfying model
     * @note Please DON'T CHANGE this signature because the grading script will directly call this function!
     */
    model get_model();

private:
    void init();
    bool dfs();

    formula phi;

    struct AtomInfo {
        int index = 0;
        bool is_decide = false;
        TrueValue value = UNDEF;
    };
    typedef std::list<AtomInfo> AtomList;

    AtomList assignedAtoms;
    AtomList unassignedAtoms;

    struct LiteralInfo {
        Sign sign = POS;
        int clause_index = 0;
    };
    typedef std::list<LiteralInfo> LiteralInfoList;

    // atom(pos int) :-> index of releted clause 
    std::vector<LiteralInfoList*> literalInfoMap;

    // index of clause (int) :-> TrueValue
    std::vector<TrueValue> clauseValue;

    // index of clause :-> count of 
    //     unassigned literals in the clause
    std::vector<int> unassignedCount;
};




#endif //DPLL_DPLL_H

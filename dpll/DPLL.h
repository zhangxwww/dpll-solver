//
// DPLL algorithm.
//

#ifndef DPLL_DPLL_H
#define DPLL_DPLL_H

#include <list>
#include <set>

#include "common.h"

enum TrueValue {
    TRUE,
    FALSE,
    UNDEF,
    UNIT
};

enum Sign {
    POS,
    NEG
};

class DPLL {
public:
    /**
     * Constructor.
     *
     * @param phi the formula to be checked
     * @note Please DON'T CHANGE this signature because the grading script will directly call this function!
     */
    DPLL(const formula& phi);
    ~DPLL();

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

    bool use_backjump = false;

    void init();
    bool dfs();

    bool sat() const;
    bool conflict() const;
    bool propagate();
    void decide();
    bool hasDecision() const;
    void backtrack();
    void backjump();

    void generateModel();

    bool propagateClause(int cidx);

    int findNextUnusedAtom() const;

    void decideAtom(int atom);
    bool updateInterpretations(int liter, bool is_decide);
    bool updateClauseValue(int liter, int cidx, Sign literalSign);

    bool updateClauseValue(const std::list<int>& cs);
    TrueValue evalClause(int cidx) const;

    void addNewClause(const std::set<int>& conflicts);
    void destroyGraph(const std::set<int>& conflicts);

    bool sameSign(Sign s, bool pos);

    formula phi;

    struct AtomInfo {
        int index = 0;
        bool is_decide = false;
        TrueValue value = UNDEF;
    };
    typedef std::list<AtomInfo> AtomList;

    AtomList runningChain;

    // atom (int) :-> Truevalue
    std::vector<TrueValue> interpretations;

    
    // last decided atom
    std::list<int> decideChain;


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

    struct GraphNode {
        bool isDecide = false;
        unsigned int decideLevel = 0xffffffff;
        std::set<int> propagations;
        std::set<int> parents;
        std::set<int> decideNodes;
    };

    typedef std::vector<GraphNode> Graph;

    Graph conflictGraph;

    int conflictNodeIndex = 0;
    int conflictClause = -1;

    model finalModel;
};


#endif //DPLL_DPLL_H

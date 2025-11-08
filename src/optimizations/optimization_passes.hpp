#ifndef OPT_PASS_HPP
#define OPT_PASS_HPP


#include "ast.hpp"
#include "constant_folding_pass.hpp"
#include "constant_propagation_pass.hpp"
#include "constant_coalesce_pass.hpp"
#include "dead_code_elimination_pass.hpp"

namespace DTL{
/*
    In this step we do the following:

    1. We tag all constant definitions outside of the for loop structure

*/



#define DTL_OPT_CONSTFOLD   (1 << 0ULL)
#define DTL_OPT_CONSTPROP   (1 << 1ULL)
#define DTL_OPT_DEADCODE    (1 << 2ULL)


enum DTL_OPTLEVEL
{
    OPT0,
    OPT1,
    OPT2,
    OPTMAX
};



class DTLOptimizer
{
public:

    /*
        Preset optimization levels
    */
    static ASTNode* OptimizeStatic(ProgramNode* prog, DTL_OPTLEVEL opt)
    {
        switch (opt)
        {
            case DTL_OPTLEVEL::OPT0:
            {

            }
            case DTL_OPTLEVEL::OPT1:
            {

            }
            case DTL_OPTLEVEL::OPT2:
            {

            }
            case DTL_OPTLEVEL::OPTMAX:
            {

            }
            default:
            {
                return prog;
            }
        }
    }

    /*
        Manual optimization for dynamic and handtuned kernels

        Useful if the compilation is in the critical path and we cannot amortize the cost 



        We pass in a vector that gives the number of each optimization.
        For example passing in the flags (DTL_OPT_CONSTFOLD|DTL_OPT_DEADCODE) and {3, 1} will run the following sequence

        1. constant folding
        2. dead code elimination
        3. constant folding
        4. constant folding
    */
    static ASTNode* OptimizeManual(ProgramNode* prog, uint8_t opt_flags, const std::vector<int>& opt_count)
    {

    }

    
private:
    DTLOptimizer()
    {

    }


    /*
        We can track the values of constants here such that we can just check here in order to propagate their value
    */
    std::unordered_map<std::string, uint32_t> m_ConstantValues;
    int m_FoldCount;

};







}



#endif
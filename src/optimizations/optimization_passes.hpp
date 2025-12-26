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



#define DTL_OPT_CONSTFOLD       (1 << 0ULL)
#define DTL_OPT_CONSTPROP       (1 << 1ULL)
#define DTL_OPT_DEADCODE        (1 << 2ULL)
#define DTL_OPT_CONSTCOALESCE   (1 << 3ULL)
#define DTL_OPT_MULTGREEDY      (1 << 4ULL)
#define DTL_OPT_ADDGREEDY       (1 << 5ULL)
#define DTL_TRANSFORM_OPTMAX    (DTL_OPT_MULTGREEDY | DTL_OPT_ADDGREEDY)
#define DTL_OPT_MAX             (DTL_OPT_CONSTFOLD|DTL_OPT_CONSTPROP|DTL_OPT_DEADCODE|DTL_OPT_CONSTCOALESCE|DTL_TRANSFORM_OPTMAX)


enum DTL_OPTLEVEL
{
    OPT0,
    OPT1,  // Prop -> Fold -> Coalesce
    OPT2,  // 2(Prop -> Fold) -> Coalesce -> Deadcode
    OPTMAX // 2(Prop -> Fold) -> Coalesce -> Deadcode -> MultGreedySchedule -> AddGreedySchedule
};



class DTLOptimizer
{
public:

    /*
        Preset optimization levels
    */
    static ASTNode* OptimizeStatic(ProgramNode* prog, DTL_OPTLEVEL opt)
    {
        int fold_count = 0;
		int prop_count = 0;
        switch (opt)
        {
            case DTL_OPTLEVEL::OPT0:
            {
                return prog;
            }
            case DTL_OPTLEVEL::OPT1:
            {
                prog = static_cast<DTL::ProgramNode*>(DTL::ConstantFoldPass::ConstantFold(prog, &fold_count));
		        prog = static_cast<DTL::ProgramNode*>(DTL::ConstantPropagationPass::ConstantPropagation(prog, &prop_count));
                prog = static_cast<DTL::ProgramNode*>(DTL::ConstantCoalescePass::ConstantCoalesce(prog));
                return prog;

            }
            case DTL_OPTLEVEL::OPT2:
            {
                for (int i = 0; i < 2; i++)
                {
                    prog = static_cast<DTL::ProgramNode*>(DTL::ConstantFoldPass::ConstantFold(prog, &fold_count));
		            prog = static_cast<DTL::ProgramNode*>(DTL::ConstantPropagationPass::ConstantPropagation(prog, &prop_count));
                }

                prog = static_cast<DTL::ProgramNode*>(DTL::ConstantCoalescePass::ConstantCoalesce(prog));
	            prog = static_cast<DTL::ProgramNode*>(DTL::DeadCodeEliminationPass::DeadCodeElimination(prog));

                return prog;
                
            }
            case DTL_OPTLEVEL::OPTMAX:
            {
                for (int i = 0; i < 2; i++)
                {
                    prog = static_cast<DTL::ProgramNode*>(DTL::ConstantFoldPass::ConstantFold(prog, &fold_count));
		            prog = static_cast<DTL::ProgramNode*>(DTL::ConstantPropagationPass::ConstantPropagation(prog, &prop_count));
                }

                prog = static_cast<DTL::ProgramNode*>(DTL::ConstantCoalescePass::ConstantCoalesce(prog));
	            prog = static_cast<DTL::ProgramNode*>(DTL::DeadCodeEliminationPass::DeadCodeElimination(prog));

                return prog;
                
            }
            default:
            {
                assert(false); // shouldn't happen
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
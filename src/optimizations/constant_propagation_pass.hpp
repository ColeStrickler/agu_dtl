#ifndef CONST_PROP_PASS_HPP
#define CONST_PROP_PASS_HPP


#include "ast.hpp"


namespace DTL{
/*
    In this step we do the following:

    1. We tag all constant definitions outside of the for loop structure

*/
class ConstantPropagationPass
{
public:
    static ASTNode* ConstantPropagation(ProgramNode* prog, int* prop_count)
    {
        auto prop_pass = new ConstantPropagationPass();
        auto progret = prog->ConstPropagation(prop_pass);
        *prop_count = prop_pass->m_PropCount;
        delete prop_pass;
        return progret;
    }


    void AddConstant(IDNode* id, IntLitNode* node);
    int FindConstant(IDNode* id);
    void IncPropCount();
    int GetPropCount();
private:
    ConstantPropagationPass() : m_PropCount(0)
    {

    }

    


    /*
        We can track the values of constants here such that we can just check here in order to propagate their value

        All ConstDeclNodes can be propagated or DeadCode Eliminated

        This is due to the fact that we restrict them to equal a single constant and disallow them to equal expressions. 

        We will allow the condition statement in the for loop to hold a variable as long as it can be propagated
    */
    std::unordered_map<std::string, int> m_ConstantValues; // we just need to store the ConstDeclNode values here
    int m_PropCount;

};







}



#endif
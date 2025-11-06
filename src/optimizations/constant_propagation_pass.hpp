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
    static ASTNode* ConstantPropagation(ProgramNode* prog)
    {
        return prog->ConstantPropagation();
    }


    
private:
    ConstantPropagationPass()
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
#ifndef CONST_FOLD_PASS_HPP
#define CONST_FOLD_PASS_HPP


#include "ast.hpp"


namespace DTL{
/*
    In this step we do the following:

    

*/
class ConstantFoldPass
{
public:
    static ASTNode* ConstantFold(ProgramNode* prog)
    {
        auto foldpass = new ConstantFoldPass();
        return prog->ConstFold(foldpass);
    }


    void IncFoldCount();
private:
    ConstantFoldPass()
    {

    }


    /*
        We can track the values of constants here such that we can just check here in order to propagate their value
    */
    int m_FoldCount;

};







}



#endif
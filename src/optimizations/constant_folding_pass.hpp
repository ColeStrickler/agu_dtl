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
    static ASTNode* ConstantFold(ProgramNode* prog, int* fold_count)
    {
        auto foldpass = new ConstantFoldPass();
        auto progret = prog->ConstFold(foldpass);
        *fold_count = foldpass->m_FoldCount;
        delete foldpass;
        return progret;
    }


    void IncFoldCount();
    int GetFoldCount();
private:
    ConstantFoldPass() : m_FoldCount(0)
    {

    }


    /*
        We can track the values of constants here such that we can just check here in order to propagate their value
    */
    int m_FoldCount;

};







}



#endif
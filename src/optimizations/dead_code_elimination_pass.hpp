#ifndef DEAD_CODE_PASS_HPP
#define DEAD_CODE_PASS_HPP


#include "ast.hpp"
#include "unordered_set"

namespace DTL{
/*
    This pass will basically only eliminate un-used ConstDeclNodes
*/
class DeadCodeEliminationPass
{
public:
    static ASTNode* DeadCodeElimination(ProgramNode* prog)
    {
        auto elim_pass = new DeadCodeEliminationPass();
        prog = (ProgramNode*)prog->DeadCodeElimination(elim_pass, 0);
        auto progret = (ProgramNode*)prog->DeadCodeElimination(elim_pass, 1);
        delete elim_pass;
        return progret;
    }



    void AddConstDecl(ConstDeclNode* constDecl);
    void AddConstArrayDecl(ConstArrayDeclNode* constArrayDecl);
    void SetIsUsed(std::string id);

    bool isInDeclMap(std::string id);
    std::unordered_set<std::string> m_UsedConstDecls;

private:
    DeadCodeEliminationPass() 
    {

    }

    



};







}



#endif
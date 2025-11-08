#ifndef CONST_COALESCE_PASS_HPP
#define CONST_COALESCE_PASS_HPP


#include "ast.hpp"
#include <unordered_set>


namespace DTL{
/*
    RUN THIS STEP AFTER DEAD CODE ELIMINATION AND CONST FOLDING&PROPAGATION
    - we eliminate all constants by forward propping them and then we coalesce
*/
class ConstantCoalescePass
{
public:
    static ASTNode* ConstantCoalesce(ProgramNode* prog)
    {
        auto coalesce_pass = new ConstantCoalescePass();
        
        /*
            We make two passes. In pass 1, we collect data

            In pass 2 we coalesce the constants
        */
        prog = (ProgramNode*)prog->ConstCoalesce(coalesce_pass, 0);
        printf("starting pass 2\n");
        auto progret = prog->ConstCoalesce(coalesce_pass, 1);

        delete coalesce_pass;
        return progret;
    }

    bool HasCoalesceableNodes();


    bool ToCoalesce(int val);
    void AddIDMapping(int val, std::string id);
    std::string GetIDMapping(int val);
    void AddToCollectionMap(int val, ASTNode* node);
    std::string GenIDString();

    std::unordered_map<int, std::unordered_set<ASTNode*>> m_CoalesceCollectionMap;
    std::unordered_map<int, std::string> m_CoalesceNewIDMap;

private:
    ConstantCoalescePass() : m_IDTicker(0)
    {

    }

    
    int m_IDTicker;

};







}



#endif
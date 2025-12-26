#ifndef AST_TRANSFORM_HPP
#define AST_TRANSFORM_HPP


#include "ast.hpp"
#include "optimizations/optimization_passes.hpp"


namespace DTL{
/*
    The goal of this step is to transform the parsed AST into a form that more suitably matches the hardware

    1.  We will collapse for loops so we have information available for mapping them to the ForLoop Registers
    2.  We will push constants down to the furthest depth of the AST, so we can map them to AddUnits with +0. This way we can
        propagate them through the systolic architecture and they will be mapped to a location ready for use when needed

*/
class ASTTransformPass
{
public:
    static ASTNode* Transform(ProgramNode* prog, uint8_t opt_flags)
    {

        return prog->TransformPass(opt_flags);
    }



private:
    ASTTransformPass()
    {

    }





};







}



#endif
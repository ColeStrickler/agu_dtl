#include "resource_analysis_pass.hpp"
#include "ast.hpp"



void DTL::ProgramNode::resourceAnalysis(ResourceAnalysis *ra, int layer)
{
    for (auto& stmt: myStatements)
    {
        stmt->resourceAnalysis(ra, 0);
    }
}


/*
    Since we alloc a Const Register here, we need to avoid calling this again when we do
    resource analysis on the ForLoop node. We want the ConstDecl child node of the ForLoop
    node to alloc a ForLoopRegister instead.
*/
void DTL::ConstDeclNode::resourceAnalysis(ResourceAnalysis *ra, int layer)
{
    ra->UseNewConst();
    ra->RegMapConst(myID, ra); // assign constants a register number
}



void DTL::PostIncStmtNode::resourceAnalysis(ResourceAnalysis *ra, int layer)
{
    return; // nothing to do here, the LocNode should already be handled
}


void DTL::ForStmtNode::resourceAnalysis(ResourceAnalysis *ra, int layer)
{
    ra->UseForLoopRegister(); // 1 per for loop. Necessitated by the grammar;
    
    /*
        We do not need to do anything for the conditional statement, resources are always allocated
        for the comparison. 
    */
    for (auto& stmt: myStatements)
        stmt->resourceAnalysis(ra, layer);
}



void DTL::OutStmtNode::resourceAnalysis(ResourceAnalysis *ra, int layer)
{
    myExp->resourceAnalysis(ra, layer);
    ra->UseNewOutStatement(); // alloc outstatement at the end so we can zero index the resources
}


/*
    This should work because we will only enter this when evaluating the OutStmtNode
*/
void DTL::IntLitNode::resourceAnalysis(ResourceAnalysis *ra, int layer)
{
    ra->UseNewConst();
    ra->RegMapConst(this, ra); // assign constants a register number
}

void DTL::PlusNode::resourceAnalysis(ResourceAnalysis* ra, int layer)
{
    if (isPassThrough())
        ra->UseNewPassThroughLayer(layer);
    else
        ra->UseNewAddUnitLayer(layer);


    /*
        We will alocate layers backwards, such that layer N is actually layer 0, 
        layer N-1 is actually layer 1, and so on
    */
    myExp1->resourceAnalysis(ra, layer+1);

    if (!isPassThrough())
        myExp2->resourceAnalysis(ra, layer+1);
}



void DTL::TimesNode::resourceAnalysis(ResourceAnalysis* ra, int layer)
{
    ra->UseNewMultUnitLayer(layer);


    /*
        We will alocate layers backwards, such that layer N is actually layer 0, 
        layer N-1 is actually layer 1, and so on
    */
    myExp1->resourceAnalysis(ra, layer+1);
    myExp2->resourceAnalysis(ra, layer+1);
}

std::string DTL::TimesNode::PrintAST(int &node_num, std::ofstream &outfile)
{
	node_num++;
	std::string name = "TimesNode" + std::to_string(node_num);
	auto exp1NodeString = myExp1->PrintAST(node_num, outfile);
	auto exp2NodeString = myExp2->PrintAST(node_num, outfile);
	outfile << name << " -> " << exp1NodeString << ";\n";
	outfile << name << " -> " << exp2NodeString << ";\n";
	
	return name;
}




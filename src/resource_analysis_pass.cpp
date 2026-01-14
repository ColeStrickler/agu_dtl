#include "resource_analysis_pass.hpp"
#include "ast.hpp"
#include "resource_alloc_pass.hpp"


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

    if (ra->GetConstRegMapping(myID->getName()) == -1)
    {
        printf("ConstDecl\n");
        ra->UseNewConst();
        ra->RegMapConst(myID->getName(), ra, myVal->GetVal()); // assign constants a register number
    }

}

void DTL::ConstArrayDeclNode::resourceAnalysis(ResourceAnalysis *ra, int layer)
{

    
    ra->UseNewConstArray(myVals.size());
    ra->RegMapConstArray(myID->getName(), ra);

    /*
        We will bind the index when it is used

        If it is never used, then we do not worry about it.
        This is just programmer inefficiency
    */
    
}

void DTL::ArrayIndexNode::resourceAnalysis(ResourceAnalysis *ra, int layer)
{
    // this actually needs to be bound to a for loop register
   //ra->BindConstArrayIndex(myID->getName(), myIndexVar->getName(), ra);
    return;
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
    if (ra->GetConstRegMapping(std::to_string(myNum)) == -1)
    {
        ra->UseNewConst();
        printf("IntLit %d 0x%x\n", myNum, this);
        ra->RegMapConst(std::to_string(myNum), ra, myNum); // assign constants a register number
    }
}

void DTL::PlusNode::resourceAnalysis(ResourceAnalysis* ra, int layer)
{
    if (isPassThrough())
    {
        ra->UseNewPassThroughLayer(layer);
    }
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

int DTL::ResourceAnalysis::GetConstArrayRegOffset() const
{
    return hwStat->nConstRegisters;
}

int DTL::ResourceAnalysis::GetConstArrayRegMapping(std::string node_name)
{
	auto it = ConstArrayRegMapping.find(node_name);
	if (it != ConstArrayRegMapping.end())
		return it->second;
	return -1;
}
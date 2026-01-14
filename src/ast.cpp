#include "ast.hpp"



void DTL::ProgramNode::PrintAST(const std::string &file)
{
	std::ofstream outfile(file);  // open for writing
	if (!outfile.is_open()) {
		std::cerr << "Failed to open file.\n";
		return;
	}
	int node_number = 0;
	std::string name = "ProgramNode";
	outfile << "digraph G {\n";
	for (auto& stmt: myStatements)
	{
		auto s = stmt->PrintAST(node_number, outfile);
		outfile << name << " -> " << s << ";\n";
	}
	outfile << "}\n";

    outfile.close();
}



std::string DTL::ConstDeclNode::PrintAST(int &node_num, std::ofstream &outfile)
{
	node_num++;
	std::string name = "ConstDeclNode" + std::to_string(node_num);
	auto typeNodeString = myType->PrintAST(node_num, outfile);
	auto idNodeString = myID->PrintAST(node_num, outfile);
	auto valString = myVal->PrintAST(node_num, outfile);
	outfile << name << " -> " << typeNodeString << ";\n";
	outfile << name << " -> " << idNodeString << ";\n";
	outfile << name << " -> " << valString << ";\n";
	return name;
}


std::string DTL::ConstArrayDeclNode::PrintAST(int &node_num, std::ofstream &outfile)
{
	node_num++;
	std::string name = "ConstArrayDeclNode" + std::to_string(node_num);
	auto type = myType->PrintAST(node_num, outfile);
	auto id = myID->PrintAST(node_num, outfile);

	outfile << name << " -> " << type << ";\n";
	outfile << name << " -> " << id << ";\n";
    return name;
}



std::string DTL::PostIncStmtNode::PrintAST(int &node_num, std::ofstream &outfile)
{
	node_num++;
	std::string name = "PostIncStmtNode" + std::to_string(node_num);
	auto locNodeString = myLoc->PrintAST(node_num, outfile);
	outfile << name << " -> " << locNodeString << ";\n";
	return name;
}

std::string DTL::ArrayIndexNode::PrintAST(int &node_num, std::ofstream &outfile)
{
    node_num++;
	std::string name = "ArrayIndexNode" + std::to_string(node_num);
	auto id = myID->PrintAST(node_num, outfile);
	auto index_id = myIndexVar->PrintAST(node_num, outfile);
	outfile << name << " -> " << id << ";\n";
	outfile << name << " -> " << index_id << ";\n";

	return name;
}


std::string DTL::ForStmtNode::PrintAST(int &node_num, std::ofstream &outfile)
{
	node_num++;
	std::string name = "ForStmtNode" + std::to_string(node_num);
	auto initNodeString = myInit->PrintAST(node_num, outfile);
	auto condNodeString = myCondExp->PrintAST(node_num, outfile);
	auto updateNodeString = myUpdateStmt->PrintAST(node_num, outfile);
	outfile << name << " -> " << initNodeString << ";\n";
	outfile << name << " -> " << condNodeString << ";\n";
	outfile << name << " -> " << updateNodeString << ";\n";
	for (auto& stmt: myStatements)
	{
		auto stmtString = stmt->PrintAST(node_num, outfile);
		outfile << name << " -> " << stmtString << ";\n";
	}
	return name;
}


std::string DTL::OutStmtNode::PrintAST(int &node_num, std::ofstream &outfile)
{
	node_num++;
	std::string name = "OutStmtNode" + std::to_string(node_num);
	auto expNodeString = myExp->PrintAST(node_num, outfile);
	outfile << name << " -> " << expNodeString << ";\n";
	return name;
}


std::string DTL::TypeNode::PrintAST(int& node_num, std::ofstream& outfile)
{
	node_num++;
	std::string name = "TypeNode" + std::to_string(node_num);
	return name;
}



std::string DTL::IntTypeNode::PrintAST(int &node_num, std::ofstream &outfile)
{
	node_num++;
	std::string name = "IntTypeNode" + std::to_string(node_num);
	return name;
}

std::string DTL::IDNode::PrintAST(int &node_num, std::ofstream &outfile)
{
	node_num++;
	std::string name = "IDNode" + std::to_string(node_num) + "_" + getName();
	return name;
}

std::string DTL::IntLitNode::PrintAST(int &node_num, std::ofstream &outfile)
{
	node_num++;
	std::string name = "IntLitNode" + std::to_string(node_num) + "_" + std::to_string(myNum);
	return name;
}

std::string DTL::PlusNode::PrintAST(int &node_num, std::ofstream &outfile)
{
	node_num++;
	std::string name = (isPassThrough() ? "PassThrough" : "PlusNode") + std::to_string(node_num);
	auto exp1NodeString = myExp1->PrintAST(node_num, outfile);
	auto exp2NodeString = myExp2->PrintAST(node_num, outfile);
	outfile << name << " -> " << exp1NodeString << ";\n";
	outfile << name << " -> " << exp2NodeString << ";\n";
	
	return name;
}

std::string DTL::LessEqNode::PrintAST(int &node_num, std::ofstream &outfile)
{
	node_num++;
	std::string name = "LessEqNode" + std::to_string(node_num);
	auto exp1NodeString = myExp1->PrintAST(node_num, outfile);
	auto exp2NodeString = myExp2->PrintAST(node_num, outfile);
	outfile << name << " -> " << exp1NodeString << ";\n";
	outfile << name << " -> " << exp2NodeString << ";\n";
	
	return name;
}


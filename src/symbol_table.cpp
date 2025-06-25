#include "symbol_table.hpp"
#include "types.hpp"
using namespace DTL;



DTL::ScopeTable::ScopeTable() : symbols(new std::unordered_map<std::string, SemSymbol*>())
{

}

SemSymbol *ScopeTable::lookup(std::string name)
{
    if (symbols->find(name) != symbols->end())
        return symbols->at(name);
    return nullptr;
}

bool ScopeTable::insert(SemSymbol* sym)
{
    std::string symName = sym->getName();
	bool alreadyInScope = (this->lookup(symName) != NULL);
	if (alreadyInScope){
		return false;
	}
	this->symbols->insert(std::make_pair(symName, sym));
	return true;
}

bool ScopeTable::clash(std::string varName) 
{
    // use this method upon declaration
    SemSymbol * found = lookup(varName);
	if (found != nullptr){
		return true;
	}
	return false;
}

std::string DTL::ScopeTable::toString() const
{
    assert(symbols);
    std::string ret;
    auto deref_symbols = *symbols;
    for (auto e: deref_symbols)
        ret += e.second->toString() + "\n";
    return ret;
    
}

SymbolTable::SymbolTable() : scopeTableChain(new std::vector<ScopeTable*>())
{
}

ScopeTable *SymbolTable::enterScope()
{
    assert(scopeTableChain);
    auto newScope = new ScopeTable();
    scopeTableChain->push_back(newScope);
    return newScope;
}

void DTL::SymbolTable::leaveScope()
{
    assert(scopeTableChain);
    assert(scopeTableChain->size() > 0);
    scopeTableChain->pop_back();
}

ScopeTable *DTL::SymbolTable::getCurrentScope()
{
    assert(scopeTableChain);
    auto size = scopeTableChain->size();
    assert(size > 0);  
    return scopeTableChain->back();
}



bool DTL::SymbolTable::insert(SemSymbol *symbol)
{
    auto current_scope = getCurrentScope();
    return current_scope->insert(symbol);
}

SemSymbol *DTL::SymbolTable::find(std::string varName)
{
    assert(scopeTableChain);
    assert(scopeTableChain->size());

    auto deref_chain = (*scopeTableChain);
    for (int i = scopeTableChain->size()-1; i >= 0; i--)
    {
        auto scope = deref_chain[i];
        auto sym = scope->lookup(varName);
        if (sym != nullptr)
            return sym;
    }

    return nullptr;
}

bool DTL::SymbolTable::clash(std::string name)
{
    /*
        We will force variables to be globally unique
    */
    assert(scopeTableChain);
    assert(scopeTableChain->size());

    auto deref_chain = (*scopeTableChain);
    for (int i = scopeTableChain->size()-1; i >= 0; i--)
    {
        auto scope = deref_chain[i];
        if (scope->clash(name))
            return true;
    }
    return false;
}

void DTL::SymbolTable::print()
{
    for(auto scope : *scopeTableChain){
		std::cout << "--- scope ---\n";
		std::cout << scope->toString();
	}
}

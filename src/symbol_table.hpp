#ifndef SYM_TABLE_HPP
#define SYM_TABLE_HPP

#include <vector>
#include <string>
#include "errors.hpp"
#include <unordered_map>
#include "types.hpp"
#include <cassert>


namespace DTL
{

    enum SymbolType
    {
        VAR,
    };

    // A semantic symbol, which represents a single
    //  variable, function, etc. Semantic symbols
    //  exist for the lifetime of a scope in the
    //  symbol table.
    class SemSymbol
    {
    public:
        SemSymbol(std::string nameIn, const DataType *typeIn)
            : myName(nameIn), myType(typeIn)
        {
            if (myType == nullptr)
            {
                throw new InternalError("symbol with no type");
            }
        }
        virtual std::string toString()  const = 0;
        std::string getName() const { return myName; }
        virtual SymbolType getSymbolType() const = 0;

        virtual const DataType *getDataType() const
        {
            return myType;
        }
        static std::string kindToString(SymbolType symKind)
        {
            switch (symKind)
            {
            case VAR:
                return "var";
            }
            return "UNKNOWN KIND";
        }

    protected:
        std::string myName;
        const DataType *myType;
    };

    class VarSymbol : public SemSymbol
    {
    public:
        VarSymbol(std::string name, const DataType *type)
            : SemSymbol(name, type) {}
        virtual SymbolType getSymbolType() const override { return VAR; }
        std::string toString() const override {
            return myType->getString() + ":" + getName();
        }
    };

    // a ScopeTable.
    class ScopeTable
    {
    public:
        ScopeTable();
        SemSymbol *lookup(std::string name);
        bool insert(SemSymbol *symbol);
        bool clash(std::string name);
        std::string toString() const;
        void addVar(std::string name, const DataType *type)
        {
            insert(new VarSymbol(name, type));
        }
        // FnSymbol * addFn(std::string name, FnType * type){
        //	FnSymbol * sym = new FnSymbol(name, type);
        //	insert(sym);
        //	return sym;
        // }
    private:
        std::unordered_map<std::string, SemSymbol *> *symbols;
    };

    class SymbolTable
    {
    public:
        SymbolTable();
        ScopeTable *enterScope();
        void leaveScope();
        ScopeTable *getCurrentScope();
        bool insert(SemSymbol *symbol);
        SemSymbol *find(std::string varName);
        bool clash(std::string name);
        void addVar(std::string name, const DataType *type)
        {
            getCurrentScope()->addVar(name, type);
        }
        // SemSymbol * addFn(std::string name, FnType * type){
        //	return getCurrentScope()->addFn(name, type);
        // }
        void print();

    private:
        std::vector<ScopeTable *> *scopeTableChain;
    };
}

#endif
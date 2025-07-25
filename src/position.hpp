#ifndef AGU_POSITION_HPP
#define AGU_POSITION_HPP

#include <string>

namespace DTL {


class Position{
public: 
	Position(size_t lineI, size_t colI, size_t lineE, size_t colE)
	: myLineI(lineI), myColI(colI), myLineE(lineE), myColE(colE){
	}
	Position(const Position * start, const Position * end)
	: myLineI(start->myLineI), myColI(start->myColI),
	  myLineE(end->myLineE),myColE(end->myColE){
	}
	virtual void expand(const Position * start, const Position * end){
	  myLineI = start->myLineI;
	  myColI = start->myColI;
	  myLineE = end->myLineE;
	  myColE = end->myColE;
	}
	virtual std::string begin() const{
		std::string result = "[" 
		+ std::to_string(myLineI)
		+ "," 
		+ std::to_string(myColI)
		+ "]";
		return result;
	}
	virtual std::string span() const{
		std::string result = begin()
		+ "-[" 
		+ std::to_string(myLineE)
		+ "," 
		+ std::to_string(myColE)
		+ "]";
		return result;
	}
private:
	size_t myLineI;
	size_t myColI;
	size_t myLineE;
	size_t myColE;

};


}

#endif 
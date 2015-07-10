#include <vector>
#include <stack>
#include <string>
#include <cstdlib>
#include <string>
#include "utility.h"
#include "floorplan_citizen.h"

//Temporary fix for MinGW to_string
std::string to_string(int i) { std::stringstream ss; ss << i; return ss.str(); }

void floorplan_citizen::initialize(module* gates)
{
    this->gates = gates;
	polish.clear();

	//Make an initial polish string of all things vertical.
	//Ex: 12V3V4V5V...
	int size = gates->gates.size();
    if(size > 1) {
		polish.push_back("0");
		polish.push_back("1");
		polish.push_back("V");
        for(int i = 2; i < size; ++i) {
			polish.push_back(to_string(i));
			polish.push_back("V");
        }
    } else if(size == 1){
        polish.push_back("0");   //Only one gate
    } else {
        //0 gates
    }
	
	/* Generate the initial operator counts. This tells us the 
	 * number of operators up to each point i in the polish string */
	opCounts.resize(polish.size());
	int seenops = 0;
	for(unsigned i=0; i!=polish.size(); ++i) {
		if(polish[i] == "H" || polish[i] == "V")
			++seenops;
		opCounts[i] = seenops;
	}

	//Create initial adjacency graph
    generateAdjacencyGraph();
}

std::vector<std::string> floorplan_citizen::getPolish()
{
    return polish;
}

/********************************************************/
/* Floorplan Citizen Fitness calculation implementation */
/********************************************************/

void floorplan_citizen::calc_fitness()
{
	
}

//TODO: Add code here

/********************************************************/
/* Floorplan Citizen String Mutation implementation     */
/********************************************************/

void floorplan_citizen::mutate()
{
    int selection = rand() % 3;
    //selection = 1; //debug
	
    switch(selection)
    {
	case 0: {
		/* Swap two operands; EG: 12HV45HV -> 12HV54HV
		 * In this case, we swap two rows of the matrix to update it. */
		auto swapped = swapOperands();
		adjgraph[swapped.first].swap(adjgraph[swapped.second]);
		}
		break;
	case 1: 
		/* Find a chain of operators, and do: H->V, V->H */
		complementChain();
		generateAdjacencyGraph();
		break;
	case 2: 
		/* Swap an adjacent operand (1,3,9)... with an adjacent H or V 
		 * swapOperandOperator updates the operator counts. */
		swapOperandOperator();
		generateAdjacencyGraph();
		break;
    }
}

std::pair<int,int> floorplan_citizen::swapOperands()
{
	int operands[2];
	int indicies[2];
	
	//Find two random indices in the polish that are not operators
	//Note: any random two. Not necessarily adjacent.
	for(int i = 0; i != 2; ++i) {
		int index = -1;
		do {
			index = rand() % polish.size();
		} while(polish[index] == "H" || polish[index] == "V");
		indicies[i] = index;
		operands[i] = std::atoi(polish[index].c_str());
	}
	
	//Swap these indices in the polish
	std::swap(polish.at(indicies[0]), polish.at(indicies[1]));
	
	//Return the elements swapped (not the indices)
	return std::make_pair(operands[0], operands[1]);
}

std::pair<int,int> floorplan_citizen::complementChain()
{
    std::vector<std::string>& str = polish;
    std::vector<int> chainIndex;
    
    for(int i=1; i<str.size(); ++i)
    {
        if( (str[i] == "H" || str[i] == "V") && (str[i-1] != "H" && str[i-1] != "V"))
        {
            chainIndex.push_back(i);
        }
    }
    
    int complementIndex = chainIndex[rand() % chainIndex.size()];
    
    //std::cout<<complementIndex<<"\n"; std::vector<std::string> stuff; //debug
    
    for(int i=complementIndex; (i<str.size() && (str[i] == "H" || str[i] == "V")); ++i)
    {
        if(str[i] == "H")
        {
            str[i] = "V";
            //stuff.push_back(str[i]);
        }
        else if(str[i] == "V")
        {
            str[i] = "H";
            //stuff.push_back(str[i]);
        }
    }
    
    //std::cout<<"changed: "; for(std::string st : stuff) { std::cout<<st<<" "; } std::cout<<"\n"; //debug
    
    return std::make_pair(complementIndex, -1);
}

std::pair<int,int> floorplan_citizen::swapOperandOperator()
{
    std::vector<std::string>& str = polish;
    int index = -1;
    
    for(int i=1; i<str.size()-2; ++i)
    {
        //std::cout<<"opcount = "<<(2*opCount[i+1])<<"\n";
        
        if(
           ((str[i-1] != str[i+1]) || (2*opCounts[i+1] < i) )
           && (str[i] != "H" && str[i] != "V")
           && (str[i+1] == "H" || str[i+1] == "V")
           )
        {
            //std::cout<<str[i-1]<<".."<<str[i]<<"..."<<str[i+1]<<"\n";
            index = i;
        }
    }
    
	//Swap the operand and operator
	std::swap(str[index], str[index+1]);
    
    std::pair<int, int> changedNumbers = std::make_pair(index,index+1);
    
    //std::cout<<">>"<<changedNumbers.first<<" "<<changedNumbers.second<<std::endl;
    
    return changedNumbers;
}

/******************************************************/
/* Floorplan Adjacency graph implementation          */
/******************************************************/

/* Floorplan adjacency matrix validation:
 * An edge src->dst is valid if nothing attached to dst attaches to src
 * in the same way (eg, both Vs).
 */
bool floorplan_citizen::validateAddition(int src, int dst)
{
    for(unsigned connection = 0; connection != adjgraph.size(); ++connection) {
        if(adjgraph[dst][connection] == '-')
            continue;
        if(adjgraph[dst][connection] == adjgraph[connection][src])
            return false;
    }
    return true;
}

/* Calculates a combined vertical/horizontal floorplan adjacency graph
 * from its polish string
 */
void floorplan_citizen::generateAdjacencyGraph()
{
    std::stack<std::vector<int>>  stack;

    //TODO: Polish representation with more than 9 modules
    int nGates = gates->gates.size();
    adjgraph.clear();
    adjgraph.resize(nGates);
    for(auto& row : adjgraph)
        row.resize(nGates,'-');

    for(const std::string& c : this->polish)
    {
        if(c == "H" || c == "V") {
            /* We take XY[OP] off the stack, and connect all elements
             * in lhs to rhs. This usually isn't correct, so it
             * needs to be validated. The validation before all
             * connections are made is erroneous, but it is N^2 vs N^3.*/
            std::vector<int> lhs = std::move(stack.top()); stack.pop();
            std::vector<int> rhs = std::move(stack.top()); stack.pop();
            for(int g : lhs) {
            for(int h : rhs) {
                if(validateAddition(g,h)) {
                    ((c == "H") ?  adjgraph[g][h] : adjgraph[h][g]) = c[0];
                }
            }
            }

            /* Concatenate vector rhs to lhs and push result back on stack.
             * This has the effect of creating rectangles of rectangles. */
            int oldSize = lhs.size();
            lhs.resize(lhs.size() + rhs.size());
            std::move(rhs.begin(), rhs.end(), lhs.begin() + oldSize);
            stack.push(std::move(lhs));
        }
        else {
            stack.push(std::vector<int>(1,std::atoi(c.c_str())));
        }
    }

#if 0
    for(unsigned i = 0; i != adjgraph.size(); ++i)
    for(unsigned j = 0; j != adjgraph.size(); ++j)
    {
        if(!validateAddition(i,j))
            adjgraph[i][j] = '-';
    }
#endif
}

std::string floorplan_citizen::getDotGraphText()
{
    std::stringstream ss;
    ss << "graph {" << std::endl;

    for(unsigned i = 0; i != adjgraph.size(); ++i)
    for(unsigned j = 0; j != adjgraph.size(); ++j)
    {
        char c = adjgraph[i][j];
        int weight = 0;

        if(c == 'H')
            weight = gates->gates[i+2].length/2 + gates->gates[j+2].length/2;
        else if(c == 'V')
            weight = gates->gates[i+2].width/2  + gates->gates[j+2].width/2;

        if(weight != 0)
            ss << '\t' << i << " -- " << j << " [label=\"  " << weight << "\"]" << std::endl;
    }

    ss << "}" << std::endl;
    return ss.str();
}


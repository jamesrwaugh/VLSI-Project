#include <vector>
#include <stack>
#include <string>
#include "utility.h"
#include "floorplan_citizen.h"

void floorplan_citizen::setGates(module* gates)
{
   this->gates = gates;
}

std::string floorplan_citizen::getPolish()
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

}

//TODO: Add code here

/******************************************************/
/* Floorplan Adjacencty graph implementation          */
/******************************************************/

/* Floorplan adjacency matrix validation:
 * An entry at i,j is valid if no other connections in row i are connected to j.
 * In other words, Makes i,j not valid if i is connected to something
 * that is connected to j in the same way (both H or V)
 * Note this does not fully correct the problem. But it's enough.
 */
bool floorplan_citizen::validateAddition(int i, int j)
{
    for(unsigned connection = 0; connection != adjgraph[i].size(); ++connection) {
        if(adjgraph[i][connection] == '-')
            continue;
        if(adjgraph[i][connection] == adjgraph[connection][j])
            return false;
    }
    return true;
}

/* Calculates a combined vertical/horizontal floorplan adjacency graph
 * from its polish string
 */
void floorplan_citizen::generateAdjacencyGraph()
{
    std::stack<std::vector<int>> stack;

    //TODO: Polish representation with more than 9 modules
    adjgraph.clear();
    adjgraph.resize(9);
    for(auto& row : adjgraph)
        row.resize(9,'-');

    for(char c : this->polish)
    {
        if(c == 'H' || c == 'V') {
            /* We take XY[OP] off the stack, and connect all elements
             * in lhs to rhs. This usually isn't correct, so it
             * needs to be validated. Doing the validation before all
             * connections are made is erronious, but it is N^2 vs N^3.*/
            std::vector<int> lhs = std::move(stack.top()); stack.pop();
            std::vector<int> rhs = std::move(stack.top()); stack.pop();
            for(int g : lhs) {
            for(int h : rhs) {
                if(validateAddition(g,h))
                    ((c == 'H') ?  adjgraph[g][h] : adjgraph[h][g]) = c;
            }
            }

            /* Concatinate vector rhs to lhs and push result back on stack.
             * This has the effect of creating rectangles of rectangles. */
            int oldSize = lhs.size();
            lhs.resize(lhs.size() + rhs.size());
            std::move(rhs.begin(), rhs.end(), lhs.begin() + oldSize);
            stack.push(std::move(lhs));
        }
        else {
            stack.push(std::vector<int>(1, c-'0'));
        }
    }
}

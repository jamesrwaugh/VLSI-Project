#include <vector>
#include <stack>
#include <string>
#include "floorplan.h"
#include "utility.h"

std::string floorplan(const module &partition)
{
    (void)(partition);
    return "Not Implemented Yet!";
}

/****************************************************/

typedef std::vector<std::vector<char>> floorplan_adjgraph;

/* Floorplan adjacency validation:
 * An entry at i,j is valid if no other connections in row i are connected to j.
 * In other words, Makes i,j not valid if i is connected to something
 * that is connected to j in the same way (both H or V)
 * Note this does not fully correct the problem. But it's enough.
 */
bool validateAddition(int i, int j, const floorplan_adjgraph& g)
{
    for(unsigned connection = 0; connection != g[i].size(); ++connection) {
        if(g[i][connection] == '-')
            continue;
        if(g[i][connection] == g[connection][j])
            return false;
    }
    return true;
}

/* Calculates a combined vertical/horizontal floorplan adjacency graph
 * from a polish string */
floorplan_adjgraph adjacencyGraph(const std::string& polish)
{
    std::stack<std::vector<int>> stack;
    floorplan_adjgraph adjGraph;

    adjGraph.resize(9);
    for(auto& row : adjGraph)
        row.resize(9,'-');

    for(char c : polish)
    {
        if(c == 'H' || c == 'V') {
            /* We take XY[OP] off the stack, and connect all elements
             * in lhs to rhs. This usually isn't correct, so it
             * needs to be validated later. */
            std::vector<int> lhs = std::move(stack.top()); stack.pop();
            std::vector<int> rhs = std::move(stack.top()); stack.pop();
            for(int g : lhs) {
            for(int h : rhs) {
                ((c == 'H') ?  adjGraph[g][h] : adjGraph[h][g]) = c;
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

    /* Validation pass. Each connection to each other connection needs to be validated.
     * See function above. If not valid, we remove it. */
    for(unsigned i = 0; i != adjGraph.size(); ++i) {
    for(unsigned j = 0; j != adjGraph.size(); ++j) {
        if(!validateAddition(i,j,adjGraph)) {
            adjGraph[i][j] = '-';
        }
    }
    }

    return std::move(adjGraph);
}

#if 1
//Floorplan test main
int main()
{
    auto graph = adjacencyGraph("21H43H76H5HVV");

    std::cout << "   ";
    for(int i = 0; i != 9; ++i)
        std::cout << i << " ";
    std::cout << std::endl;

    int i = 0;
    for(const auto& row : graph) {
        std::cout << i++ << " " << row << std::endl;
    }
}
#endif

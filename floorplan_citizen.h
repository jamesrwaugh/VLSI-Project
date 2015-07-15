#ifndef FLOORPLAN_CITIZEN_H
#define FLOORPLAN_CITIZEN_H
#include <string>
#include <vector>
#include "module.h"

/* floorplan_citizen defines a Citizen type to be used
 * with a genetic algorithm */

class floorplan_citizen
{
public:
    //Citizen's fitness value
    long fitness = 0;

public:
    //Sets the gates to floorplan
    void initialize(module* gates);

    //Returns the polish string of the citizen
    std::vector<std::string> getPolish();

    //Return text for graph in DOT format
    std::string getDotGraphText();

    /* The fitness of a floorplan is measured in total wire length.
     * This is calculated as sum(i,j) cij * dij, where cij is the number
     * of connections between gate i and j, and dij is the (approximate) distance. */
    void calc_fitness();

    /* Mutating a floorplan solution involves one of the three operations
     *   on its polish representation:
     * 1) Swap two adjacent operands
     * 2) Complement an operator chain of non-zero length
     * 3) Swap two adjacent operands and operators */
    void mutate();

private:
    //Polish representation of the plan
    std::vector<std::string> polish;

    //Pointer to shared floorplan set of gates
    module* gates = nullptr;

private:
    //Mutation functions and types
    //Components: Roger polish string manipulations
    std::pair<int,int> swapOperands();
    std::pair<int,int> complementChain();
    std::pair<int,int> swapOperandOperator();
	std::vector<int> opCounts;

    //Fitness evaluation functions and types
    //Components: Dijkstra's on the adjgraph, and summation
    //TODO: Add functions
    std::vector<int> shortestPaths(int start);
    int gateDistance(int,int,char);

    //Adjacency graph types and functions
    typedef std::vector<std::vector<char>> floorplan_adjgraph;
    bool validateAddition(int src, int dst);
    void generateAdjacencyGraph();
    floorplan_adjgraph adjgraph;
    bool adjgraphValid = true;
};

#endif // FLOORPLAN_CITIZEN_H


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
    int fitness = 0;

public:
    //Sets the gates to floorplan
    void setGates(module* gates);

    //Returns the polish string of the citizen
    std::string getPolish();

    /* The fitness of a floorplan is measured in total wire length.
     * This is calculated as sum(i,j) cij * dij, where cij is the number
     * of connections between gate i and j, and dij is the (approximate) distance. */
    void calc_fitness();

    /* Mutating a floorplan solution involves one of the three operatons
     *   on its polish representation:
     * 1) Swap two adjacent operands
     * 2) Complement an operator chain of non-zero length
     * 3) Swap two adjacent operands and operators */
    void mutate();

private:
    //Polish representation of the plan
    std::string polish;

    //Pointer to shared floorplan set of gates
    module* gates = nullptr;

private:
    //Mutation functions and types
    //Components: Roger polish string manipulations
    //TODO: Add functions

    //Fitness evaluation functions and types
    //Components: Dijkstra's on the adjgraph, and summation
    //TODO: Add functions

    //Adjacency graph types and functions
    typedef std::vector<std::vector<char>> floorplan_adjgraph;
    bool validateAddition(int i, int j);
    void generateAdjacencyGraph();
    floorplan_adjgraph adjgraph;
};

#endif // FLOORPLAN_CITIZEN_H

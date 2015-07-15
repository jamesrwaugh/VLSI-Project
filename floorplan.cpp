#include <vector>
#include <stack>
#include <string>
#include "floorplan.h"
#include "floorplan_citizen.h"
#include "genetic_algorithm.h"
#include "utility.h"

//Floorplan genetic algorithm derivation
class FloorplanGenetic : public GeneticAlgorithm<floorplan_citizen>
{
public:
    //Constructor; set lower sizes for population size and such
    FloorplanGenetic() :
        GeneticAlgorithm<floorplan_citizen>::GeneticAlgorithm(256, 2048, 0.10, 0.10)
        { }

    //Sets the module for the floorplan
    void setGates(module* gates)
    {
        this->gates = gates;
    }

protected:
    void init_population(population& pop) override
    {
        if(gates == nullptr)
            error("Floorplan algorithm called with no gates");
        for(floorplan_citizen& citizen : pop) {
            citizen.initialize(this->gates);
        }
    }

    void calc_fitness(population& pop) override
    {
        for(floorplan_citizen& citizen : pop)
            citizen.calc_fitness();
    }

    void mate(floorplan_citizen& child,
        const floorplan_citizen& mom,
        const floorplan_citizen& dad) override
    {
        child = (rand() % 1) ? mom : dad;
    }

    void mutate(floorplan_citizen& member) override
    {
        member.mutate();
    }

private:
    //The gates the form a floorplan over.
    module* gates = nullptr;
};


/******************************************************/

std::vector<std::string> floorplan(module& partition)
{
    FloorplanGenetic algo;
    algo.setGates(&partition);
    return algo.go().getPolish();
}


#if 0
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

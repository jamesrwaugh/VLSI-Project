// Source: http://www.generation5.org/content/2003/gahelloworld.asp

#include <iostream>
#include <vector>
#include <algorithm>
#include <math.h>

template<typename Citizen>
class GeneticAlgoritm
{
public:
    GeneticAlgoritm(int popSize, int maxIter, float eliteRate, float mutateRate);

    Citizen go();   //Runs the algorithm until a Citizen has 0 fitness

protected:
    typedef std::vector<Citizen> population;

    //init_population: Initilize a population
    //calc_fitness:    Update fitness values of the entire population
    //mate:            Form a new child citizen out of two parent citizens
    //mutate:          Mutate a member of the population in some way

    virtual void init_population(population& pop) = 0;
    virtual void calc_fitness(population& pop) = 0;
    virtual void mate(Citizen& child, const Citizen& mom, const Citizen& dad) = 0;
    virtual void mutate(Citizen& member) = 0;

private:
    void mate_populations(); //Creates the next generation
    void swap_populations(); //Makes the next generation (beta) the new generation (alpha)
    void sort_by_fitness();  //Sorts current generation by fitness

private:
    int GA_POPSIZE;          // population size
    int GA_MAXITER;          // maximum iterations (generations)
    int GA_MUTATE_THRESH;    // rand() threshold for mutation
    int GA_ESIZE;            // number of citizens to move to next generation

    population pop_alpha;    //The current generation
    population pop_beta;     //The next generation
};

template<typename Citizen>
GeneticAlgoritm<Citizen>::GeneticAlgoritm(int popSize, int maxIter, float eliteRate, float mutateRate)
    : GA_POPSIZE(popSize)
    , GA_MAXITER(maxIter)
    , GA_MUTATE_THRESH(RAND_MAX * mutateRate)
    , GA_ESIZE(GA_POPSIZE * eliteRate)
    { }

template<typename Citizen>
void GeneticAlgoritm<Citizen>::mate_populations()
{
    //Elitism: The strongest survive to next generation (pop_alpha -> pop_beta)
    std::copy(pop_beta.begin(), pop_beta.begin() + GA_ESIZE, pop_alpha.begin());

    // Mate the rest (pop_alpha X pop_alpha -> pop_beta)
    for (int i=GA_ESIZE; i<GA_POPSIZE; ++i)
    {
        int i1 = rand() % (GA_POPSIZE / 2);
        int i2 = rand() % (GA_POPSIZE / 2);
        mate(pop_beta[i], pop_alpha[i1], pop_alpha[i2]);
        if(rand() < GA_MUTATE_THRESH)
            mutate(pop_beta[i]);
    }
}

template<typename Citizen>
void GeneticAlgoritm<Citizen>::swap_populations()
{
    std::swap(pop_alpha, pop_beta);
}

template<typename Citizen>
void GeneticAlgoritm<Citizen>::sort_by_fitness()
{
    std::sort(pop_alpha.begin(), pop_alpha.end(), [](Citizen& a, Citizen& b) { return a.fitness < b.fitness; });
}

template<typename Citizen>
Citizen GeneticAlgoritm<Citizen>::go()
{
    init_population(pop_alpha);
    pop_beta.resize(pop_alpha.size());

    for (int i=0; i<GA_MAXITER; ++i)
    {
        calc_fitness(pop_alpha);    // calculate fitness
        sort_by_fitness();          // sort them

        //See if the best citizen has reached the target
        if(pop_alpha.front().fitness <= 1)
            break;

        mate_populations();   // mate the population together
        swap_populations();   // swap buffers
    }

    return pop_alpha.front();
}

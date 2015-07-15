#include <algorithm>
#include <tuple>
#include <iostream>
#include <set>
#include <thread>
#include <future>
#include <ciso646>
#include "utility.h"
#include "kerninghan.h"

//Type definitions used in this file
typedef unsigned int gate;
typedef std::vector<int>  vint;
typedef std::vector<vint> vvint;
typedef std::set<gate>    partition;
typedef std::tuple<gate,gate,int> swappair;

class KernighanLinSolver
{
public:
    KernighanLinSolver(const vvint& matrix)
    {
        initPartitions(matrix);
        initConnections(matrix);
        solve(matrix);
    }
    
    operator std::pair<vint,vint>() 
    {
        vint va, vb;
        for(gate g : a) 
            va.push_back(g);
        for(gate g : b) 
            vb.push_back(g);
        return std::move(std::make_pair(std::move(va), std::move(vb)));
    }
    
private:
    partition a;    //Partition A
    partition b;    //Partition B
    partition ap;   //Partition A'
    partition bp;   //Partition B'
    vint  external; //Vector of # external wires for each gate
    vint  internal; //Vector of # internal wires for each gate
    vint  swapped;  //WHo's been swapped?
    vint  d_values; //Calculated D values (external[g] - internal[g])
    vvint gains;    //gain values matrix
    
private:
    void initPartitions(const vvint& matrix)
    {
        //We start with a random partition of the gates'
        int n2 = matrix.size() / 2;
        int n  = matrix.size();
        for(int i = 0; i != n2; ++i)
            a.insert(i);
        for(int i = n2; i != n; ++i)
            b.insert(i);
    }

    void initConnections(const vvint& matrix)
    {
        //Initializing and filling internal and external connections
        int numGates = matrix.size();
        external.resize(numGates, 0);
        internal.resize(numGates, 0);
        swapped.resize(numGates, 0);
        recalculateWireCosts(matrix);
        
        //Initializing D and gain values connections
        d_values.resize(numGates);
        gains.resize(numGates);
        for(vint& row : gains) row.resize(numGates);

        //Fill in D values for partition a and b, and gain matrix
        recomputeDValues(matrix);
        recalculateGains(matrix);
    }
    
    void recalculateWireCosts(const vvint& matrix)
    {
        std::fill(internal.begin(), internal.end(), 0);
        std::fill(external.begin(), external.end(), 0);
        unsigned numGates = matrix.size();
        for(gate g = 0; g != numGates; ++g) 
        {
            const partition& mine = (a.find(g) != a.end()) ? a : b;
            const vint& row = matrix[g];
            for(gate connection = 0; connection != row.size(); ++connection) {
                if((!row[connection]) || (g == connection))
                    continue;
                if(mine.find(connection) != mine.end()) {
                    internal[g] += 1;
                } else {
                    external[g] += 1;
                }
            }
        }
    }
    
    void recomputeDValues(const vvint& matrix)
    {
        recalculateWireCosts(matrix);
        for(gate g : a)
            d_values[g] = getDValue(g);        
        for(gate g : b)
            d_values[g] = getDValue(g);    
    }

    int getDValue(gate which)
    {
        return external[which] - internal[which];
    }

    int getSwapGain(gate a, gate b, const vvint& matrix)
    {
        return getDValue(a) + getDValue(b) - 2*matrix[a][b];
    }
    
    swappair getBestSwapPair()
    {
        gate max_i = 0, max_j = 0;
        int  max_value = -99;

        for(gate i : ap) 
        for(gate j : bp)
            if(gains[i][j] > max_value && not(swapped[i]) && not(swapped[j])) {
                max_i = i;
                max_j = j;
                max_value = gains[i][j];
            }

        return swappair(max_i, max_j, max_value);
    }
    
    std::pair<int,int> getBestPartialSumKG(std::vector<swappair>& swapPair)
    {
        std::vector<int> gis(swapPair.size());
        std::vector<int> sums(swapPair.size());
        
        //Extracts all gi values into `sums`
        std::transform(swapPair.begin(), swapPair.end(), gis.begin(), [](swappair& p){return std::get<2>(p);});
        
        //Calculates all partial sums across gv
        std::partial_sum(gis.begin(), gis.end(), sums.begin());

        //We need the maximum one
        auto it = std::max_element(sums.begin(), sums.end());
        
        int k_best =  it - sums.begin();
        int g_best = gis[k_best];
        return {k_best, g_best};
    }
    
    void recalculateGains(const vvint& matrix)
    {
        for(auto& row : gains)
            std::fill(row.begin(), row.end(), 0);
        for(gate g : a) 
            for(gate h : b) {
                gains[g][h] = getSwapGain(g, h, matrix);
                gains[h][g] = getSwapGain(h, g, matrix);    
            }
    }
    
    void recalculateDValues(gate rm_a, gate rm_b, const vvint& matrix)
    {
        recalculateWireCosts(matrix);
        for(gate x : ap) {
            if(!(matrix[x][rm_a] or matrix[x][rm_b]))
                continue;
            d_values[x] = getDValue(x) + 2*matrix[x][rm_a] - 2*matrix[x][rm_b];
        }
        for(gate y : bp) {
            if(!(matrix[y][rm_a] or matrix[y][rm_b]))
                continue;        
            d_values[y] = getDValue(y) + 2*matrix[y][rm_b] - 2*matrix[y][rm_a];    
        }
        recalculateGains(matrix);
    }
    
    void solve(const vvint& matrix)
    {
        while(1)
        {
            //"Queue" of maximum gain pairs; stores av, bv, and gv
            std::vector<swappair> swapPairs;
            
            //Initializes A' and B' to the full partitions
            ap = a; bp = b;
            
            for(unsigned i = 1; i != matrix.size()/2; ++i)
            {
                auto swapPair = getBestSwapPair();
                gate rm_a = std::get<0>(swapPair);
                gate rm_b = std::get<1>(swapPair);            
                ap.erase(rm_a);
                bp.erase(rm_b);
                swapPairs.emplace_back( swapPair );        

                /* Else we need to update the D values for all gates that were connected to
                 * rm_a and rm_b */
                recalculateDValues(rm_a, rm_b, matrix);            
            }

            /* After A' and B' are empty, we find a k to maximize g_max, the sum of gv[1],...,gv[k].
             * Then if g_max > 0, from 0 to k av and bv are swapped in a and b--the original partitions */             
            auto kg_pair = getBestPartialSumKG(swapPairs);
            int k_max = kg_pair.first;
            int g_max = kg_pair.second;
            if(g_max > 0) {
                for(int i = 0; i != k_max+1; ++i) {
                    gate swap_a = std::get<0>(swapPairs[i]);
                    gate swap_b = std::get<1>(swapPairs[i]);    
                    swapped[swap_a] = 1;
                    swapped[swap_b] = 1;
                    a.erase(swap_a);
                    b.erase(swap_b);
                    a.insert(swap_b);
                    b.insert(swap_a);
                }
                recomputeDValues(matrix);
            } else {
                break;
            }
        }
    }
};


/************************************************************************/

std::pair<std::vector<int>,std::vector<int>> kernighanLinSolve(const std::vector<std::vector<int>>& matrix)
{
    return KernighanLinSolver(matrix);
}

typedef std::pair<int,   vint> connpair;
typedef std::pair<int,stdcell> cellpair;

//After the module is partitioned into submodules, the IOs are broken by `rebuildModule`
//because they are directly copied from the src, and gates are removed from src.
//So gate connections between modules are broken. This function fixes this problem.
void fixIOGates(module& m)
{
    auto& realIns = m.gates[0].outputs;
    auto& realOuts = m.gates[1].inputs;
    auto inputs  = realIns;
    auto outputs = realOuts;
    realIns.clear();
    realOuts.clear();

    //Accumate a vector of all gates' inputs and outputs
    auto allIns  = m.gates[2].inputs;
    auto allOuts = m.gates[2].outputs;
    for(unsigned i = 3; i < m.gates.size(); ++i) {
        const stdcell& gi = m.gates[i];
        allIns.insert ( allIns.end(), gi.inputs.begin(),  gi.inputs.end());
        allOuts.insert(allOuts.end(), gi.outputs.begin(), gi.outputs.end());
    }

    //We need to sort them to use set functions. Also remove duplicates from allI/allO
    std::sort(inputs.begin(), inputs.end());
    std::sort(outputs.begin(), outputs.end());
    std::sort(allIns.begin(), allIns.end());
    std::sort(allOuts.begin(), allOuts.end());
    allIns.resize(std::unique(allIns.begin(), allIns.end()) - allIns.begin());
    allOuts.resize(std::unique(allOuts.begin(), allOuts.end()) - allOuts.begin());

    //The IOs we want to keep are present in both allIns/allOuts and original non-partitioned inputs/outputs
    std::set_intersection(allIns.begin(),   allIns.end(),  inputs.begin(),  inputs.end(), std::back_inserter(realIns));
    std::set_intersection(allOuts.begin(), allOuts.end(), outputs.begin(), outputs.end(), std::back_inserter(realOuts));

    //All present in outputs that aren't present in any gate's input are new outputs of the module
    std::set_difference(allOuts.begin(), allOuts.end(), allIns.begin(), allIns.end(), std::back_inserter(realOuts));

    //All present in inputs that don't come from any output are new inputs of the module
    std::set_difference(allIns.begin(), allIns.end(), allOuts.begin(), allOuts.end(), std::back_inserter(realIns));

    //The previous set operations may have duplicated things--resort and unique the ranges
    std::sort(realIns.begin(), realIns.end());
    std::sort(realOuts.begin(), realOuts.end());
    realIns.resize(std::unique(realIns.begin(), realIns.end()) - realIns.begin());
    realOuts.resize(std::unique(realOuts.begin(), realOuts.end()) - realOuts.begin());
}

//KL gives us only a list of gates like [1, 3, 4, 5]. `rebuildModule` builds a proper `module`
//using the list from KL, cutting from src (original) to partitioned (dest), where `gates`
//is the output from KL that should be in the partition.
void rebuildModule(module& dest, const vint& gates, const module& src)
{
    dest.name = src.name;

    //Allocate space for the new connectivity matrix
    int nGates = gates.size();
    dest.connections.resize(nGates);
    for(vint& row : dest.connections)
        row.resize(nGates);

    for(int i = 0; i != nGates; ++i)
    {
        //+2 to avoid shit about I/Os being first gates...
        //Beacause KL does not see the first two gates
        gate g = gates[i];

        //Copy a gate `g` over
        dest.gates.push_back(src.gates[g]);

        //We are rebuilding the connectivity matrix to only those gates
        //we should be keeping. We are transforming [g][h] into [i][j]
        for(int j = 0; j != nGates; ++j) {
            gate h = gates[j];
            dest.connections[i][j] += src.connections[g][h];
        }
    }

    //Now we need to fix IO gates (gates[0] and [1]) because some gates were removed
    fixIOGates(dest);
}

//Remedy for wires wires that connect to both the internal and external partitions
void fixInterPartitionWires(module& r0, module& r1, const module& src)
{
    //Module inputs that are not from src and DO NOT have wires from the other partition
    //are added as outputs from the other partition
    /* How this is done:
     * - push back (r0.inputs - src.inputs) into outputs
     * - merge the seperated two ranges (outputs and added range) inside output
     * - remove duplicates from outputs */
    auto& inputs = r0.gates[0].outputs;
    auto& outputs = r1.gates[1].inputs;
    const auto& srcins = src.gates[0].outputs;
    size_t oldsize = outputs.size();

    std::set_difference(inputs.begin(), inputs.end(), srcins.begin(), srcins.end(), std::back_inserter(outputs));
    std::inplace_merge(outputs.begin(), outputs.begin() + oldsize, outputs.end());
    outputs.resize(std::unique(outputs.begin(), outputs.end()) - outputs.begin());
}

/****************************************************************/

//Remedy for inputs and outputs gates being gates 0 and 1
void removeIOGates(module& m)
{
    auto& conns = m.connections;
    auto& gates = m.gates;
    conns.erase(conns.begin(), conns.begin() + 2);
    gates.erase(gates.begin(), gates.begin() + 2);
    for(auto& row : conns)
        row.erase(row.begin(), row.begin() + 2);
}

//Remedy. KL gives back a vint, we just insert 0 and 1 to say IO gates are there too
//+2 becasue KL retutning the 0th gate is actaully the 2nd gate (no IO gates in KL)
void insertIOGates(vint& partition)
{
    for(int& gate : partition)
        gate += 2;
    partition.insert(partition.begin(), 1);    //Output gate
    partition.insert(partition.begin(), 0);    //Inptus gate
}

/** Toplevel Kernighan Lin function **/
std::pair<module, module> kernighanLin(const module& m)
{
    module r0, r1;
    module m_kl = m;

    //I/O gates are hidden from the KL algorithm...
    removeIOGates(m_kl);
    auto partitions = kernighanLinSolve(m_kl.connections);

    //...Then we are inseting them back
    insertIOGates(partitions.first);
    insertIOGates(partitions.second);

    //After partition, the I/O gates are terribly broken.
    rebuildModule(r0, partitions.first, m);
    rebuildModule(r1, partitions.second, m);
    fixInterPartitionWires(r0, r1, m);
    fixInterPartitionWires(r1, r0, m);

    return std::make_pair(std::move(r0), std::move(r1));
}

/****************************************************************/

std::pair<int,int> getModuleDimentions(const module& m, const PadframeFile& pad)
{
    int w=0, h=0, max_h=0;
    int slicewidth = (pad.usableWidth() / pad.slicesHoriz()) * 0.75;

    //Initial: width is sum of all widths, height is highest gate's height
    for(const stdcell& g : m.gates)
        w += g.length;
    max_h = std::max_element(m.gates.begin(), m.gates.end(),
            [](const stdcell& a, const stdcell& b) { return a.width < b.width; })->width;

    /* h is the highest gate's height times the number of times w goes over
     * the slice wdith, or is only max_h if it does not go over */
    h = max_h * std::max(1, w/slicewidth);
    w -= slicewidth * (h / max_h);


    return std::make_pair(w,h);
}

std::vector<module> kerninghanLinPadframeSlice(const module &m, const PadframeFile &f)
{
    std::vector<module> result;

    int sliceWidth  = f.usableWidth()  / f.slicesHoriz();
    int sliceHeight = f.usableHeight() / f.slicesVert();
    std::pair<int,int> dimensions = getModuleDimentions(m,f);

    /* If the module is bigger than the padframe slice, we recursively
     * partition it into two. Otherwise, return the entire module (base case) */

    if(dimensions.first > sliceWidth || dimensions.second > sliceHeight)
    {
        //Apply KL on this thread
        auto partitions = kernighanLin(m);

        //Spawn two threads to partition the two partitions
        auto future0 = std::async(kerninghanLinPadframeSlice, partitions.first,  f);
        auto future1 = std::async(kerninghanLinPadframeSlice, partitions.second, f);

        //Wait for the threads and combine their results
        std::vector<module> parts0 = future0.get();
        std::vector<module> parts1 = future1.get();
        result.insert(result.end(), parts0.begin(), parts0.end());
        result.insert(result.end(), parts1.begin(), parts1.end());
    }
    else {
        //Base case: Module size is within slice w/h, we return only that module
        result.push_back(std::move(m));
    }

    return result;
}

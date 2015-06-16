#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include "utility.h"
#include "output.h"

//To make life a bit easier in this file
typedef std::pair<module,module> partition;

void tryAppend(std::string& target, const std::string& candidate)
{
    if(target.find(candidate) == std::string::npos)
        target.append(" ").append(candidate).append(",");
}


void getSubcktWireLines(const module& partition, 
    std::string& out_inputs, std::string& out_output, std::string& out_wires)
{
    std::string inputLine, outputLine, wireLine;
    const auto& inputs  = partition.gates[0].outputs;
    const auto& outputs = partition.gates[1].inputs;

    if(!inputs.empty())
        inputLine = "  .IN";
    if(!outputs.empty())
        outputLine = "  .OUT";
    
    /* We look though each gate's inputs and outputs. If they do not tie to the
     * module's inputs or outputs, they are wires that connect to other gates.
     * We need to declare each of these */
    for(unsigned i = 2; i < partition.gates.size(); ++i)
    {
        const stdcell& g = partition.gates[i];

        for(const std::string& pin : g.inputs) {
            bool found = std::find(inputs.begin(), inputs.end(), pin) != inputs.end();
            tryAppend((found) ? inputLine : wireLine, pin);
        }
        for(const std::string& pin : g.outputs) {
            bool found = std::find(outputs.begin(), outputs.end(), pin) != outputs.end();
            tryAppend((found) ? outputLine : wireLine, pin);
        }
    }
    
    if(!wireLine.empty())
        wireLine.insert(0, "  .WIRE").pop_back();   //If wires added, insert WIRE and remove last comma
    if(!inputs.empty())
        inputLine.pop_back();   //Removing last comma
    if(!outputs.empty())
        outputLine.pop_back();  //Removing last comma
        
    out_inputs = std::move(inputLine);
    out_output = std::move(outputLine);
    out_wires  = std::move(wireLine);
}


void getSubcktGateLine(const stdcell& gate, int count, const MattCellFile& cells, std::string& out)
{
    std::stringstream ss;
    ss << "  x" << count << "\t" << std::left << std::setw(6) << gate.name;
    
    const stdcell& cell = cells[gate.name];    //To get standard information (the A part below)
    
    //Give a .A(B) string for each input/output and its attachment. Bad duplicated code.
    for(unsigned i = 0; i != gate.inputs.size(); ++i) {
        std::string connection = ".";
        connection.append(cell.inputs[i]).append("(").append(gate.inputs[i]).append(")");
        ss << std::left << std::setw(12) << connection;
    }
    for(unsigned i = 0; i != gate.outputs.size(); ++i) {
        std::string connection = ".";
        connection.append(cell.outputs[i]).append("(").append(gate.outputs[i]).append(")");
        ss << std::left << std::setw(12) << connection;
    }

    out = std::move(ss.str());
}

/* Given a module, returns the .subckt text as a string to be written in
 * the slice .subckts file. `sliceNum` is the number of the slice containing the
 * partition, and `partitionNum` is the number of the partition in the slice */
std::string getSubcktText(const module& partition, const MattCellFile& cells, int sliceNum, int partitionNum)
{
    std::stringstream ss;
    
    //The initial comment and ".subckt Px_YY" line
    ss << "#subckt describing partition "   << partitionNum << " in slice " 
       << std::setfill('0') << std::setw(2) << sliceNum << '\n'; 
    ss << ".subckt P" << sliceNum  << "_"  << std::setfill('0') << std::setw(2) << partitionNum;

    //The a.I, b.I, y.O etc text after name
    for(const std::string& s : partition.gates[0].outputs)
        ss << " " << s << ".I";
    for(const std::string& s : partition.gates[1].inputs)
        ss << " " << s << ".O";
    ss << '\n';

    //.IN, .OUT, and .WIRE lines
    std::string inputLine, outputLine, wireLine;
    getSubcktWireLines(partition, inputLine, outputLine, wireLine);
    if(!inputLine.empty())  ss << inputLine  << '\n';
    if(!outputLine.empty()) ss << outputLine << '\n';
    if(!wireLine.empty())   ss << wireLine   << '\n';
    
    //Gate lines
    for(unsigned i = 2; i < partition.gates.size(); ++i) {
        const stdcell& gate = partition.gates[i];
        std::string s;
        getSubcktGateLine(gate, i-2, cells, s);
        ss << s << '\n';
    }
    
    //Ending syntax
    ss << ".end_subckt" << '\n';
    return ss.str();
}

std::string getSubcktText(const partition& p, const MattCellFile &cells, int sliceNum, int partitionNum)
{
    std::stringstream ss;
    ss << getSubcktText(p.first, cells, sliceNum, partitionNum) << std::endl;
    ss << getSubcktText(p.second, cells, sliceNum, partitionNum+1);
    return ss.str();
}

int getExternWireCost(const module& a, const module& b)
{
    std::vector<std::string> wires;
    const auto& ins0  = a.gates[0].outputs;
    const auto& ins1  = b.gates[0].outputs;
    const auto& outs0 = a.gates[1].inputs;
    const auto& outs1 = b.gates[1].inputs;

    //External wire cost is easily determined as intersection between
    //the inputs and outputs across modules
    std::set_intersection(ins0.begin(), ins0.end(), outs1.begin(), outs1.end(), std::back_inserter(wires));
    std::set_intersection(ins1.begin(), ins1.end(), outs0.begin(), outs0.end(), std::back_inserter(wires));

    //`wires` now contains all external wiring between a and b
    return wires.size();
}

/****************************************************************************/

SubcktFile::SubcktFile(const std::string &filename, int sliceNum, const MattCellFile &cells)
    : sliceNumber(sliceNum)
    , partitionNumber(0)
    , cellsRef(cells)
{
    file.open(filename);
    if(!file.is_open()) {
        error("Could not open .subckt file ", filename, " for writing");
    }
}

SubcktFile::~SubcktFile()
{
    file.close();
}

std::string SubcktFile::getHeaderText(const std::pair<module,module>& p)
{
    char buffer[32];
    std::stringstream ss;

    const module& a = p.first;
    const module& b = p.second;
    int cost = getExternWireCost(a, b);
    std::sprintf(buffer, "#External wiring for P%d_%02d,P%d_%02d: %d\n",
        sliceNumber, partitionNumber, sliceNumber, partitionNumber+1, cost);
    ss << buffer;
    std::sprintf(buffer, "#Gate counts: %lu,%lu\n", a.gates.size()-2, b.gates.size()-2);
    ss << buffer;

    return ss.str();
}

std::ostream& SubcktFile::operator<<(const std::pair<module,module>& partitions)
{
    file << getHeaderText(partitions) << std::endl;
    file << getSubcktText(partitions, cellsRef, sliceNumber, partitionNumber) << std::endl;
    partitionNumber += 2;
    return file;
}

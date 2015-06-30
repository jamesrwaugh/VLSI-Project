#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include "utility.h"
#include "output.h"

void getSubcktWireLines(const module& partition,
    std::string& out_inputs, std::string& out_output, std::string& out_wires)
{
    std::string inputLine, outputLine, wireLine;
    const auto& inputs  = partition.gates[0].outputs;
    const auto& outputs = partition.gates[1].inputs;

    //Add initial .IN and .OUT if we are going to be adding something
    if(!inputs.empty())  inputLine = ".IN";
    if(!outputs.empty()) outputLine = ".OUT";

    //Copy the module's inputs/outputs to the .IN and .OUT lines
    for(const std::string& item :  inputs)
         inputLine.append(" ").append(item).append(",");
    for(const std::string& item : outputs)
        outputLine.append(" ").append(item).append(",");
    
    /* We look though each gate's inputs and outputs. If they do not tie to the
     * module's inputs or outputs, they are wires that connect to other gates,
     * and are added to the .WIRE line if they are not there already */
    auto tryAddToWireLine =
    [&](const std::string& pin, const std::vector<std::string>& io) {
        if((std::find(io.begin(), io.end(), pin) == io.end()) && (wireLine.find(pin) == std::string::npos))
            wireLine.append(" ").append(pin).append(",");
    };

    for(unsigned i = 2; i < partition.gates.size(); ++i) {
        for(const std::string& pin : partition.gates[i].inputs)
            tryAddToWireLine(pin, inputs);
        for(const std::string& pin : partition.gates[i].outputs)
            tryAddToWireLine(pin, outputs);
    }
    
    //Removing last commas on the strings.
    //And if wires is not empty, it needs .WIRE inserted
    if(!wireLine.empty())
        wireLine.insert(0, ".WIRE").pop_back();
    if(!inputs.empty())
        inputLine.pop_back();
    if(!outputs.empty())
        outputLine.pop_back();
        
    out_inputs = std::move(inputLine);
    out_output = std::move(outputLine);
    out_wires  = std::move(wireLine);
}


std::string getSubcktGateLine(const stdcell& gate, const MattCellFile& cells, int count)
{
    char buffer[64];
    std::stringstream ss;

    //Write gate name. ex: "x0 nand2"
    std::sprintf(buffer, "x%-3d %-6s ", count, gate.name.c_str());
    ss << buffer;
    
    //To get standard information (the A part below)
    const stdcell& cell = cells[gate.name];

    //Give a ".A(B)" string for each input/output and its attachment. Bad duplicated code.
    for(unsigned i = 0; i != gate.inputs.size(); ++i) {
        std::sprintf(buffer, ".%s(%s)", cell.inputs[i].c_str(), gate.inputs[i].c_str());
        ss << std::left << std::setw(12) << buffer;
    }
    for(unsigned i = 0; i != gate.outputs.size(); ++i) {
        std::sprintf(buffer, ".%s(%s)", cell.outputs[i].c_str(), gate.outputs[i].c_str());
        ss << std::left << std::setw(12) << buffer;
    }

    return ss.str();
}

/* Given a module, returns the .subckt text as a string to be written in
 * the slice .subckts file. `sliceNum` is the number of the slice containing the
 * partition, and `partitionNum` is the number of the partition in the slice */
std::string getSubcktTextSingle(const module& partition, const MattCellFile& cells, int sliceNum, int partitionNum)
{
    std::stringstream ss;
    std::string inputLine, outputLine, wireLine;

    /* The format of a .subkt is this:
     * #subckt describing partition YY in slice X
     * .subckt PX_YY [Inputs].I [Outputs].O
     *   .IN [Inputs]
     *   .OUT [Outputs]
     *   .WIRE [Wires]
     *   x0 [Gate]
     *   x1 [Gate]
     *   ....
     *   xN [Gate]
     * .end_subckt
     */
    
    //The initial comment and ".subckt Px_YY" line
    char buffer[64];
    std::sprintf(buffer, "#subckt describing partition %d in slice %02d\n", partitionNum, sliceNum);
    ss << buffer;
    std::sprintf(buffer, ".subckt P%d_%02d", sliceNum, partitionNum);
    ss << buffer;

    //The [Inputs] [Outputs] text after name
    for(const std::string& s : partition.gates[0].outputs)  ss << " " << s << ".I";
    for(const std::string& s : partition.gates[1].inputs)   ss << " " << s << ".O";
    ss << '\n';

    //.IN, .OUT, and .WIRE lines
    getSubcktWireLines(partition, inputLine, outputLine, wireLine);
    if(!inputLine.empty())  ss << " " << inputLine  << '\n';
    if(!outputLine.empty()) ss << " " << outputLine << '\n';
    if(!wireLine.empty())   ss << " " << wireLine   << '\n';
    
    //x0..xN [Gate] Gate lines
    for(unsigned i = 2; i < partition.gates.size(); ++i)
        ss << " " << getSubcktGateLine(partition.gates[i], cells, i-2) << '\n';
    
    //Ending syntax
    ss << ".end_subckt" << '\n';
    return ss.str();
}

std::string getSubcktText(const std::pair<module,module>& p, const MattCellFile &cells, int sliceNum, int partitionNum)
{
    std::stringstream ss;
    ss << getSubcktTextSingle(p.first,  cells, sliceNum, partitionNum) << std::endl;
    ss << getSubcktTextSingle(p.second, cells, sliceNum, partitionNum+1);
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

std::string SubcktFile::getHeaderText(const std::pair<module,module>& p)
{
    char buffer[32];
    std::stringstream ss;

    const module& a = p.first;
    const module& b = p.second;
    int cost = getExternWireCost(a, b), pn = partitionNumber, sn = sliceNumber;

    std::sprintf(buffer, "#External wiring for P%d_%02d,P%d_%02d: %d\n", sn, pn, sn, pn+1, cost);
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

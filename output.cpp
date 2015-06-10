#include <iostream>
#include <sstream>
#include <ios>
#include <iomanip>
#include <algorithm>
#include "output.h"

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
    for(const stdcell& g : partition.gates) 
    {
        for(const std::string& pin : g.inputs) {
            bool found = std::find(inputs.begin(), inputs.end(), pin) != inputs.end();
            if(found) tryAppend(inputLine,pin); 
                else  tryAppend(wireLine ,pin);
        }
        for(const std::string& pin : g.outputs) {
            bool found = std::find(outputs.begin(), outputs.end(), pin) != outputs.end();
            if(found) tryAppend(outputLine,pin); 
                else  tryAppend(wireLine ,pin);
        }
    }
    
    if(!wireLine.empty())    //If wires aded, insert WIRE and remove last comma
        wireLine.insert(0, "  .WIRE").pop_back();
    if(!inputs.empty())        //Removing last comma
        inputLine.pop_back();
    if(!outputs.empty())    //Removing last comma
        outputLine.pop_back();        
        
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
    for(int i = 0; i != gate.inputs.size(); ++i) {
        std::string connection = ".";
        connection.append(cell.inputs[i]).append("(").append(gate.inputs[i]).append(")");
        ss << std::left << std::setw(12) << connection;
    }
    for(int i = 0; i != gate.outputs.size(); ++i) {
        std::string connection = ".";
        connection.append(cell.outputs[i]).append("(").append(gate.outputs[i]).append(")");
        ss << std::left << std::setw(12) << connection;
    }

    out = std::move(ss.str());
}


std::string getSubcktText(const module& partition, const MattCellFile& cells, int sliceNum, int partitionNum)
{
    std::stringstream ss;
    
    //The initial comment and ".subckt Px_YY" line
    ss << "#subckt describing partition "   << partitionNum << " in slice " 
       << std::setfill('0') << std::setw(2) << sliceNum << '\n'; 
    ss << ".subckt P" << 0  << "_"  << std::setfill('0') << std::setw(2) << 1;

    //The a.I, b.I, y.O etc text after name
    for(const std::string& s : partition.gates[0].outputs)
        ss << " " << s << ".I";
    for(const std::string& s : partition.gates[1].inputs)
        ss << " " << s << ".O";
    ss << '\n';

    //.IN, .OUT, and .WIRE lines
    std::string dec_inputs, dec_outputs, dec_wires;
    getSubcktWireLines(partition, dec_inputs, dec_outputs, dec_wires);
    ss << dec_inputs << '\n' << dec_outputs << '\n' << dec_wires << '\n';
    
    //Gate lines
    for(int i = 2; i < partition.gates.size(); ++i) {
        const stdcell& gate = partition.gates[i];
        std::string s;
        getSubcktGateLine(gate, i-2, cells, s);
        ss << s << '\n';
    }
    
    //Ending syntax
    ss << ".end_subckt" << '\n';
    return ss.str();
}

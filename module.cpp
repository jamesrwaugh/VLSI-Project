#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <fstream>
#include "utility.h"
#include "stdcell.h"
#include "module.h"

void cellIO(std::vector<module>& m)
{
    //go through all structures
    for(int i=0; i<m.size(); ++i)
    {
        //Get gates and resize the connectivity matirx
        std::vector<stdcell>& g = m[i].gates;
        m[i].connections.resize(g.size());
        for(std::vector<int>& row : m[i].connections)
            row.resize(g.size());

        //for each gates in module m[i]
        for(int j=0; j<g.size(); ++j)
        {
            //for each gate output, find a gate with the proper input
            for(int k=0; k<g[j].outputs.size(); ++k)
            {
                //check array of gates in this model
                for(int l=0; l<g.size(); ++l)
                {
                    //for each input
                    for(int n=0; n<g[l].inputs.size(); ++n)
                    {
                        if(g[j].outputs[k] == g[l].inputs[n])
                        {
                            //Here we've found a match from an output to an input
                            m[i].connections[j][l] += 1;
                            m[i].connections[l][j] += 1;    //To be symmetric
                        }
                    }
                }
            }
        }
    }
}

std::pair<std::string, std::string> getGateIONames(const std::string& fullIOName)
{
    size_t equalPos   = fullIOName.find('=');
    size_t bracketPos = fullIOName.find('[');
    std::string gateName = fullIOName.substr(0, equalPos);
    std::string connName;
    if(bracketPos != std::string::npos) {
        connName = fullIOName.substr(bracketPos+1, fullIOName.find(']') - (bracketPos+1));
    } else {
        connName = fullIOName.substr(equalPos+1, std::string::npos);
    }
    return std::make_pair(gateName, connName);
}

std::vector<module> readModuleFile(const std::string& fileName, MattCellFile& cells)
{
    static const std::string delim = " \t";

    std::vector<module> allModels;
    module tmpModel;
    std::ifstream stream(fileName);
    std::string line;
    
    while(getline_fixed(stream, line))
    {
        if(line.find(".model") != std::string::npos)
        {
            std::vector<std::string> tmpName = Split(line, delim);
            tmpModel.name = tmpName[1];
        }
        else if(line.find(".inputs") != std::string::npos)
        {
            stdcell tmpCell;
            std::vector<std::string> inputInfo = Split(line, delim);
            tmpCell.name = "inputs";
            for(int i=1; i<inputInfo.size(); ++i) {
                tmpCell.outputs.push_back(inputInfo[i]);
            }
            tmpModel.gates.push_back(tmpCell);
        }
        else if(line.find(".outputs") != std::string::npos)
        {
            stdcell tmpCell;
            std::vector<std::string> outputInfo = Split(line, delim);
            tmpCell.name = "outputs";
            for(int i=1; i<outputInfo.size(); ++i) {
                tmpCell.inputs.push_back(outputInfo[i]);
            }
            tmpModel.gates.push_back(tmpCell);
        }
        else if(line.find(".gate") != std::string::npos)
        {
            stdcell tmpCell;
            std::vector<std::string> gateInfo = Split(line, delim);
            tmpCell.name = gateInfo[1];

            for(int i=2; i<gateInfo.size(); ++i)
            {
                //Map lookup standard cell information and fill tmpCell
                const stdcell& cell = cells[tmpCell.name];    
                tmpCell.width = cell.width;
                tmpCell.length = cell.length;
                const std::vector<std::string>& outs = cell.outputs;

                //Parses the A=[B] or A=B string into "gateName" A and "connectName" B
                auto connectName = getGateIONames(gateInfo[i]);
                
                /* Look for the "gateName" in the cell's gate outputs. If it is there, then it is connected
                 * to `connectName` through that gate pin. Push back into outputs/inputs 
                 */
                if(std::find(outs.begin(), outs.end(), connectName.first) != outs.end()) {
                    tmpCell.outputs.push_back(connectName.second);
                }  else {
                    tmpCell.inputs.push_back(connectName.second);
                }
            }

            tmpModel.gates.push_back(tmpCell);
        }
        else if(line.find(".end") != std::string::npos)
        {
            allModels.push_back(tmpModel);
            tmpModel.name = "";
            tmpModel.gates.clear();
        }
    }
    
    cellIO(allModels);

    return allModels;
}

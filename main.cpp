// argv[1]:   input.txt
#include "Def.h"
using namespace std;

int main(int argc, char* argv[]){
    vector<baseStation> BS_list;
    vector< vector<string> > cmd;
    vector< vector<int> > cmdIdx;
    int MAX_round=1;
    // Read Input //
    if(readInputOpt(argv[1],BS_list)==false)
        return 0;
    
    // Generate possible command //
    cmdGenerate(BS_list,cmd);
    
    // Generate combination of command //
    MAX_round=cmdComboGen(cmd,cmdIdx);
    
    // Optimization Process //
    for(int round_idx=0;round_idx<MAX_round;round_idx++){
        // Set Pa command //
        setPaCmd(BS_list,cmd,cmdIdx,round_idx);
        
        // Schedule //
        RBalloc(BS_list);
        
        // Calc Sub-band SINR of all UEs //
        calcsubSINR(BS_list);
        
        // Calc avg. SINR //
        calcavgSINR(BS_list);
        
        // Select UE CQI by SINR //
        for(int i=0;i<BS_list.size();i++)
            for(int j=0;j<BS_list[i].UE_list.size();j++)
                BS_list[i].UE_list[j].CQI=selectCQI(BS_list[i].UE_list[j].avgSINR);
        
        // Record Result//
        // TODO .... //
        showUEinfo(BS_list);
        
        // Initialization //
        initBSlist(BS_list);
    }
    
    //showUEsinr(BS_list);
    //showUEallocRB(BS_list);
    //showBSinfo(BS_list);
    
    return 0;
}
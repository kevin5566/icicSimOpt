// argv[1]:   input.txt
#include "Def.h"
using namespace std;

int main(int argc, char* argv[]){
    vector<baseStation> BS_list;
    vector< vector<string> > cmd;
    vector< vector<int> > cmdIdx;
    vector< vector<UEinfo> > DATA;
    int MAX_round=0;
    // Read Input //
    if(readInputOpt(argv[1],BS_list)==false)
        return 0;
    
    // Generate possible command //
    cmdGenerate(BS_list,cmd);
    
    // Calc RSRP //
    //calcRSRP(BS_list);
    
    // Generate combination of command //
    MAX_round=cmdComboGen(cmd,cmdIdx);
    MAX_round=3;
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
        
        // Select UE MCS by SINR //
        for(int i=0;i<BS_list.size();i++)
            for(int j=0;j<BS_list[i].UE_list.size();j++)
                BS_list[i].UE_list[j].MCS=selectMCS(BS_list[i].UE_list[j].avgSINR);
        
        // Record Result//
        saveUEinfo(BS_list,DATA);
        
        // Initialization //
        initBSlist(BS_list);
    }
    
    // Show all combination of RB cmd result //
    //showAllresult(DATA);
    
    showGJresult(DATA,cmd,cmdIdx,0);
    showGJresult(DATA,cmd,cmdIdx,1);
    showGJresult(DATA,cmd,cmdIdx,2);
    
    return 0;
}
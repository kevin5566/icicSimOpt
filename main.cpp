// argv[1]:   input.txt
#include "Def.h"
using namespace std;

int main(int argc, char* argv[]){
    vector<baseStation> BS_list;
    vector< vector<int> > graph;
    vector< vector<int> > cluster;
    vector< vector<string> > cmd;
    vector< vector<int> > cmdIdx;
    vector< vector<UEinfo> > DATA;
    int MAX_round=0;

    // Read Input //
    if(readInputOpt(argv[1],BS_list)==false)
        return 0;
    
    // Calc RSRP //
    calcRSRP(BS_list);
    //showUERSRP(BS_list);
    
    // Calc Initial SINR in order to Select CQI //
    calcsubSINR(BS_list);
    calcavgSINR(BS_list);
    for(int i=0;i<BS_list.size();i++)
        for(int j=0;j<BS_list[i].UE_list.size();j++)
            BS_list[i].UE_list[j].CQI=selectCQI(BS_list[i].UE_list[j].avgSINR);
    //////////////////////////////////////////////

    // Generate possible command //
    locateUE(BS_list);
    buildGraph(BS_list,graph);
    //showGraph(graph);
    selectClusterByGreedy(BS_list,graph,cluster);
    //selectClusterByDP(graph,cluster);
    //showCluster(cluster);
    //commonCmdGenerate(BS_list,cmd);
    bestCmdGenerate(BS_list,cluster,cmd);
    //cmdGenerate(BS_list,cluster,cmd);
    //showAllCmd(cmd);

    // Generate combination of command //
    MAX_round=cmdComboGen(cmd,cmdIdx);
    //MAX_round=3;

    // Optimization Process //
    for(int round_idx=0;round_idx<MAX_round;round_idx++){
        // Initialization //
        initBSlist(BS_list);
        
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
    }
    
    // Show all combination of RB cmd result //
    //showAllresult(DATA);
    //return 0;

    // Show Feasible Pa Cmd in Different Criteria //
    // Default: Maximum Total Throughput //
    //showGJresult(DATA,cmd,cmdIdx);
    // 1      : All type users achieve the similar level //
    //showGJresult(DATA,cmd,cmdIdx,1);
    // 2      : Edge Throughput Guaranteed //
    //showGJresult(DATA,cmd,cmdIdx,2);

    //showUEsinr(BS_list);
    showUEinfo(BS_list);
    //showUEallocRB(BS_list);
    //showBSinfo(BS_list);

    showAllTput(BS_list);
    showAvgCQI(BS_list);
    showAvgMCS(BS_list);
    
    return 0;
}
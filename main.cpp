// argv[1]:   input.txt
#include "Def.h"
using namespace std;

int main(int argc, char* argv[]){
    vector<baseStation> BS_list;
    vector< vector<string> > cmd;
    vector< vector<int> > cmdIdx;
    // Read Input //
    if(readInput(argv[1],BS_list)==false)
        return 0;
    
    // Generate possible command //
    cmdGenerate(BS_list, cmd);
    
    // Generate combination of command //
    cmdComboGen(cmd,cmdIdx);
    
    for(int j=0;j<cmdIdx[0].size();j++){
        for(int i=0;i<cmdIdx.size();i++)
            cout<<cmdIdx[i][j];
        cout<<endl;
    }
    
    // Read Pa command //
    /*
    for(int i=0;i<BSnum;i++){
        getline(infile,tmpline);
        for(int j=0;j<tmpline.length();j++){
            BS_list[i].RB_pa.push_back(tmpline[j]-'0');
        }
    }
    */
        
    // Schedule //
    RBalloc(BS_list);
    
    // Calc Sub-band SINR of all UEs //
    calcsubSINR(BS_list);
    
    // Calc avg. SINR //
    double SINR_max=0;
    double SINR_min=1000;
    calcavgSINR(BS_list,SINR_max,SINR_min);
    
    // Select UE CQI by SINR //
    for(int i=0;i<BS_list.size();i++)
        for(int j=0;j<BS_list[i].UE_list.size();j++)
            BS_list[i].UE_list[j].CQI=selectCQI(BS_list[i].UE_list[j].avgSINR);
    
    //showUEsinr(BS_list);
    showUEinfo(BS_list);
    //showUEallocRB(BS_list);
    //showBSinfo(BS_list);
    
    return 0;
}
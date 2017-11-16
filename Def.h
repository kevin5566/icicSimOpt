#include<vector>
#include<iostream>
#include<fstream>
#include<string>
#include<sstream>
#include<random>
#include<math.h>
#include<iomanip>
#include<numeric>
#include<algorithm>
using namespace std;

// Shadow Effect //
//default_random_engine seed1;
//lognormal_distribution<double> shadow_lognormal(0.0,3.16);
//normal_distribution<double> shadow_normal(0.0,3.16);

// Parameter //
const double  c=299792458;        // Light Speed
const double  carrierFreq=2.66*1000000000;    // (Hz) 1.9*1000000000
const double  d_0=2.0;          // ref. Distance(m)
const int     n=4;                // Path Loss Exponent
const double  N_0=-130.0;         // Noise Density(dBm/Hz)
const double  BW=10*1000000;      // Total BW(Hz)
const int     N_band=50;          // Subband num

const double  pa_level[8]={-6,-4.77,-3,-1.77,0,1,2,3};

const int level_size=106;
const int CQI_size=15;
const int MCS_map_size=24;

const int total_RBG_num = 16;

typedef enum{
    CENTER=0,
    MIDDLE=1,
    EDGE=2,
} Position;

const string p_type[3]={"Center", "Middle", "Edge"};

struct UE{
    double x;
    double y;
    Position UePosition;
    double avgSINR;
    int    CQI;
    int    MCS;
    int    pa;
    // TODO // //double RSSI;
    vector<double> RSRP;
    vector<double> subbandSINR;
    vector<bool> subbandMask;   //0: no use; 1: used
    UE(double a, double b, Position c, int d){
        x=a;
        y=b;
        UePosition=c;
        pa=d;
        avgSINR=1;
        CQI=0;
        MCS=0;
        for(int i=0;i<N_band;i++){
            subbandSINR.push_back(0);
            subbandMask.push_back(0);
        }
    }
};

struct baseStation{
    double x;
    double y;
    double power;
    int    subbandnum;
    vector<double> sub_P;
    vector<int> sub_alloc;  //-1: no use
    vector<int> RB_pa;
    vector<int> RB_pa_actual; //-1: no use
    vector<UE> UE_list;
    baseStation(double a, double b, double c){
        x=a;
        y=b;
        power=c;
        subbandnum=N_band;
        for(int i=0;i<subbandnum;i++){
            sub_P.push_back(power);
            sub_alloc.push_back(-1);
            RB_pa.push_back(-1);
            RB_pa_actual.push_back(-1);
        }
    }
};

struct UEinfo{
    int BSidx;
    int UEidx;
    Position UePosition;
    int RBnum;
    double CQI_thrghput;
    double MCS_thrghput;
    UEinfo(int i, int j, Position a, int b, double c, double d){
        BSidx=i;
        UEidx=j;
        UePosition=a;
        RBnum=b;
        CQI_thrghput=c;
        MCS_thrghput=d;
    }
};

// Function //

// Return strg value (i.e. Power-PathLoss, unit: dBm) //
double getStrg(vector<baseStation> BS_list, int i, int j, int k, int l, bool isRS);

bool readInput(char* ptr, vector<baseStation> &BS_list);
bool readInputOpt(char* ptr, vector<baseStation> &BS_list);
void calcRSRP(vector<baseStation> &BS_list);
void cmdGenerate(vector<baseStation> BS_list, vector< vector<string> > &cmd);
int cmdComboGen(vector< vector<string> > cmd, vector< vector<int> > &cmdIdx);
void setPaCmd(vector<baseStation> &BS_list, vector< vector<string> > cmd, vector<vector<int> > cmdIdx, int round_idx);
void RBalloc(vector<baseStation> &BS_list);
void calcsubSINR(vector<baseStation> &BS_list);
void calcavgSINR(vector<baseStation> &BS_list);
int selectCQI(double SNR_UPPERBOUND, double BLER_UPPERBOUND=0.1);
int selectMCS(double SNR);
void showUEinfo(vector<baseStation> BS_list);
void showUEallocRB(vector<baseStation> BS_list);
void showBSinfo(vector<baseStation> BS_list);
void showUEsinr(vector<baseStation> BS_list);
void showUERSRP(vector<baseStation> BS_list);
void initBSlist(vector<baseStation> &BS_list);
void saveUEinfo(vector<baseStation> BS_list, vector< vector<UEinfo> > &DATA);
void showAllresult(vector< vector<UEinfo> > DATA);
void showGJresult(vector< vector<UEinfo> > DATA, vector< vector<string> > cmd, vector< vector<int> > cmdIdx, int OptType=0);

const double  SNR_CQI[level_size][CQI_size]={
    { -14.5,-12.5,-10.5,-8.5,-6.5,-4.5,-4.5,-0.5,1.5,3.5,5.5,7,8.5,10.25,12 },
    { -14.4,-12.4,-10.4,-8.4,-6.4,-4.4,-4.4,-0.4,1.6,3.6,5.6,7.1,8.6,10.35,12.1 },
    { -14.3,-12.3,-10.3,-8.3,-6.3,-4.3,-4.3,-0.3,1.7,3.7,5.7,7.2,8.7,10.45,12.2 },
    { -14.2,-12.2,-10.2,-8.2,-6.2,-4.2,-4.2,-0.2,1.8,3.8,5.8,7.3,8.8,10.55,12.3 },
    { -14.1,-12.1,-10.1,-8.1,-6.1,-4.1,-4.1,-0.1,1.9,3.9,5.9,7.4,8.9,10.65,12.4 },
    { -14,-12,-10,-8,-6,-4,-4,0,2,4,6,7.5,9,10.75,12.5 },
    { -13.9,-11.9,-9.9,-7.9,-5.9,-3.9,-3.9,0.1,2.1,4.1,6.1,7.6,9.1,10.85,12.6 },
    { -13.8,-11.8,-9.8,-7.8,-5.8,-3.8,-3.8,0.2,2.2,4.2,6.2,7.7,9.2,10.95,12.7 },
    { -13.7,-11.7,-9.7,-7.7,-5.7,-3.7,-3.7,0.3,2.3,4.3,6.3,7.8,9.3,11.05,12.8 },
    { -13.6,-11.6,-9.6,-7.6,-5.6,-3.6,-3.6,0.4,2.4,4.4,6.4,7.9,9.4,11.15,12.9 },
    { -13.5,-11.5,-9.5,-7.5,-5.5,-3.5,-3.5,0.5,2.5,4.5,6.5,8,9.5,11.25,13 },
    { -13.4,-11.4,-9.4,-7.4,-5.4,-3.4,-3.4,0.6,2.6,4.6,6.6,8.1,9.6,11.35,13.1 },
    { -13.3,-11.3,-9.3,-7.3,-5.3,-3.3,-3.3,0.7,2.7,4.7,6.7,8.2,9.7,11.45,13.2 },
    { -13.2,-11.2,-9.2,-7.2,-5.2,-3.2,-3.2,0.8,2.8,4.8,6.8,8.3,9.8,11.55,13.3 },
    { -13.1,-11.1,-9.1,-7.1,-5.1,-3.1,-3.1,0.9,2.9,4.9,6.9,8.4,9.9,11.65,13.4 },
    { -13,-11,-9,-7,-5,-3,-3,1,3,5,7,8.5,10,11.75,13.5 },
    { -12.9,-10.9,-8.9,-6.9,-4.9,-2.9,-2.9,1.1,3.1,5.1,7.1,8.6,10.1,11.85,13.6 },
    { -12.8,-10.8,-8.8,-6.8,-4.8,-2.8,-2.8,1.2,3.2,5.2,7.2,8.7,10.2,11.95,13.7 },
    { -12.7,-10.7,-8.7,-6.7,-4.7,-2.7,-2.7,1.3,3.3,5.3,7.3,8.8,10.3,12.05,13.8 },
    { -12.6,-10.6,-8.6,-6.6,-4.6,-2.6,-2.6,1.4,3.4,5.4,7.4,8.9,10.4,12.15,13.9 },
    { -12.5,-10.5,-8.5,-6.5,-4.5,-2.5,-2.5,1.5,3.5,5.5,7.5,9,10.5,12.25,14 },
    { -12.4,-10.4,-8.4,-6.4,-4.4,-2.4,-2.4,1.6,3.6,5.6,7.6,9.1,10.6,12.35,14.1 },
    { -12.3,-10.3,-8.3,-6.3,-4.3,-2.3,-2.3,1.7,3.7,5.7,7.7,9.2,10.7,12.45,14.2 },
    { -12.2,-10.2,-8.2,-6.2,-4.2,-2.2,-2.2,1.8,3.8,5.8,7.8,9.3,10.8,12.55,14.3 },
    { -12.1,-10.1,-8.1,-6.1,-4.1,-2.1,-2.1,1.9,3.9,5.9,7.9,9.4,10.9,12.65,14.4 },
    { -12,-10,-8,-6,-4,-2,-2,2,4,6,8,9.5,11,12.75,14.5 },
    { -11.9,-9.9,-7.9,-5.9,-3.9,-1.9,-1.9,2.1,4.1,6.1,8.1,9.6,11.1,12.85,14.6 },
    { -11.8,-9.8,-7.8,-5.8,-3.8,-1.8,-1.8,2.2,4.2,6.2,8.2,9.7,11.2,12.95,14.7 },
    { -11.7,-9.7,-7.7,-5.7,-3.7,-1.7,-1.7,2.3,4.3,6.3,8.3,9.8,11.3,13.05,14.8 },
    { -11.6,-9.6,-7.6,-5.6,-3.6,-1.6,-1.6,2.4,4.4,6.4,8.4,9.9,11.4,13.15,14.9 },
    { -11.5,-9.5,-7.5,-5.5,-3.5,-1.5,-1.5,2.5,4.5,6.5,8.5,10,11.5,13.25,15 },
    { -11.4,-9.4,-7.4,-5.4,-3.4,-1.4,-1.4,2.6,4.6,6.6,8.6,10.1,11.6,13.35,15.1 },
    { -11.3,-9.3,-7.3,-5.3,-3.3,-1.3,-1.3,2.7,4.7,6.7,8.7,10.2,11.7,13.45,15.2 },
    { -11.2,-9.2,-7.2,-5.2,-3.2,-1.2,-1.2,2.8,4.8,6.8,8.8,10.3,11.8,13.55,15.3 },
    { -11.1,-9.1,-7.1,-5.1,-3.1,-1.1,-1.1,2.9,4.9,6.9,8.9,10.4,11.9,13.65,15.4 },
    { -11,-9,-7,-5,-3,-1,-1,3,5,7,9,10.5,12,13.75,15.5 },
    { -10.9,-8.9,-6.9,-4.9,-2.9,-0.9,-0.9,3.1,5.1,7.1,9.1,10.6,12.1,13.85,15.6 },
    { -10.8,-8.8,-6.8,-4.8,-2.8,-0.8,-0.8,3.2,5.2,7.2,9.2,10.7,12.2,13.95,15.7 },
    { -10.7,-8.7,-6.7,-4.7,-2.7,-0.7,-0.7,3.3,5.3,7.3,9.3,10.8,12.3,14.05,15.8 },
    { -10.6,-8.6,-6.6,-4.6,-2.6,-0.6,-0.6,3.4,5.4,7.4,9.4,10.9,12.4,14.15,15.9 },
    { -10.5,-8.5,-6.5,-4.5,-2.5,-0.5,-0.5,3.5,5.5,7.5,9.5,11,12.5,14.25,16 },
    { -10.4,-8.4,-6.4,-4.4,-2.4,-0.4,-0.4,3.6,5.6,7.6,9.6,11.1,12.6,14.35,16.1 },
    { -10.3,-8.3,-6.3,-4.3,-2.3,-0.3,-0.3,3.7,5.7,7.7,9.7,11.2,12.7,14.45,16.2 },
    { -10.2,-8.2,-6.2,-4.2,-2.2,-0.2,-0.2,3.8,5.8,7.8,9.8,11.3,12.8,14.55,16.3 },
    { -10.1,-8.1,-6.1,-4.1,-2.1,-0.1,-0.1,3.9,5.9,7.9,9.9,11.4,12.9,14.65,16.4 },
    { -10,-8,-6,-4,-2,0,0,4,6,8,10,11.5,13,14.75,16.5 },
    { -9.9,-7.9,-5.9,-3.9,-1.9,0.1,0.1,4.1,6.1,8.1,10.1,11.6,13.1,14.85,16.6 },
    { -9.8,-7.8,-5.8,-3.8,-1.8,0.2,0.2,4.2,6.2,8.2,10.2,11.7,13.2,14.95,16.7 },
    { -9.7,-7.7,-5.7,-3.7,-1.7,0.3,0.3,4.3,6.3,8.3,10.3,11.8,13.3,15.05,16.8 },
    { -9.6,-7.6,-5.6,-3.6,-1.6,0.4,0.4,4.4,6.4,8.4,10.4,11.9,13.4,15.15,16.9 },
    { -9.5,-7.5,-5.5,-3.5,-1.5,0.5,0.5,4.5,6.5,8.5,10.5,12,13.5,15.25,17 },
    { -9.4,-7.4,-5.4,-3.4,-1.4,0.6,0.6,4.6,6.6,8.6,10.6,12.1,13.6,15.35,17.1 },
    { -9.3,-7.3,-5.3,-3.3,-1.3,0.7,0.7,4.7,6.7,8.7,10.7,12.2,13.7,15.45,17.2 },
    { -9.2,-7.2,-5.2,-3.2,-1.2,0.8,0.8,4.8,6.8,8.8,10.8,12.3,13.8,15.55,17.3 },
    { -9.1,-7.1,-5.1,-3.1,-1.1,0.9,0.9,4.9,6.9,8.9,10.9,12.4,13.9,15.65,17.4 },
    { -9,-7,-5,-3,-1,1,1,5,7,9,11,12.5,14,15.75,17.5 },
    { -8.9,-6.9,-4.9,-2.9,-0.9,1.1,1.1,5.1,7.1,9.1,11.1,12.6,14.1,15.85,17.6 },
    { -8.8,-6.8,-4.8,-2.8,-0.8,1.2,1.2,5.2,7.2,9.2,11.2,12.7,14.2,15.95,17.7 },
    { -8.7,-6.7,-4.7,-2.7,-0.7,1.3,1.3,5.3,7.3,9.3,11.3,12.8,14.3,16.05,17.8 },
    { -8.6,-6.6,-4.6,-2.6,-0.6,1.4,1.4,5.4,7.4,9.4,11.4,12.9,14.4,16.15,17.9 },
    { -8.5,-6.5,-4.5,-2.5,-0.5,1.5,1.5,5.5,7.5,9.5,11.5,13,14.5,16.25,18 },
    { -8.4,-6.4,-4.4,-2.4,-0.4,1.6,1.6,5.6,7.6,9.6,11.6,13.1,14.6,16.35,18.1 },
    { -8.3,-6.3,-4.3,-2.3,-0.3,1.7,1.7,5.7,7.7,9.7,11.7,13.2,14.7,16.45,18.2 },
    { -8.2,-6.2,-4.2,-2.2,-0.2,1.8,1.8,5.8,7.8,9.8,11.8,13.3,14.8,16.55,18.3 },
    { -8.1,-6.1,-4.1,-2.1,-0.1,1.9,1.9,5.9,7.9,9.9,11.9,13.4,14.9,16.65,18.4 },
    { -8,-6,-4,-2,0,2,2,6,8,10,12,13.5,15,16.75,18.5 },
    { -7.9,-5.9,-3.9,-1.9,0.1,2.1,2.1,6.1,8.1,10.1,12.1,13.6,15.1,16.85,18.6 },
    { -7.8,-5.8,-3.8,-1.8,0.2,2.2,2.2,6.2,8.2,10.2,12.2,13.7,15.2,16.95,18.7 },
    { -7.7,-5.7,-3.7,-1.7,0.3,2.3,2.3,6.3,8.3,10.3,12.3,13.8,15.3,17.05,18.8 },
    { -7.6,-5.6,-3.6,-1.6,0.4,2.4,2.4,6.4,8.4,10.4,12.4,13.9,15.4,17.15,18.9 },
    { -7.5,-5.5,-3.5,-1.5,0.5,2.5,2.5,6.5,8.5,10.5,12.5,14,15.5,17.25,19 },
    { -7.4,-5.4,-3.4,-1.4,0.6,2.6,2.6,6.6,8.6,10.6,12.6,14.1,15.6,17.35,19.1 },
    { -7.3,-5.3,-3.3,-1.3,0.7,2.7,2.7,6.7,8.7,10.7,12.7,14.2,15.7,17.45,19.2 },
    { -7.2,-5.2,-3.2,-1.2,0.8,2.8,2.8,6.8,8.8,10.8,12.8,14.3,15.8,17.55,19.3 },
    { -7.1,-5.1,-3.1,-1.1,0.9,2.9,2.9,6.9,8.9,10.9,12.9,14.4,15.9,17.65,19.4 },
    { -7,-5,-3,-1,1,3,3,7,9,11,13,14.5,16,17.75,19.5 },
    { -6.9,-4.9,-2.9,-0.9,1.1,3.1,3.1,7.1,9.1,11.1,13.1,14.6,16.1,17.85,19.6 },
    { -6.8,-4.8,-2.8,-0.8,1.2,3.2,3.2,7.2,9.2,11.2,13.2,14.7,16.2,17.95,19.7 },
    { -6.7,-4.7,-2.7,-0.7,1.3,3.3,3.3,7.3,9.3,11.3,13.3,14.8,16.3,18.05,19.8 },
    { -6.6,-4.6,-2.6,-0.6,1.4,3.4,3.4,7.4,9.4,11.4,13.4,14.9,16.4,18.15,19.9 },
    { -6.5,-4.5,-2.5,-0.5,1.5,3.5,3.5,7.5,9.5,11.5,13.5,15,16.5,18.25,20 },
    { -6.4,-4.4,-2.4,-0.4,1.6,3.6,3.6,7.6,9.6,11.6,13.6,15.1,16.6,18.35,20.1 },
    { -6.3,-4.3,-2.3,-0.3,1.7,3.7,3.7,7.7,9.7,11.7,13.7,15.2,16.7,18.45,20.2 },
    { -6.2,-4.2,-2.2,-0.2,1.8,3.8,3.8,7.8,9.8,11.8,13.8,15.3,16.8,18.55,20.3 },
    { -6.1,-4.1,-2.1,-0.1,1.9,3.9,3.9,7.9,9.9,11.9,13.9,15.4,16.9,18.65,20.4 },
    { -6,-4,-2,0,2,4,4,8,10,12,14,15.5,17,18.75,20.5 },
    { -5.9,-3.9,-1.9,0.1,2.1,4.1,4.1,8.1,10.1,12.1,14.1,15.6,17.1,18.85,20.6 },
    { -5.8,-3.8,-1.8,0.2,2.2,4.2,4.2,8.2,10.2,12.2,14.2,15.7,17.2,18.95,20.7 },
    { -5.7,-3.7,-1.7,0.3,2.3,4.3,4.3,8.3,10.3,12.3,14.3,15.8,17.3,19.05,20.8 },
    { -5.6,-3.6,-1.6,0.4,2.4,4.4,4.4,8.4,10.4,12.4,14.4,15.9,17.4,19.15,20.9 },
    { -5.5,-3.5,-1.5,0.5,2.5,4.5,4.5,8.5,10.5,12.5,14.5,16,17.5,19.25,21 },
    { -5.4,-3.4,-1.4,0.6,2.6,4.6,4.6,8.6,10.6,12.6,14.6,16.1,17.6,19.35,21.1 },
    { -5.3,-3.3,-1.3,0.7,2.7,4.7,4.7,8.7,10.7,12.7,14.7,16.2,17.7,19.45,21.2 },
    { -5.2,-3.2,-1.2,0.8,2.8,4.8,4.8,8.8,10.8,12.8,14.8,16.3,17.8,19.55,21.3 },
    { -5.1,-3.1,-1.1,0.9,2.9,4.9,4.9,8.9,10.9,12.9,14.9,16.4,17.9,19.65,21.4 },
    { -5,-3,-1,1,3,5,5,9,11,13,15,16.5,18,19.75,21.5 },
    { -4.9,-2.9,-0.9,1.1,3.1,5.1,5.1,9.1,11.1,13.1,15.1,16.6,18.1,19.85,21.6 },
    { -4.8,-2.8,-0.8,1.2,3.2,5.2,5.2,9.2,11.2,13.2,15.2,16.7,18.2,19.95,21.7 },
    { -4.7,-2.7,-0.7,1.3,3.3,5.3,5.3,9.3,11.3,13.3,15.3,16.8,18.3,20.05,21.8 },
    { -4.6,-2.6,-0.6,1.4,3.4,5.4,5.4,9.4,11.4,13.4,15.4,16.9,18.4,20.15,21.9 },
    { -4.5,-2.5,-0.5,1.5,3.5,5.5,5.5,9.5,11.5,13.5,15.5,17,18.5,20.25,22 },
    { -4.4,-2.4,-0.4,1.6,3.6,5.6,5.6,9.6,11.6,13.6,15.6,17.1,18.6,20.35,22.1 },
    { -4.3,-2.3,-0.3,1.7,3.7,5.7,5.7,9.7,11.7,13.7,15.7,17.2,18.7,20.45,22.2 },
    { -4.2,-2.2,-0.2,1.8,3.8,5.8,5.8,9.8,11.8,13.8,15.8,17.3,18.8,20.55,22.3 },
    { -4.1,-2.1,-0.1,1.9,3.9,5.9,5.9,9.9,11.9,13.9,15.9,17.4,18.9,20.65,22.4 },
    { -4,-2,0,2,4,6,6,10,12,14,16,17.5,19,20.75,22.5 }
};

const double  BLER_CQI[level_size][CQI_size]={
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,0.9998,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,0.9998,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 0.9998,0.9998,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 0.9998,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 0.9992,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 0.9994,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 0.999,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 0.9988,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 0.9966,0.9996,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 0.9974,0.9998,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 0.9942,0.9988,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 0.9932,0.9984,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 0.99,0.9978,1,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 0.9854,0.997,0.9998,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 0.9812,0.9936,0.9994,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 0.9744,0.9888,0.999,1,1,1,1,1,1,1,1,1,1,1,1 },
    { 0.968,0.9842,0.9976,0.9998,1,1,1,1,1,1,1,1,1,1,1 },
    { 0.9504,0.9742,0.9984,0.9986,1,1,1,1,1,1,1,1,1,1,1 },
    { 0.939,0.9658,0.9914,0.9978,0.9996,1,1,1,1,1,0.9998,1,1,1,1 },
    { 0.9248,0.9478,0.9876,0.9898,1,0.9998,1,0.9998,1,1,0.9992,1,1,1,1 },
    { 0.8986,0.928,0.9802,0.983,0.9996,0.9994,1,0.9996,0.9996,0.9996,0.9976,1,1,1,1 },
    { 0.8666,0.9044,0.9678,0.973,0.9982,0.9978,1,0.9986,0.9968,0.9976,0.988,1,1,1,1 },
    { 0.8232,0.866,0.9468,0.942,0.996,0.9932,1,0.995,0.99,0.99,0.9644,0.9994,1,1,1 },
    { 0.8,0.8306,0.9182,0.9084,0.9896,0.9836,1,0.9832,0.9706,0.9712,0.9062,0.9984,1,1,1 },
    { 0.749,0.7674,0.884,0.8398,0.978,0.9612,1,0.957,0.9198,0.918,0.794,0.9928,1,1,1 },
    { 0.6914,0.7012,0.8356,0.7692,0.9462,0.919,1,0.888,0.8222,0.8242,0.6234,0.9756,1,1,0.9998 },
    { 0.6464,0.66,0.7606,0.6466,0.8934,0.8336,1,0.7762,0.6914,0.6822,0.4386,0.9346,0.9988,1,1 },
    { 0.5886,0.598,0.6856,0.548,0.8222,0.7292,1,0.6356,0.5012,0.4932,0.2472,0.8442,0.9964,0.9992,0.9998 },
    { 0.5312,0.498,0.5952,0.4324,0.7232,0.5832,1,0.4478,0.3138,0.3124,0.1164,0.6858,0.984,0.9968,0.9992 },
    { 0.469,0.4152,0.5008,0.3022,0.5864,0.4318,1,0.2824,0.164,0.1658,0.0462,0.4844,0.9494,0.987,0.9962 },
    { 0.4232,0.3422,0.421,0.207,0.436,0.2872,1,0.1548,0.0736,0.0744,0.0136,0.3004,0.8646,0.9518,0.9912 },
    { 0.3468,0.2916,0.3192,0.1304,0.3086,0.1742,1,0.0634,0.0276,0.027,0.0026,0.154,0.7126,0.8776,0.9556 },
    { 0.2836,0.2062,0.2362,0.0858,0.194,0.0918,1,0.025,0.0096,0.0082,0.001,0.0616,0.516,0.7288,0.8886 },
    { 0.2476,0.1716,0.1724,0.0436,0.1108,0.0424,1,0.0076,0.0026,0.0016,0.0004,0.0154,0.3054,0.5062,0.7776 },
    { 0.1954,0.1184,0.113,0.0202,0.0506,0.0158,1,0.0022,0.0004,0.0002,0,0.0082,0.1506,0.313,0.6158 },
    { 0.1632,0.0842,0.0666,0.0128,0.0264,0.0074,1,0.0002,0,0,0,0.0014,0.054,0.151,0.4174 },
    { 0.1146,0.0532,0.0414,0.0038,0.0094,0.0022,1,0.0002,0,0,0,0.0004,0.0198,0.0642,0.262 },
    { 0.0826,0.0354,0.0238,0.0022,0.0042,0.0006,1,0,0,0,0,0,0.0048,0.0186,0.1518 },
    { 0.0616,0.0238,0.0134,0.0008,0.0012,0,1,0,0,0,0,0,0.001,0.007,0.0704 },
    { 0.048,0.0166,0.0058,0.0002,0.0004,0,1,0.0002,0,0,0,0,0,0.0022,0.0322 },
    { 0.0352,0.008,0.0042,0.0002,0,0,1,0,0,0,0,0,0.0002,0.0006,0.0146 },
    { 0.0202,0.0044,0.0016,0,0,0,1,0,0,0,0,0,0.0002,0.001,0.0058 },
    { 0.0176,0.0024,0.0012,0,0,0,0.9992,0,0,0,0,0,0,0.0006,0.0012 },
    { 0.0116,0.001,0.0002,0,0,0,0.995,0,0,0,0,0,0,0.0002,0.0008 },
    { 0.0088,0.0008,0,0,0,0,0.9828,0,0,0,0,0,0,0.0004,0.0004 },
    { 0.0036,0.0004,0,0,0,0,0.956,0,0,0,0,0,0,0.0002,0 },
    { 0.0022,0.0002,0,0,0,0,0.9016,0,0,0,0,0,0,0.0002,0 },
    { 0.0012,0.0004,0,0,0,0,0.8108,0,0,0,0,0,0,0.0002,0.0002 },
    { 0.0006,0.0006,0,0,0,0,0.6698,0,0,0,0,0,0,0,0 },
    { 0.0004,0,0,0,0,0,0.5298,0,0,0,0,0,0,0,0 },
    { 0.0008,0,0,0,0,0,0.3608,0,0,0,0,0,0,0,0 },
    { 0.0004,0,0,0,0,0,0.2172,0,0,0,0,0,0,0,0 },
    { 0.0004,0,0,0,0,0,0.1224,0,0,0,0,0,0,0,0 },
    { 0,0,0,0,0,0,0.0516,0,0,0,0,0,0,0,0 },
    { 0,0,0,0,0,0,0.0268,0,0,0,0,0,0,0,0 },
    { 0,0,0,0,0,0,0.0086,0,0,0,0,0,0,0,0 },
    { 0,0,0,0,0,0,0.0032,0,0,0,0,0,0,0,0 },
    { 0,0,0,0,0,0,0.001,0,0,0,0,0,0,0,0 },
    { 0,0,0,0,0,0,0.0006,0,0,0,0,0,0,0,0 },
    { 0,0,0,0,0,0,0.0004,0,0,0,0,0,0,0,0 },
    { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
    { 0,0,0,0,0,0,0.0002,0,0,0,0,0,0,0,0 },
    { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
    { 0,0,0,0,0,0,0.0002,0,0,0,0,0,0,0,0 },
    { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
    { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
    { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
};

const double CQI_eff[16]={0, 0.1523, 0.2344, 0.3770, 0.6016, 0.8770, 1.1758, 1.4766, 1.9141, 2.4063, 2.7305, 3.3223, 3.9023, 4.5234, 5.1152, 5.5547};
// table is follow the Table 7.2.3-1 of TS 36.213 v12.3.0

const double SNR2MCS_range[MCS_map_size]={
    25.0, 24.0, 23.0, 22.0, 21.0,
    20.0, 19.0, 18.0, 17.0, 16.5,
    15.5, 14.5, 14.0, 13.0, 12.0,
    11.0, 10.0, 9.5,  9.0,  7.0,
    5.0,  3.0,  0 };
const int SNR2MCS_map[MCS_map_size]={
    28, 27, 26, 25, 24,
    23, 22, 21, 20, 19,
    18, 17, 16, 15, 14,
    13, 12, 11, 10, 7,
    6,  4,  2 };

const double MCS_TBS_50mimo[29]={2768, 3600, 4432, 5712, 7248, 8784, 10320, 12400, 13936, 15984, 15984, 17520, 19824, 22896, 25920, 28224, 30528, 30528, 32832, 36672, 39696, 42768, 45840, 50912, 54752, 56672, 61152, 63408, 65712};

/*  MCS TBSidx TBS Mapping Table
 
 MCS	TBSidx	PRB100	PRB50	50mimo
 0      0       2792	1384	2768
 1      1       3624	1800	3600
 2      2       4584	2216	4432
 3      3       5736	2856	5712
 4      4       7224	3624	7248
 5      5       8760	4392	8784
 6      6       10296	5160	10320
 7      7       12216	6200	12400
 8      8       14112	6968	13936
 9      9       15840	7992	15984
 10     9       15840	7992	15984
 11     10      17568	8760	17520
 12     11      19848	9912	19824
 13     12      22920	11448	22896
 14     13      25456	12960	25920
 15     14      28336	14112	28224
 16     15      30576	15264	30528
 17     15      30576	15264	30528
 18     16      32856	16416	32832
 19     17      36696	18336	36672
 20     18      39232	19848	39696
 21     19      43816	21384	42768
 22     20      46888	22920	45840
 23     21      51024	25456	50912
 24     22      55056	27376	54752
 25     23      57336	28336	56672
 26     24      61664	30576	61152
 27     25      63776	31704	63408
 28     26      75376	32856	65712
*/

/* SNR MCS TBS Mapping Table
 
 SNR	MCS	PRB100	PRB50	50mimo
 0      2	4584	2216	4432
 1      2	4584	2216	4432
 2      2	4584	2216	4432
 3      4	7224	3624	7248
 4      4	7224	3624	7248
 5      6	10296	5160	10320
 6      6	10296	5160	10320
 7      7	12216	6200	12400
 8      7	12216	6200	12400
 9      10	15840	7992	15984
 10     12	19848	9912	19824
 11     12	19848	9912	19824
 12     14	25456	12960	25920
 13     14	25456	12960	25920
 14     16	30576	15264	30528
 15     17	30576	15264	30528
 16     17	30576	15264	30528
 17     20	39232	19848	39696
 18     20	39232	19848	39696
 19     22	46888	22920	45840
 20     22	46888	22920	45840
 21     24	55056	27376	54752
 22     24	55056	27376	54752
 23     26	61664	30576	61152
 24     26	61664	30576	61152
 25     28	75376	32856	65712
*/
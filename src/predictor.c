//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include <string.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "Ge Chang";
const char *studentID   = "A53240181";
const char *email       = "chg073@eng.ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int lhistoryBits;
int ghistoryBits;
int pcIndexBits;

int bpType;       // Branch Prediction Type
int verbose;


//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//

//------The Static Prdictor------
//We assume the instruction is always taken
//initialization: no need


//------The Tournament Predictor------

//construct the localPredictor
uint32_t *localPHT;
uint8_t *localBHT;
uint8_t localRes;

//construct the globalPredictor
uint8_t *globalBHT;
uint32_t currStatus; //current status of the branch
uint8_t globalRes;

//choice maker
uint8_t *predictorPre; //00->0 SL, 01->1: WL, 10->2: WG, 11->3: SG

//------Custom------
//We choose a dynamic branch predictor here
//two bits here: Starting with WNT

struct dynamicMap {
    uint32_t pcAdd;
    uint32_t status;
};

struct  dynamicPos{
    uint32_t pos;
    uint8_t find; //1: find, 0: didn't find
};

struct dynamicMap dMap[MAXBRANCHNUM];

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

int calPow(int num){
    int ans = 1;
    for(int i = 0; i < num; i++){
        ans = ans*2;
    }
    return ans;
}

void modifyPrediction(uint8_t *counter, uint8_t result){
    if(result == TAKEN){
        if(*counter != ST) (*counter)++;
    }else{
        if(*counter != SN) (*counter)--;
    }
}
//------Static Functions------//
//no initialization in Static

//------Gshare Functions------//
//by Wenyu Zhang

//------Tournament------//
//local predictor

uint8_t localPrediction(uint32_t pc){
    uint32_t index1 = pc & (( 1 << pcIndexBits) - 1);
    uint32_t index2 = localPHT[index1];
    uint8_t localP = localBHT[index2];

    if(localP == WN || localP ==SN){
        localRes = NOTTAKEN;
    }else localRes = TAKEN;

    return localRes;
}

//global predictor


uint8_t globalPrediction(uint32_t pc){
    uint32_t index1 = currStatus & (( 1 << ghistoryBits) - 1);
    uint8_t globalP = globalBHT[index1];

    if(globalP == WN || globalP ==SN){
        globalRes = NOTTAKEN;
    }else globalRes = TAKEN;

    return globalRes;
}

//choice maker
uint8_t choiceMaker(uint32_t pc){
    uint32_t index1 = (currStatus) & ((1 << ghistoryBits) - 1);
    uint32_t choice = predictorPre[index1];

    localPrediction(pc);
    globalPrediction(pc);

    if(choice == WN || choice == SN){
        return globalRes;
    }else{
        return localRes;
    }
    //return res1;
}

void tournamentInit() {
    //------initialization for the tournament predictor------
    //local predictor
    localBHT = malloc((1 << lhistoryBits) * sizeof(uint8_t));
    localPHT = malloc((1 << pcIndexBits) * sizeof(uint32_t));
    memset(localBHT, WN, (1 << lhistoryBits) * sizeof(uint8_t));
    memset(localPHT, 0, (1 << pcIndexBits) * sizeof(uint32_t));
    //global predictor
    currStatus = 0;
    globalBHT = malloc((1 << ghistoryBits) * sizeof(uint8_t));
    memset(globalBHT, WN, (1 << ghistoryBits) * sizeof(uint8_t));
    //------initialization for the custom predictor------
    predictorPre = malloc((1 << ghistoryBits) * sizeof(uint8_t));
    memset(predictorPre, WN, (1 << ghistoryBits) * sizeof(uint8_t));
}

void tournamentUpdate(uint32_t pc, uint8_t result){

    if(localRes != globalRes) {
        modifyPrediction(&predictorPre[currStatus], (localRes == result)? TAKEN : NOTTAKEN);
    }

    uint32_t index1 = pc & ((1 << pcIndexBits) - 1);
    uint32_t index2 = localPHT[index1];

    modifyPrediction(&(localBHT[index2]), result);
    localPHT[index1] <<= 1;
    localPHT[index1] &= ((1 << lhistoryBits) - 1);
    localPHT[index1] |= result;
    modifyPrediction(&globalBHT[currStatus], result);
    currStatus <<= 1;
    currStatus &= ((1 << ghistoryBits) - 1);
    currStatus |= result;
    return;
}

//------Custom------//
/* struct dynamicPos findDynamicBranch(uint32_t pc){
    struct dynamicPos ans;
    ans.find = 0;
    ans.pos = 0;
    for(int i = 0; i < sizeof(dMap)/sizeof(struct dynamicMap); i++){
        if(dMap[i].pcAdd == pc){
            ans.find = 1;
            ans.pos = i;
            return ans;
        }else if(dMap[i].pcAdd == -1){
            ans.find = 0;
            ans.pos = i;
            return ans;
        }else{}
    }
    return ans;
}

*/
// Initialize the predictor
//
void
init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //
  //no initialization for static predictor
    switch (bpType) {
        case STATIC:
            return;
        case TOURNAMENT:
            tournamentInit();
            break;
        case GSHARE:
            break;
        case CUSTOM:
        default:
            break;
    }

}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{
  //
  //TODO: Implement prediction scheme
  //


  // Make a prediction based on the bpType
  //struct dynamicPos ans = findDynamicBranch(pc);

  switch (bpType) {
    case STATIC:
        return TAKEN;
    case GSHARE:
        return TAKEN;
    case TOURNAMENT:
        return choiceMaker(pc);
    case CUSTOM:
        //if(ans.find == 0) return NOTTAKEN;
        //else if(dMap[ans.pos].status <= 2) return NOTTAKEN;
        //else return TAKEN;
        return TAKEN;
    default:
        return TAKEN;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// result 'result' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void
train_predictor(uint32_t pc, uint8_t result)
{
  //
  //TODO: Implement Predictor training
  //
    switch (bpType) {
        case STATIC:
            break;
        case TOURNAMENT:
            tournamentUpdate(pc, result);
            break;
        case GSHARE:
            break;
        case CUSTOM:
            //neural_train(pc, outcome);
            //tage_train(pc, outcome);
            //wp_train(pc, outcome);
            //perceptron_train(pc, outcome);
            break;
        default:
            break;
    }

    // If there is not a compatable bpType then return NOTTAKEN
    return;
  //------Custom------//
/*  struct dynamicPos ans = findDynamicBranch(pc);

  if(ans.find == 0){
    dMap[ans.pos].pcAdd = pc;
    //dMap[ans.pos].status = 1;
  }
  else{
    if(result == NOTTAKEN){
      if(dMap[ans.pos].status != 0) dMap[ans.pos].status--;
    }else{
      if(dMap[ans.pos].status != 3) dMap[ans.pos].status++;
    }
  }
    printf("Train finished \n");
  return; */
}

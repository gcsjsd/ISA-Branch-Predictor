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
#include "TAGE.h"

//
// TODO:Student Information
//
const char *studentName = "Ge Chang, Wenyu Zhang";
const char *studentID   = "A53240181, A53238371";
const char *email       = "chg073@eng.ucsd.edu, wez078@ucsd.edu";

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

//------The Static Predictor------
//We assume the instruction is always taken
//initialization: no need

//------The GShare Predictor------

//------The Tournament Predictor------

//construct the localPredictor(Tournament)
uint32_t *localPHT;
uint8_t *localBHT;
uint8_t localRes;

//construct the globalPredictor(GShare and Tournament)
uint8_t *globalBHT;
uint32_t currStatus; //current status of the branch
uint8_t globalRes;

//choice maker
uint8_t *predictorPre; //00->0 SL, 01->1: WL, 10->2: WG, 11->3: SG

//------Custom------
//We use TAGE branch predictor
//the explanation and implementation detail is in TAGE.h


//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

//------Static Functions------//
//no initialization in Static

//------Gshare Functions------//
//global predictor

uint8_t GSharePrediction(uint32_t pc){
    uint32_t index1 = pc & (( 1 << ghistoryBits) - 1);
    uint32_t index2 = currStatus & ((1 << ghistoryBits) - 1);
    uint32_t index = (index1 ^ index2) & ((1 << ghistoryBits) - 1);
    uint8_t globalP = globalBHT[index];

    if(globalP == WN || globalP ==SN){
        globalRes = NOTTAKEN;
    }else globalRes = TAKEN;

    return globalRes;
}

void GShareInit() {
    //------initialization for the GShare predictor------
    //global predictor
    //init BH
    currStatus = 0;
    //init BHT
    globalBHT = malloc((1 << ghistoryBits) * sizeof(uint8_t));
    memset(globalBHT, WN, (1 << ghistoryBits) * sizeof(uint8_t));
}

void GShareUpdate(uint32_t pc, uint8_t result){
    //update BHT
    uint32_t index1 = pc & (( 1 << ghistoryBits) - 1);
    uint32_t index2 = currStatus & ((1 << ghistoryBits) - 1);
    uint32_t index = (index1 ^ index2) & ((1 << ghistoryBits) - 1);
    modifyPrediction(&globalBHT[index], result);
    //update BH
    currStatus <<= 1;
    currStatus &= ((1 << ghistoryBits) - 1);
    currStatus |= result;
    return;
}

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

//funcion to update the predictor

void modifyPrediction(uint8_t *counter, uint8_t result){
    if(result == TAKEN){
        if(*counter != ST) (*counter)++;
    }else{
        if(*counter != SN) (*counter)--;
    }
}


// Initialize the predictor

void
init_predictor()
{
    switch (bpType) {
        case STATIC:
            return;
        case TOURNAMENT:
            tournamentInit();
            break;
        case GSHARE:
            GShareInit();
            break;
        case CUSTOM:
            tage_init();
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
  switch (bpType) {
    case STATIC:
        return TAKEN;
    case GSHARE:
        return GSharePrediction(pc);
    case TOURNAMENT:
        return choiceMaker(pc);
    case CUSTOM:
        //if(ans.find == 0) return NOTTAKEN;
        //else if(dMap[ans.pos].status <= 2) return NOTTAKEN;
        //else return TAKEN;
        return tage_predict(pc);
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
    switch (bpType) {
        case STATIC:
            break;
        case TOURNAMENT:
            tournamentUpdate(pc, result);
            break;
        case GSHARE:
            GShareUpdate(pc, result);
            break;
        case CUSTOM:
            tage_train(pc, result);
            break;
        default:
            break;
    }
    return;
}

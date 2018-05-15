//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "NAME";
const char *studentID   = "PID";
const char *email       = "EMAIL";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };


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
const int ghistoryBits; // Number of bits used for Global History
const int lhistoryBits; // Number of bits used for Local History
const int pcIndexBits;  // Number of bits used for PC index

//construct the localPredictor
struct localNode {
    uint32_t pattern;
    uint8_t status;
};

struct localPcNode {
    uint32_t pcAdd;
    struct localNode localPcMap[lhistoryBits];
};

struct localNode lMap[pcIndexBits];

//construct the globalPredictor
struct globalNode {
    uint32_t pattern = 0;
    uint8_t status = 1;
};

struct globalNode gMap[ghistoryBits];

//------Custom------
//We choose a dynamic branch predictor here
//two bits here: Starting with WNT

struct dynamicMap {
    uint32_t pcAdd;
    uint32_t status;
};

struct  dynamicPos{
    uint8_t pos;
    uint8_t find; //1: find, 0: didn't find
};

struct dynamicMap dMap[MAXBRANCHNUM];

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

//------Static Functions------//
//no need

//------Gshare Functions------//
//by Wenyu Zhang

//------Tournament------//


//------Custom------//
struct dynamicPos findDynamicBranch(uint32_t pc){
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


// Initialize the predictor
//
void
init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //

  //no initialization for static predictor
  for(int i = 0; i < sizeof(dMap)/sizeof(struct dynamicMap); i++){
    dMap[i].pcAdd = -1;
    dMap[i].status = 1; //00->0 SNT, 01->1: WNT, 10->2: WT, 11->3: ST
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
  struct dynamicPos ans = findDynamicBranch(pc);

  switch (bpType) {
    case STATIC:
        return TAKEN;
    case GSHARE:
    case TOURNAMENT:
    case CUSTOM:
        if(ans.find == 0) return NOTTAKEN;
        else if(dMap[ans.pos].status <= 2) return NOTTAKEN;
        else return TAKEN;
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void
train_predictor(uint32_t pc, uint8_t outcome)
{
  //
  //TODO: Implement Predictor training
  //


  //------Custom------//
  struct dynamicPos ans = findDynamicBranch(pc);

  if(ans.find == 0){
    dMap[ans.pos].pcAdd = pc;
    //dMap[ans.pos].status = 1;
  }
  else{
    if(outcome == NOTTAKEN){
      if(dMap[ans.pos].status != 0) dMap[ans.pos].status--;
    }else{
      if(dMap[ans.pos].status != 3) dMap[ans.pos].status++;
    }
  }
  return;
}

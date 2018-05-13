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


int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//

//The Static Prdictor

struct staticMap {
    uint32_t pcAdd;
    uint32_t status;
};

struct  StaticPos{
    uint8_t pos;
    uint8_t find; //1: find, 0: didn't find
};


struct staticMap map[MAXBRANCHNUM];


//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void
init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //

  for(int i = 0; i < sizeof(map)/sizeof(struct staicMap); i++){
    map[i].pcAdd = -1;
    map[i].status = 1; //00->0 SNT, 01->1: WNT, 10->2: WT, 11->3: ST
  }

}


struct StaticPos findStaticBranch(uint32_t pc){
  struct StaticPos ans;
  ans.find = 0;
  ans.pos = 0;
  for(int i = 0; i < sizeof(map)/sizeof(struct staticMap); i++){
    if(map[i].pcAdd == pc){
      ans.find = 1;
      ans.pos = i;
      return ans;
    }else if(map[i] == -1){
      ans.find = 0;
      ans.pos = i;
      return ans;
    }else{}
  }
  return ans;
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
  struct StaticPos ans = findStaticBranch(pc);

  switch (bpType) {
    case STATIC:
      if(ans.find == 0) return NOTTAKEN;
      else if(map[ans.pos].status <= 2) return NOTTAKEN;
      else return TAKEN;
    case GSHARE:
    case TOURNAMENT:
    case CUSTOM:
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
  struct StaticPos ans = findStaticBranch(pc);

  if(ans.find == 0){
    map[ans.pos]
  }
  else if(map[ans.pos].status <= 2) return NOTTAKEN;
  else return TAKEN;

  if(outcome == 0){

  }
}

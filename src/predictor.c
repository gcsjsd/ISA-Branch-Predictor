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
struct localNode {
    //uint32_t pattern;
    uint8_t status;
};

struct localPcNode {
    uint32_t pcAdd;
    uint32_t currLocalPattern;
    struct localNode localPcMap[4096];
};

struct localPos {
    uint32_t pos;
    uint8_t find;
};

struct localPcNode lMap[4096];

//construct the globalPredictor
struct globalNode {
    //uint32_t pattern;
    uint8_t status;
};


struct  globalPos {
    uint32_t pos;
    uint8_t find;
};

struct globalNode gMap[4096];
uint32_t currGlobalPattern;

//choice maker
uint8_t predictorPre = 1; //00->0 SL, 01->1: WL, 10->2: WG, 11->3: SG

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

//------Static Functions------//
//no initialization in Static

//------Gshare Functions------//
//by Wenyu Zhang

//------Tournament------//
//local predictor

struct localPos findLocalBranch(uint32_t pc){
    struct localPos ans;
    ans.find = 0;
    ans.pos = 0;
    for(int i = 0; i < calPow(pcIndexBits); i++){
        printf("%d \n", lhistoryBits);
        if(lMap[i].pcAdd == pc){
            ans.find = 1;
            ans.pos = i;
            return ans;
        }else if(lMap[i].pcAdd == -1){
            lMap[i].pcAdd = pc;
            ans.find = 0;
            ans.pos = i;
            return ans;
        }else{}
    }
    return ans;
}

uint8_t localRes(uint32_t pc){
    printf("cal local \n");
    struct localPos position = findLocalBranch(pc);
    if(position.find == 0){
        printf("giving 0 \n");
        //lMap[position.pos].pcAdd = pc;
        printf("giving local \n");
        return NOTTAKEN;
    }else{
        //printf("%d \n", position.pos);
        //printf("%d \n", lMap[position.pos].currLocalPattern);
        if(lMap[position.pos].localPcMap[lMap[position.pos].currLocalPattern].status <= 1){
            printf("giving local \n");
            return NOTTAKEN;
        }
        else{
            printf("giving local \n");
            return TAKEN;
        }
    }
    return NOTTAKEN;
}

//global predictor

struct globalPos findGlobalBranch(uint32_t pc){
    struct globalPos ans;
    ans.find = 0;
    ans.pos = 0;
    for(int i = 0; i < calPow(ghistoryBits); i++){
        if(i == currGlobalPattern){
            ans.find = 1;
            ans.pos = i;
            return ans;
        }else{}
    }
    return ans;
}

uint8_t globalRes(uint32_t pc){
    printf("cal global \n");
    if(gMap[findGlobalBranch(pc).pos].status <= 1) return NOTTAKEN;
    else return TAKEN;
}

//choice maker
uint8_t choiceMaker(uint8_t res1, uint8_t res2){
    if(res1 == res2) return res1;
    else{
        if(predictorPre <= 1) return res1;
        else return res2;
    }
    //return res1;
}

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

  //------initialization for the tournament predictor------
  //local predictor
    printf("Initialization begins \n");
  for(int i = 0; i < calPow(pcIndexBits); i++){
      lMap[i].pcAdd = -1;
      lMap[i].currLocalPattern = 0;
      for (int j = 0; j < calPow(lhistoryBits); j++) {
          lMap[i].localPcMap[j].status = 1; //00->0 SNT, 01->1: WNT, 10->2: WT, 11->3: ST
          //lMap[i].localPcMap[j].pattern = j; //different pattern 00000... -> 11111...
      }
  }
  //global predictor
  currGlobalPattern = 0;
  for(int i = 0; i < calPow(ghistoryBits); i++){
      //gMap[i].pattern = i;
      gMap[i].status = 1;//00->0 SNT, 01->1: WNT, 10->2: WT, 11->3: ST
  }
  //------initialization for the custom predictor------
  for(int i = 0; i < sizeof(dMap)/sizeof(struct dynamicMap); i++){
    dMap[i].pcAdd = -1;
    dMap[i].status = 1; //00->0 SNT, 01->1: WNT, 10->2: WT, 11->3: ST
  }
    printf("Initialization finished \n");
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
        printf("T-prediction begins \n");
        return choiceMaker(localRes(pc), globalRes(pc));
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
    printf("Train begins\n");
  //------Tournament------
  uint8_t res1 = localRes(pc);
  uint8_t res2 = globalRes(pc);
  //update local
    printf("Train local\n");
  struct localPos position = findLocalBranch(pc);
  if(outcome == TAKEN){
      if(lMap[position.pos].localPcMap[lMap[position.pos].currLocalPattern].status < 3){
          lMap[position.pos].localPcMap[lMap[position.pos].currLocalPattern].status++;
      }
      uint32_t tmp = (lMap[position.pos].currLocalPattern<<1) + 1;
      lMap[position.pos].currLocalPattern = tmp;
  }else{
      if(lMap[position.pos].localPcMap[lMap[position.pos].currLocalPattern].status > 0){
          lMap[position.pos].localPcMap[lMap[position.pos].currLocalPattern].status--;
      }
      uint32_t tmp = lMap[position.pos].currLocalPattern<<1;
      lMap[position.pos].currLocalPattern = tmp;
  }
    printf("Train global\n");
  //update global
  if(outcome == TAKEN){
      if(gMap[findGlobalBranch(pc).pos].status < 3){
          gMap[findGlobalBranch(pc).pos].status++;
      }else{}
      currGlobalPattern = (currGlobalPattern << 1) + 1;
  }else{
      if(gMap[findGlobalBranch(pc).pos].status < 0){
          gMap[findGlobalBranch(pc).pos].status--;
      }else{}
      currGlobalPattern = currGlobalPattern << 1;
  }
  //update predictor
    printf("Train predictor\n");
  if(res1 != res2){
      if(res1 == outcome){
          if(predictorPre > 0) predictorPre--;
      }else{
          if(predictorPre < 3) predictorPre++;
      }
  }

  //------Custom------//
    printf("Train custom\n");
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
    printf("Train finished \n");
  return;
}

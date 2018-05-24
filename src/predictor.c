//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "Ge Chang, Wenyu Zhang";
const char *studentID   = "A53240181, A53238371";
const char *email       = "chg073@eng.ucsd.edu, wez078@ucsd.edu";

//------Custom:TAGE branch predictor

/* 1. Introduction
 *
 *   The conception of TAGE predictor is introduced in http://www.irisa.fr/caps/people/seznec/JILP-COTTAGE.pdf.
 * Based on the simulation in this paper, we choose our own  proper parameters and then implement our branch predictor.
 *
 *   The TAGE branch predictor, known as TAgged GEometric histLen length, is one kind of hybrid branch predictor combining
 * the GEometric histLen length as well as the PPM-like predictor. It relies on the hit-miss detection as the prediction
 * and update computation.
 *
 *   In TAGE, the predictor has two components, one is a simple bimodal predictor which offers basic prediction when the
 * TAGE table can't offer the prediction, the others are M distinct predictors tables rely on the branch address and the
 * globalHis branch.
 *
 *
 */



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
//bimodal predictor
int8_t biPredictor[BIMODAL_LENGTH];

//TAGE table
const uint8_t GEOMETRICS[TABLE_NUM] = {128, 64, 32, 16, 8, 4, 2, 0};

typedef struct EntryStruct{
    int8_t saturateCounter;
    uint16_t tag;
    int8_t valid;
} Entry ;

typedef struct histCompressed{
    int8_t geometryLength;
    int8_t targetLength;
    uint32_t compressed;
} histCom ;

typedef struct BankStruct{
    int geometry;       //This is predefined by GEOMETRICS and never changes, so does not count into total number of bits used.
    Entry entry[1 << ENTRY_BIT];
    histCom indexCompressed;
    histCom tagCompressed[2];
} Bank;

uint8_t globalHistory[MAX_LEN];
uint32_t pathHistory;

Bank tageBank[TABLE_NUM];
uint8_t primaryBank = TABLE_NUM;
uint8_t alternateBank = TABLE_NUM;
uint8_t primaryPrediction = NOTTAKEN;
uint8_t alternatePrediction = NOTTAKEN;
uint8_t lastPrediction = NOTTAKEN;

uint32_t bankGlobalIndex[TABLE_NUM];
int tagResult[TABLE_NUM];

int8_t useAlternate = 8;




//------------------------------------//
//        Predictor calEntryunctions         //
//------------------------------------//

//Assistance functions

void modifyPrediction(uint8_t *counter, uint8_t result){
    if(result == TAKEN){
        if(*counter != ST) (*counter)++;
    }else{
        if(*counter != SN) (*counter)--;
    }
}


int calEntry(int table, int size, int bank) {
    int tmp1;
    int tmp2;
    table = table & ((1 << size) - 1);
    tmp1 = (table & ((1 << ENTRY_BIT) - 1));
    tmp2 = (table >> ENTRY_BIT);
    tmp2 = ((tmp2 << bank) & ((1 << ENTRY_BIT) - 1)) + (tmp2 >> (ENTRY_BIT - bank));
    table = tmp1 ^ tmp2;
    table = ((table << bank) & ((1 << ENTRY_BIT) - 1)) + (table >> (ENTRY_BIT - bank));
    return table;
}

uint32_t calIndex(uint32_t pc, int bankIdx) {
    int index;
    if (tageBank[bankIdx].geometry >= 16)
        index =
                pc ^ (pc >> ((ENTRY_BIT - (TABLE_NUM - bankIdx - 1)))) ^ tageBank[bankIdx].indexCompressed.compressed
                ^ calEntry(pathHistory, 16, bankIdx);

    else
        index =
                pc ^ (pc >> (ENTRY_BIT - TABLE_NUM + bankIdx + 1)) ^
                tageBank[bankIdx].indexCompressed.compressed ^
                calEntry(pathHistory, tageBank[bankIdx].geometry, bankIdx);

    return (uint32_t) (index & ((1 << (ENTRY_BIT)) - 1));

}

uint16_t calEntryTag(uint32_t pc, int bankIndex) {
    int tag = pc ^(tageBank[bankIndex].tagCompressed[0].compressed) ^((tageBank[bankIndex].tagCompressed[1].compressed) << 1);
    return (uint16_t) (tag & ((1 << (LEN_TAG - ((bankIndex + (TABLE_NUM & 1)) / 2))) - 1));
}

//------Static calEntryunctions------//
//no initialization in Static

//------Gshare calEntryunctions------//
//globalHis predictor

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
    //globalHis predictor
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

//globalHis predictor


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
    //globalHis predictor
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
//Init functions
void tagInit(){
    memset(biPredictor, (1 << (BIMODAL_COUNTER_SIZE / 2)) - 1, sizeof(uint8_t) * BIMODAL_LENGTH);
    for(uint32_t i = 0 ; i < TABLE_NUM ; i++){
        tageBank[i].geometry = GEOMETRICS[i];

        tageBank[i].indexCompressed.compressed = 0;
        tageBank[i].indexCompressed.geometryLength = GEOMETRICS[i];
        tageBank[i].indexCompressed.targetLength = ENTRY_BIT;

        tageBank[i].tagCompressed[0].compressed = 0;
        tageBank[i].tagCompressed[0].geometryLength = GEOMETRICS[i];
        tageBank[i].tagCompressed[0].targetLength = (int8_t) (LEN_TAG - ((i + (TABLE_NUM & 1)) / 2));
        tageBank[i].tagCompressed[1].compressed = 0;
        tageBank[i].tagCompressed[1].geometryLength = GEOMETRICS[i];
        tageBank[i].tagCompressed[1].targetLength = (int8_t) (LEN_TAG - ((i + (TABLE_NUM & 1)) / 2) - 1);

        for(uint32_t j = 0 ; j < (1 << ENTRY_BIT) ; j++){
            tageBank[i].entry[j].saturateCounter = 0;
            tageBank[i].entry[j].tag = 0;
            tageBank[i].entry[j].valid = 0;
        }

    }


    memset(bankGlobalIndex, 0, sizeof(uint32_t) * TABLE_NUM);
    memset(globalHistory, 0, sizeof(uint8_t) * MAX_LEN);
    pathHistory = 0;
    srand((unsigned int) time(NULL));
}

//bimodal prediction
uint8_t biPrediction(uint32_t pc){
    return (uint8_t) ((biPredictor[BIMODAL_INDEX(pc)] >= (1 << (BIMODAL_COUNTER_SIZE / 2))) ? TAKEN : NOTTAKEN);
}

//TAGE prediction

uint8_t tagePredictor(uint32_t pc){


    for(uint32_t i = 0 ; i < TABLE_NUM ; i++){
        tagResult[i] = calEntryTag(pc, i);
        bankGlobalIndex[i] = calIndex(pc, i);
    }

    primaryPrediction = NOTTAKEN;
    alternatePrediction = NOTTAKEN;
    primaryBank = TABLE_NUM;
    alternateBank = TABLE_NUM;

    for(uint8_t i = 0 ; i < TABLE_NUM ; i++){
        if(tageBank[i].entry[bankGlobalIndex[i]].tag == tagResult[i]){
            primaryBank = i; break;
        }
    }

    for(uint8_t i = primaryBank + 1 ; i < TABLE_NUM ; i++){
        if(tageBank[i].entry[bankGlobalIndex[i]].tag == tagResult[i]){
            alternateBank = i; break;
        }
    }

    if (primaryBank < TABLE_NUM) {
        if (alternateBank < TABLE_NUM) {
            alternatePrediction = ((tageBank[alternateBank].entry[bankGlobalIndex[alternateBank]]
                                            .saturateCounter >= 0)
                                   ? TAKEN : NOTTAKEN);
        }else {
            alternatePrediction = biPrediction(pc);
        }

        if((tageBank[primaryBank].entry[bankGlobalIndex[primaryBank]].saturateCounter != 0) ||
           (tageBank[primaryBank].entry[bankGlobalIndex[primaryBank]].saturateCounter != 1) ||
           (tageBank[primaryBank].entry[bankGlobalIndex[primaryBank]].valid != 0) ||
           (useAlternate < 8)
                ){
            lastPrediction = (tageBank[primaryBank].entry[bankGlobalIndex[primaryBank]].saturateCounter >= 0) ? TAKEN : NOTTAKEN;
        }
        else {
            lastPrediction = alternatePrediction;
        }

    } else {
        alternatePrediction = biPrediction(pc);
        lastPrediction = alternatePrediction;
    }

    return lastPrediction;
}


//Update function
void updateHist(histCom* histLen, uint8_t* globalHis){
    uint32_t newCompressed = (histLen->compressed << 1) + globalHis[0];
    newCompressed ^=  globalHis[histLen->geometryLength] << (histLen->geometryLength % histLen->targetLength);
    newCompressed ^= (newCompressed >> histLen->targetLength);
    newCompressed &= (1 << histLen->targetLength) - 1;
    histLen->compressed = newCompressed;

}


void updateSaturate(int8_t *counterVal, int taken, int completeLen) {
    if (taken) {
        if ((*counterVal) < ((1 << (completeLen - 1)) - 1)) {
            (*counterVal)++;
        }
    } else {
        if ((*counterVal) > -(1 << (completeLen - 1))) {
            (*counterVal)--;
        }
    }
}

void updateSaturateMinMax(int8_t *counterVal, int taken, int min, int max) {
    if (taken) {
        if ((*counterVal) < max) {
            (*counterVal)++;
        }
    } else {
        if ((*counterVal) > min) {
            (*counterVal)--;
        }
    }
}

//train functions
void tageTrain(uint32_t pc, uint8_t outcome) {
    
    int adjustSign = ((lastPrediction != outcome) & (primaryBank > 0));

    if (primaryBank < TABLE_NUM) {
        Entry entry = tageBank[primaryBank].entry[bankGlobalIndex[primaryBank]];
        
        int adjustSign2 =
                (entry.saturateCounter == -1 || entry.saturateCounter == 0)
                && (entry.valid == 0);

        if (adjustSign2) {
            if (primaryPrediction == outcome)
                adjustSign = 0;
            
            if (primaryPrediction != alternatePrediction) {
                updateSaturate(&useAlternate, alternatePrediction == outcome, 4);
                if (alternatePrediction == outcome) {
                    if (useAlternate < 7)
                        useAlternate++;
                } else if (useAlternate > -8){
                    useAlternate--;
                }
            }
        }
    }
    
    if (adjustSign) {
        
        int8_t min = 127;
        for (int i = 0; i < primaryBank; i++) {
            if (tageBank[i].entry[bankGlobalIndex[i]].valid < min){
                min = tageBank[i].entry[bankGlobalIndex[i]].valid;
            }
        }

        if (min > 0) {
            for (int i = primaryBank - 1; i >= 0; i--) {
                tageBank[i].entry[bankGlobalIndex[i]].valid--;
            }
        } else {
            int Y = rand() & ((1 << (primaryBank - 1)) - 1);
            int X = primaryBank - 1;
            while ((Y & 1) != 0) {
                X--;
                Y >>= 1;
            }
            
            for (int i = X; i >= 0; i--) {
                if (tageBank[i].entry[bankGlobalIndex[i]].valid == min) {
                    tageBank[i].entry[bankGlobalIndex[i]].tag = calEntryTag(pc, i);
                    tageBank[i].entry[bankGlobalIndex[i]].saturateCounter = (outcome == TAKEN) ? 0 : -1;
                    tageBank[i].entry[bankGlobalIndex[i]].valid = 0;
                    break;
                }
            }

        }
    }
    
    if (primaryBank < TABLE_NUM) {
        updateSaturate(&(tageBank[primaryBank].entry[bankGlobalIndex[primaryBank]].saturateCounter), outcome, LEN_COUNTS);
    } else {
        updateSaturateMinMax(&(biPredictor[BIMODAL_INDEX(pc)]), outcome, 0, (1 << BIMODAL_COUNTER_SIZE) - 1);

    }

    if ((lastPrediction != alternatePrediction)) {
        updateSaturateMinMax(&(tageBank[primaryBank].entry[bankGlobalIndex[primaryBank]].valid),
                             (lastPrediction == outcome),
                             0, 3);
    }
    
    for(int i = MAX_LEN - 1 ; i > 0 ; i--){
        globalHistory[i] = globalHistory[ i - 1];
    }
    globalHistory[0] = outcome ? TAKEN : NOTTAKEN;


    pathHistory = (pathHistory << 1) + (pc & 1);
    pathHistory = (pathHistory & ((1 << 16) - 1));
    for (int i = 0; i < TABLE_NUM; i++) {
        updateHist(&(tageBank[i].indexCompressed), globalHistory);
        updateHist(&(tageBank[i].tagCompressed[0]), globalHistory);
        updateHist(&(tageBank[i].tagCompressed[1]), globalHistory);
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
            tagInit();
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
        return tagePredictor(pc);
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
            tageTrain(pc, result);
            break;
        default:
            break;
    }
    return;
}

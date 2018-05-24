//
// Created by Ge Chang on 2018/5/22.
//

#ifndef CSE240A_TAGE_H
#define CSE240A_TAGE_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define NOTTAKEN  0
#define TAKEN     1

//------TAGE branch predictor

/* 1. Introduction
 *
 *   The conception of TAGE predictor is introduced in http://www.irisa.fr/caps/people/seznec/JILP-COTTAGE.pdf.
 * Based on the simulation in this paper, we choose our own  proper parameters and then implement our branch predictor.
 *
 *   The TAGE branch predictor, known as TAgged GEometric history length, is one kind of hybrid branch predictor combining
 * the GEometric history length as well as the PPM-like predictor. It relies on the hit-miss detection as the prediction
 * and update computation.
 *
 *   In TAGE, the predictor has two components, one is a simple bimodal predictor which offers basic prediction when the
 * TAGE table can't offer the prediction, the others are M distinct predictors tables rely on the branch address and the
 * global branch.
 *
 * 2. Structures of code
 * 
 * 
 *
 *
 *
 *
 *
 */

/*
 * Implementation of TAGE Predictor based on:
 *  A case for (partially)-tagged geometric history length predictors
 *  Andre Seznec, IRISA
 *  http://www.irisa.fr/caps/people/seznec/JILP-COTTAGE.pdf
 *  http://www.irisa.fr/caps/
 *  with slight modifications.
 *
 *  Configuration:
 *
 *  1. 1x basic bimodal predictor, There are BIMODAL_LENGTH = 4099 entries in it.
 *     Each entry is a 2-bit saturate counter.
 *     #define BIMODAL_LENGTH 4099
 *     #define BIMODAL_COUNTER_SIZE 2
 *     int8_t biPredictor[BIMODAL_LENGTH];    // Bimodal Predictor table
 *     Size for bimodal predictor: 2 * (4099) = 8198 bits.
 *
 *  2. 7x TAGE tables
 *     #define TABLE_NUM 7                      // 7x TAGE tables
 *     #define LEN_TAG 10                       // 10 bit tag length
 *     #define ENTRY_BIT 9                     // index into global table in each bank is 9 bit, thus there are 2^9 = 512 entries in each table.
 *     #define LEN_COUNTS 3                     // Each entry has a 3 bit saturate counter, 2 bit valid counter, 10 bit tag.
 *     typedef struct histCompressed{
            int8_t geometryLength;
            int8_t targetLength;
            uint32_t compressed;
        } histCom ;                   // 8 + 8 + 32 = 48 bits.

        typedef struct EntryStruct{
            int8_t saturateCounter;             // 3 bit saturate counter
            uint16_t tag;                       // 10 bit tag
            int8_t valid;                  // 2 bit valid counter
        } Entry ;                           // 15 bits in total.

       typedef struct BankStruct{
            int geometry;                       //This is predefined by GEOMETRICS and never changes, so does not count into total number of bits used.
            Entry entry[1 << ENTRY_BIT];   // (1 << ENTRY_BIT) = (1 << 9) = 512 items for each table
            histCom indexCompressed;  // 48 bits.
            histCom tagCompressed[2]; // 48 * 2 bits.
        } Bank;


 *     Total space for entries: 512 * 15 = 7680 bits.
 *     Each table has 3 histCom, (8 + 8 + 32) = 48 bit.
 *     Total size: 7 * (48 + 15 + 7680) = 54201 bits
 *
 *
 *  3. 1x Global History Table
 *     #define MAX_LEN 131
 *     uint8_t globalHistory[MAX_LEN];
 *
 *     MAX_LEN = 131 entries, 1 bit per entry
 *     Total space: 131 bits.
 *
 *  4. BankGlobalIndex:
 *     uint32_t bankGlobalIndex[TABLE_NUM];
 *     Store the entry index to each bank.
 *     Each entry consumes ENTRY_BIT = 9 bit.
 *     Total space: 9 * 7 = 63 bits.
 *
 *  5. tagResult:
 *     int tagResult[TABLE_NUM];
 *     Store the 10-bit tag to each bank in last computation.
 *     Total space: 10 * 7 = 70 bits.
 *
 *  Total size:
 *     8198 + 54201 + 131 + 63 + 70 = 62663 bits.
 *
 */

//------predictor parameters------//

//bimodal











#endif //CSE240A_TAGE_H

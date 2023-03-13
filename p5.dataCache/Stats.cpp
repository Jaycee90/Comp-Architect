/************************************************************************
 * Submitted by: Jean Claude Turambe, jnn56
 * CS 3339 - Spring 2022, Texas State University
 * Project 5 Data cache
 * Copyright 2022, Lee B. Hinkle, all rights reserved
 * Based on prior work by Martin Burtscher and Molly O'Neil
 * Redistribution in source or binary form, with or without modification,
 * is *not* permitted. Use in source or binary form, with or without
 * modification, is only permitted for academic use in CS 3339 at
 * Texas State University.
 ************************************************************************/
 
#include "Stats.h"

Stats::Stats() {
  cycles = PIPESTAGES - 1; // pipeline startup cost
  flushes = 0;
  stalls = 0;
  bubbles = 0;

  memops = 0;
  branches = 0;
  taken = 0;

  for(int i = IF1; i < PIPESTAGES; i++) {
  	rawHazards[i] = 0;//keep track of RAW hazards statistics
    resultReg[i] = -1;
    resultStage[i] = 0;//advance stages every time resulReg[] advance
  }
}

void Stats::clock(PIPESTAGE pipeStage) {
  cycles++;

  // advance all stages in pipeline
  for(int i = WB; i > pipeStage; i--) {
    resultReg[i] = resultReg[i-1];
    resultStage[i] = resultStage[i-1];
  }
  // inject a NOP into pipestage 
  resultReg[pipeStage] = -1;
  resultStage[pipeStage] = 0;
}

void Stats::registerSrc(int r,PIPESTAGE needed) {
	int bubbleCount = 0;
	int valValid = 0;
	int neededVal = 0;
	for (int i = EXE1; i < WB; i++){
		if(resultReg[i] == r){
			rawHazards[i]++;
			valValid = resultStage[i]-i;
			neededVal = needed - ID;
			
			if(valValid > neededVal){
				bubbleCount = valValid - neededVal;
			}
			for(int bub = 0; bub < bubbleCount; bub++){
				bubble();
			}
			break;	
		}
	}
}

void Stats::registerDest(int r,PIPESTAGE available) {
	resultReg[ID] = r;
	resultStage[ID] = available;	
}

void Stats::flush(int count) {// count == how many ops to flush
	for (int i = 0; i < count; i++){
		flushes++;
		clock(IF1); // Leverage clock(IF1) for flush
	}
}

void Stats::bubble(){
	bubbles++;
	clock(EXE1);
}

void Stats::stall(int stallCount) {
    stalls += stallCount;
    cycles += stallCount;
}

void Stats::showPipe() {
  // this method is to assist testing and debug, please do not delete or edit
  // you are welcome to use it but remove any debug outputs before you submit
  cout << "              IF1  IF2 *ID* EXE1 EXE2 MEM1 MEM2 WB         #C      #B      #F" << endl; 
  cout << "  resultReg ";
  for(int i = 0; i < PIPESTAGES; i++) {
    cout << "  " << dec << setw(2) << resultReg[i] << " ";
  }
  cout << "   " << setw(7) << cycles << " " << setw(7) << bubbles << " " << setw(7) << flushes;
  cout << endl;
}

/***********************************************************************
 * Submitted by: Jean Claude Turambe, jnn56
 * :CS 3339 - Spring 2022, Texas State University
 * Project 5 Data Cache
 * Copyright 2021, Lee B. Hinkle, all rights reserved
 * Based on prior work by Martin Burtscher and Molly O'Neil
 * Redistribution in source or binary form, with or without modification,
 * is *not* permitted. Use in source or binary form, with or without
 * modification, is only permitted for academic use in CS 3339 at
 * Texas State University.
 **********************************************************************/

#include "CPU.h"

const string CPU::regNames[] = {"$zero","$at","$v0","$v1","$a0","$a1","$a2","$a3",
                                "$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7",
                                "$s0","$s1","$s2","$s3","$s4","$s5","$s6","$s7",
                                "$t8","$t9","$k0","$k1","$gp","$sp","$fp","$ra"};

CPU::CPU(uint32_t pc, Memory &iMem, Memory &dMem) : pc(pc), iMem(iMem), dMem(dMem) {
  for(int i = 0; i < NREGS; i++) {
    regFile[i] = 0;
  }
  hi = 0;
  lo = 0;
  regFile[28] = 0x10008000; // gp
  regFile[29] = 0x10000000 + dMem.getSize(); // sp

  instructions = 0;
  stop = false;
}

void CPU::run() {
  while(!stop) {
    instructions++;
    
    fetch();
    decode();
    execute();
    mem();
    writeback();
    stats.clock(IF1);// start advancing stage into pipeline
    
    //stats.showPipe();//turn it off before submission

    D(printRegFile());
  }
}

void CPU::fetch() {
  instr = iMem.loadWord(pc);
  pc = pc + 4;
}

/////////////////////////////////////////
// ALL YOUR CHANGES GO IN THIS FUNCTION 
/////////////////////////////////////////
void CPU::decode() {
  uint32_t opcode;      // opcode field
  uint32_t rs, rt, rd;  // register specifiers
  uint32_t shamt;       // shift amount (R-type)
  uint32_t funct;       // funct field (R-type)
  uint32_t uimm;        // unsigned version of immediate (I-type)
  int32_t simm;         // signed version of immediate (I-type)
  uint32_t addr;        // jump address offset field (J-type)

  opcode = (instr >> 26) & 0x3f; //opcode field is 6 bits long
  rs = (instr >> 21) & 0x1f;    //Value of 5bits long
  rt = (instr >> 16) & 0x1f;   //Value of 5bits long
  rd = (instr >> 11) & 0x1f;  //Value of 5bits long
  shamt = (instr >> 6) & 0x1f; //value of 5bits long(6 to 10)
  funct = instr & 0x3f; //Value of 6 bits long (0 to 5).
  uimm = instr & 0xffff; //16 bit immediate value(0-15)
  simm = ((signed)uimm << 16) >> 16;//16 bit signed immediate value
  addr = instr & 0x3ffffff;//26-bit shortened address of the destination(0-25).

  // Control signals' safe default values
  opIsLoad = false;
  opIsStore = false;
  opIsMultDiv = false;
  aluOp = ADD;
  writeDest = false;
  destReg = 0;
  aluSrc1 = 0;
  aluSrc2 = 0;
  storeData = 0;

  D(cout << "  " << hex << setw(8) << pc - 4 << ": ");
  switch(opcode) {
    case 0x00:
      switch(funct) {
        case 0x00: D(cout << "sll " << regNames[rd] << ", " << regNames[rs] << ", " << dec << shamt);
                   writeDest = true; destReg = rd; stats.registerDest(rd,MEM1);
				   aluOp = SHF_L;
				   aluSrc1 = regFile[rs]; stats.registerSrc(rs,EXE1);
				   aluSrc2 = shamt;    
                   break; 
                   
        case 0x03: D(cout << "sra " << regNames[rd] << ", " << regNames[rs] << ", " << dec << shamt);
                   writeDest = true; destReg = rd; stats.registerDest(rd,MEM1);
                   aluOp = SHF_R;
                   aluSrc1 = regFile[rs]; stats.registerSrc(rs,EXE1);
                   aluSrc2 = shamt;
				   break; 
				   
        case 0x08: D(cout << "jr " << regNames[rs]);
                   pc = regFile[rs]; stats.registerSrc(rs,ID); stats.flush(2);//PC jump to the contents of the first source register.
				   break;
				   
        case 0x10: D(cout << "mfhi " << regNames[rd]);
                   writeDest = true; destReg = rd; stats.registerDest(rd,MEM1);
                   aluOp =ADD;//ADD is used with register ZERO to move
                   aluSrc1 = hi; stats.registerSrc(REG_HILO,EXE1);
                   aluSrc2 = regFile[REG_ZERO]; 
				   break;
				   
        case 0x12: D(cout << "mflo " << regNames[rd]);
                   writeDest = true; destReg = rd; stats.registerDest(rd,MEM1);
                   aluOp =ADD;//add is used with register ZERO to move
                   aluSrc1 = lo; stats.registerSrc(REG_HILO,EXE1);
                   aluSrc2 = regFile[REG_ZERO]; 
				   break;
				   
        case 0x18: D(cout << "mult " << regNames[rs] << ", " << regNames[rt]);
                   writeDest = true; destReg = REG_HILO; stats.registerDest(REG_HILO,WB); opIsMultDiv = true;
                   aluOp = MUL;
                   aluSrc1 = regFile[rs]; stats.registerSrc(rs,EXE1);
                   aluSrc2 = regFile[rt];stats.registerSrc(rt,EXE1); 
				   break;
				   
        case 0x1a: D(cout << "div " << regNames[rs] << ", " << regNames[rt]);
                   writeDest = true; destReg = REG_HILO; stats.registerDest(REG_HILO,WB); opIsMultDiv = true; 
                   aluOp = DIV;
                   aluSrc1 = regFile[rs]; stats.registerSrc(rs,EXE1);
                   aluSrc2 = regFile[rt]; stats.registerSrc(rt,EXE1);
				   break;
				   
        case 0x21: D(cout << "addu " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
                   writeDest = true; destReg = rd; stats.registerDest(rd,MEM1);
                   aluOp = ADD;
                   aluSrc1 = regFile[rs]; stats.registerSrc(rs,EXE1);
                   aluSrc2 = regFile[rt]; stats.registerSrc(rt,EXE1);
				   break;
				   
        case 0x23: D(cout << "subu " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
                   writeDest = true; destReg = rd; stats.registerDest(rd,MEM1);
                   aluOp = ADD;
                   aluSrc1 = regFile[rs]; stats.registerSrc(rs,EXE1);
                   aluSrc2 = -(regFile[rt]); stats.registerSrc(rt,EXE1);//Negating the register value to subtract
				   break;
				   
        case 0x2a: D(cout << "slt " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
                   writeDest = true; destReg = rd; stats.registerDest(rd,MEM1);
                   aluOp = CMP_LT;
                   aluSrc1 = regFile[rs]; stats.registerSrc(rs,EXE1);
                   aluSrc2 = regFile[rt]; stats.registerSrc(rt,EXE1);
				   break;
				   
        default: cerr << "unimplemented instruction: pc = 0x" << hex << pc - 4 << endl;
      }
      break;
    case 0x02: D(cout << "j " << hex << ((pc & 0xf0000000) | addr << 2)); // P1: pc + 4
               pc = (pc & 0xf0000000) | addr << 2; stats.flush(2);//update PC
			   break;
			   
    case 0x03: D(cout << "jal " << hex << ((pc & 0xf0000000) | addr << 2)); // P1: pc + 4
               writeDest = true; destReg = REG_RA; // writes PC+4 to $ra
               aluOp = ADD; //ALU should pass pc thru unchanged
               stats.registerDest(REG_RA,EXE1);
               aluSrc1 = pc;
               aluSrc2 = regFile[REG_ZERO]; // always reads zero
               pc = (pc & 0xf0000000) | addr << 2;
               stats.flush(2);
               break;
               
    case 0x04: D(cout << "beq " << regNames[rs] << ", " << regNames[rt] << ", " << pc + (simm << 2));
               //comparing registers for equality and update the PC field
               stats.registerSrc(rs,ID); 
			   stats.registerSrc(rt,ID); stats.countBranch(); 
			   if((signed)regFile[rs] == (signed)regFile[rt]){
			   pc = pc + (simm << 2);
			   stats.countTaken(); 
			   stats.flush(2);
			   }
			   break; 
			    
    case 0x05: D(cout << "bne " << regNames[rs] << ", " << regNames[rt] << ", " << pc + (simm << 2));
               //comparing registers for equality and update the PC field
               stats.registerSrc(rs,ID); 
			   stats.registerSrc(rt,ID); stats.countBranch(); 
			   if((signed)regFile[rs] != (signed)regFile[rt]){
			   pc = pc + (simm << 2);
			   stats.countTaken(); 
			   stats.flush(2);
			   }
			   break; 
			   
    case 0x09: D(cout << "addiu " << regNames[rt] << ", " << regNames[rs] << ", " << dec << simm);
               writeDest = true; destReg = rt; stats.registerDest(rt,MEM1); 
               aluOp = ADD;
               aluSrc1 = regFile[rs]; stats.registerSrc(rs,EXE1); 
               aluSrc2 = simm;
               break;
               
    case 0x0c: D(cout << "andi " << regNames[rt] << ", " << regNames[rs] << ", " << dec << uimm);
               writeDest = true; destReg = rt; stats.registerDest(rt,MEM1); 
               aluOp = AND;               
               aluSrc1 = regFile[rs]; stats.registerSrc(rs,EXE1);     
               aluSrc2 = uimm;
			   break;
			   
    case 0x0f: D(cout << "lui " << regNames[rt] << ", " << dec << simm);
               writeDest = true; destReg = rt; stats.registerDest(rt,MEM1); 
               aluOp = SHF_L;               
               aluSrc1 = simm;
               aluSrc2 = 16;//16 bits are stored in the register
			   break; 
    case 0x1a: D(cout << "trap " << hex << addr);
               switch(addr & 0xf) {
                 case 0x0: cout << endl; break;
                 case 0x1: cout << " " << (signed)regFile[rs]; stats.registerSrc(rs,EXE1);
                           break;
                 case 0x5: cout << endl << "? "; cin >> regFile[rt]; stats.registerDest(rt,MEM1);
                           break;
                 case 0xa: stop = true; break;
                 default: cerr << "unimplemented trap: pc = 0x" << hex << pc - 4 << endl;
                          stop = true;
               }
               break;
    case 0x23: D(cout << "lw " << regNames[rt] << ", " << dec << simm << "(" << regNames[rs] << ")");
			   writeDest = true; destReg = rt;  
			   opIsLoad = true;
               aluOp = ADD;
               stats.registerDest(rt,WB);
               stats.registerSrc(rs,EXE1);
               stats.countMemOp();
               aluSrc1 = regFile[rs]; 
               aluSrc2 = simm;
			   break;  
    case 0x2b: D(cout << "sw " << regNames[rt] << ", " << dec << simm << "(" << regNames[rs] << ")");
               //store word doesn't write to register. 
			   //notify stats that we are reading regFile pointed out to by rt.
               opIsStore = true; storeData = regFile[rt];  
               aluOp = ADD;
               stats.registerSrc(rs,EXE1);
               stats.registerSrc(rt,MEM1);
               stats.countMemOp();
               aluSrc1 = regFile[rs]; 
               aluSrc2 = simm;
               break;  
    default: cerr << "unimplemented instruction: pc = 0x" << hex << pc - 4 << endl;
  }
  D(cout << endl);
}

void CPU::execute() {
  aluOut = alu.op(aluOp, aluSrc1, aluSrc2);
}

void CPU::mem() {
  if(opIsLoad){
  	stats.stall(cacheStats.access(aluOut,LOAD));
  	writeData = dMem.loadWord(aluOut);
  }else
    writeData = aluOut;

  if(opIsStore){
  	stats.stall(cacheStats.access(aluOut,STORE));
  	dMem.storeWord(storeData, aluOut);
  }
}

void CPU::writeback() {
  if(writeDest && destReg > 0) // skip when write is to zero_register
    regFile[destReg] = writeData;
  
  if(opIsMultDiv) {
    hi = alu.getUpper();
    lo = alu.getLower();
  }
}

void CPU::printRegFile() {
  cout << hex;
  for(int i = 0; i < NREGS; i++) {
    cout << "    " << regNames[i];
    if(i > 0) cout << "  ";
    cout << ": " << setfill('0') << setw(8) << regFile[i];
    if( i == (NREGS - 1) || (i + 1) % 4 == 0 )
      cout << endl;
  }
  cout << "    hi   : " << setfill('0') << setw(8) << hi;
  cout << "    lo   : " << setfill('0') << setw(8) << lo;
  cout << dec << endl;
}

void CPU::printFinalStats() {
  cout << "Program finished at pc = 0x" << hex << pc << "  ("
       << dec << instructions << " instructions executed)" << endl;
  cout << endl;

  cout << "Cycles: " << stats.getCycles() <<endl;
  cout << "CPI: " << fixed << setprecision(2)<<(1.0 * stats.getCycles() / instructions) << endl<< endl;
  cout << "Bubbles: " << stats.getBubbles() << endl;
  cout << "Flushes: " << stats.getFlushes() << endl;
  
  cout << "Stalls:  " << stats.getStalls() << endl <<endl;
  cacheStats.printFinalStats();
}

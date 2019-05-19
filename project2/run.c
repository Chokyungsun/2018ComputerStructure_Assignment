/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*   SCE212 Ajou University                                    */
/*   run.c                                                     */
/*   Adapted from CS311@KAIST                                  */
/*                                                             */
/***************************************************************/

#include <stdio.h>

#include "util.h"
#include "run.h"
int count = 1;

/***************************************************************/
/*                                                             */
/* Procedure: get_inst_info                                    */
/*                                                             */
/* Purpose: Read insturction information                       */
/*                                                             */
/***************************************************************/
instruction* get_inst_info(uint32_t pc)
{
	return &INST_INFO[(pc - MEM_TEXT_START) >> 2];
}

/***************************************************************/
/*                                                             */
/* Procedure: process_instruction                              */
/*                                                             */
/* Purpose: Process one instrction                             */
/*                                                             */
/***************************************************************/
void process_instruction()
{
	/*if(NUM_INST < ++count){
		CURRENT_STATE.PC -= 4;
		RUN_BIT = FALSE;
		return;
	}*/

	//printf("NUM_INST: %d, count: %d\n", NUM_INST, count);

	instruction* instr;
	instr = get_inst_info(CURRENT_STATE.PC);
	if(instr->opcode == 0 && instr->r_t.r_i.r_i.r.rd == 0 && instr->r_t.r_i.r_i.r.shamt == 0 && instr->func_code == 0){
		RUN_BIT = FALSE;
		return;
	}

	switch(instr->opcode)
	{
            //Type I
            case 0x9:		//(0x001001)ADDIU
            CURRENT_STATE.REGS[instr->r_t.r_i.rt] = CURRENT_STATE.REGS[instr->r_t.r_i.rs] + instr->r_t.r_i.r_i.imm;
            CURRENT_STATE.PC += 4;
            break;
            case 0xc:		//(0x001100)ANDI
            CURRENT_STATE.REGS[instr->r_t.r_i.rt] = CURRENT_STATE.REGS[instr->r_t.r_i.rs] & instr->r_t.r_i.r_i.imm;
            CURRENT_STATE.PC += 4;
            break;
            case 0xf:		//(0x001111)LUI
            CURRENT_STATE.REGS[instr->r_t.r_i.rt] = (instr->r_t.r_i.r_i.imm) << 16;
            CURRENT_STATE.PC += 4;
            break;
            case 0xd:		//(0x001101)ORI
            CURRENT_STATE.REGS[instr->r_t.r_i.rt] = CURRENT_STATE.REGS[instr->r_t.r_i.rs] + instr->r_t.r_i.r_i.imm;
            CURRENT_STATE.PC += 4;
            break;
            case 0xb:		//(0x001011)SLTIU
            CURRENT_STATE.REGS[instr->r_t.r_i.rt] = (CURRENT_STATE.REGS[instr->r_t.r_i.rs] < instr->r_t.r_i.r_i.imm) ? 1 : 0;
            CURRENT_STATE.PC += 4;
            break;
            case 0x23:		//(0x100011)LW
            CURRENT_STATE.REGS[instr->r_t.r_i.rt] = mem_read_32(CURRENT_STATE.REGS[instr->r_t.r_i.rs] + instr->r_t.r_i.r_i.imm);
            CURRENT_STATE.PC += 4;
            	//printf("%x\n", CURRENT_STATE.REGS[instr->r_t.r_i.rt]);
            break;
            case 0x2b:		//(0x101011)SW
            mem_write_32(CURRENT_STATE.REGS[instr->r_t.r_i.rs] + instr->r_t.r_i.r_i.imm, CURRENT_STATE.REGS[instr->r_t.r_i.rt]);
            CURRENT_STATE.PC += 4;
            	//printf("%x\n", CURRENT_STATE.REGS[instr->r_t.r_i.rt]);
            break;
            case 0x4:		//(0x000100)BEQ
            if(CURRENT_STATE.REGS[instr->r_t.r_i.rs] == CURRENT_STATE.REGS[instr->r_t.r_i.rt])
            {
            	CURRENT_STATE.PC += 4 + (instr->r_t.r_i.r_i.imm)*4; 
            	break;
            }
            else{
            	CURRENT_STATE.PC += 4; 
            	break;
            }
            case 0x5:		//(0x000101)BNE
            if(CURRENT_STATE.REGS[instr->r_t.r_i.rs] != CURRENT_STATE.REGS[instr->r_t.r_i.rt])
            {
            	CURRENT_STATE.PC += 4 + (instr->r_t.r_i.r_i.imm)*4; 
            	break;
            }
            else{
            	CURRENT_STATE.PC += 4; 
            	break;
            }

            //TYPE R
            case 0x0:		//(0x000000)ADDU, AND, NOR, OR, SLTU, SLL, SRL, SUBU  if JR
            switch(instr->func_code){
            		case 0x21:		//(0x100001)ADDU
            		CURRENT_STATE.REGS[instr->r_t.r_i.r_i.r.rd] = CURRENT_STATE.REGS[instr->r_t.r_i.rs] + CURRENT_STATE.REGS[instr->r_t.r_i.rt];
            		CURRENT_STATE.PC += 4; 
            		break;
            		case 0x24:		//(0x100010)AND
            		CURRENT_STATE.REGS[instr->r_t.r_i.r_i.r.rd] = CURRENT_STATE.REGS[instr->r_t.r_i.rs] & CURRENT_STATE.REGS[instr->r_t.r_i.rt];
            		CURRENT_STATE.PC += 4; 
            		break;
            		case 0x27:		//(0x101110)NOR
            		CURRENT_STATE.REGS[instr->r_t.r_i.r_i.r.rd] = ~(CURRENT_STATE.REGS[instr->r_t.r_i.rs] |CURRENT_STATE.REGS[instr->r_t.r_i.rt]);
            		CURRENT_STATE.PC += 4; 
            		break;
            		case 0x25:		//(0x100101)OR
            		CURRENT_STATE.REGS[instr->r_t.r_i.r_i.r.rd] = CURRENT_STATE.REGS[instr->r_t.r_i.rs] | CURRENT_STATE.REGS[instr->r_t.r_i.rt];
            		CURRENT_STATE.PC += 4; 
            		break;
            		case 0x2b:		//(0x101011)SLTU
            		CURRENT_STATE.REGS[instr->r_t.r_i.r_i.r.rd] = (CURRENT_STATE.REGS[instr->r_t.r_i.rs] < CURRENT_STATE.REGS[instr->r_t.r_i.rt]) ? 1 :0;
            		CURRENT_STATE.PC += 4; 
            		break;
            		case 0x00:		//(0x000000)SLL
            		CURRENT_STATE.REGS[instr->r_t.r_i.r_i.r.rd] = CURRENT_STATE.REGS[instr->r_t.r_i.rt] << instr->r_t.r_i.r_i.r.shamt;
            		CURRENT_STATE.PC += 4; 
            		break;
            		case 0x02:		//(0x000010)SRL
            		CURRENT_STATE.REGS[instr->r_t.r_i.r_i.r.rd] = CURRENT_STATE.REGS[instr->r_t.r_i.rt] >> instr->r_t.r_i.r_i.r.shamt;
            		CURRENT_STATE.PC += 4; 
            		break;
            		case 0x23:		//(0x100011)SUBU
            		CURRENT_STATE.REGS[instr->r_t.r_i.r_i.r.rd] = CURRENT_STATE.REGS[instr->r_t.r_i.rs] - CURRENT_STATE.REGS[instr->r_t.r_i.rt];
            		CURRENT_STATE.PC += 4; 
            		break;
            		case 0x08:		//(0x001000)JR
            		CURRENT_STATE.PC = CURRENT_STATE.REGS[instr->r_t.r_i.rs]; 
            		break;

            	}
            	break;

            //TYPE J
            case 0x2:		//(0x000010)J
            CURRENT_STATE.PC =  instr->r_t.target*4;
            break;
            case 0x3:		//(0x000011)JAL
            CURRENT_STATE.REGS[31] = CURRENT_STATE.PC + 8;
            CURRENT_STATE.PC = instr->r_t.target*4;
            break;

            default:
            CURRENT_STATE.PC -= 4;
            RUN_BIT = FALSE;
            return;
        }

        return;
    }

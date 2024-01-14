#include <stdio.h>
#include <stdlib.h>
#include "sim5.h"

/*
* Author: Minh Duong
* Purpose: Simulate a pipelined CPU that supports the following instructions::
* add, addu, sub, subu, addi, addiu
* and, or, xor, nor
* slt, slti
* lw, sw
* beq, bne, j
* andi, ori
* lui
*/


/*
* Function for extracting fields from the instruction, including opcode,
* rd, rs, rt, shamt, func, imm16, imm32, address.
*/
void extract_instructionFields(WORD instruction, InstructionFields* fieldsOut) {
	int i;
	fieldsOut->opcode = (instruction >> 26) & 0x3F;	// bits 26-31
	fieldsOut->rs = (instruction >> 21) & 0x1F;	// bit 21-25
	fieldsOut->rt = (instruction >> 16) & 0x1F;	// bit 16-20

	// R type
	fieldsOut->rd = (instruction >> 11) & 0x1F;	// bit 11-15
	fieldsOut->shamt = (instruction >> 6) & 0x1F;	// bit 6-10
	fieldsOut->funct = instruction & 0x3F;		// bit 0-5

	// I type
	fieldsOut->imm16 = instruction & 0xFFFF;	// bit 0-15
	//if (fieldsOut->opcode == 12) {
		//fieldsOut->imm32 = fieldsOut->imm16;
	//}
	//else {
		fieldsOut->imm32 = signExtend16to32(fieldsOut->imm16);
	//}

	// J type
	fieldsOut->address = instruction & 0x3FFFFFF;		// bits 0-25
}

/*
* Function for determining if a stall is required. If need to stall the ID phase,
* return 1; if not, then return 0.
*/
int IDtoIF_get_stall(InstructionFields* fields,
    ID_EX* old_idex, EX_MEM* old_exmem) {

	// in case previous instruction is a LW
	if (old_idex->memRead) {
		// if previous rt == current rs or 
        // previous rt == current rt (in case of R instruction), then stall
		if (old_idex->rt == fields->rs || old_idex->rt == fields->rt && fields->opcode == 0) {
			return 1;
		}
	}
    // in case current instruction is a SW 
    // forwarding cases
    if (fields->opcode == 0x2b) {
        if ((old_idex->rd == fields->rt && old_idex->regDst == 1) || (old_idex->rt == fields->rt && old_idex->regDst == 0)) {
            return 0;
        }
        // if 2 previous rt == current rt, then stall
        if (old_exmem->writeReg == fields->rt && old_exmem->regWrite) {
            return 1;
        }
    }
	return 0;

}


/*
* Function for determining if the program needs to perform branch/jump. 
* Return 0 if no b/j is needed, 1 if PC jumps to branch destination, 
* 2 if PC jumps to j destination
*/
int IDtoIF_get_branchControl(InstructionFields* fields, WORD rsVal, WORD rtVal) {
    // Instructions BEQ and BNE
    if ((fields->opcode == 0x04 && rsVal == rtVal) || (fields->opcode == 0x05 && rsVal != rtVal)) {
        return 1;
    }
    // Instruction J
    else if (fields->opcode == 0x02) {
        return 2;
    }
    return 0;
}


/*
* Function for calculating address that the code would jump to in BEQ, BNE
*/
WORD calc_branchAddr(WORD pcPlus4, InstructionFields* fields) {
    return pcPlus4 + (fields->imm32 << 2);
}


/*
* Function for calculating address that the code would jump to in J
*/
WORD calc_jumpAddr(WORD pcPlus4, InstructionFields* fields) {
    return ((pcPlus4 >> 28) << 28) | (fields->address << 2);
}


/*
* Function for executing the ID phase based on stall decision, instruction fields,
* rs, rt, then store data into the new ID_EX register, and return 1 if valid opcode,
* or 0 otherwise
*/
int execute_ID(int IDstall, InstructionFields* fieldsIn, WORD pcPlus4, WORD rsVal, WORD rtVal, ID_EX* new_idex) {
    WORD op = fieldsIn->opcode;
    WORD funct = fieldsIn->funct;

    // Copy instruction fields into new ID_EX
    new_idex->rs = fieldsIn->rs;
    new_idex->rt = fieldsIn->rt;
    new_idex->rd = fieldsIn->rd;
    new_idex->rsVal = rsVal;
    new_idex->rtVal = rtVal;
    new_idex->imm16 = fieldsIn->imm16;
    new_idex->imm32 = fieldsIn->imm32;

    // STALL
    if (IDstall) {
        new_idex->ALUsrc = 0;
        new_idex->ALU.bNegate = 0;
        new_idex->ALU.op = 0;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 0;
        new_idex->regWrite = 0;
        new_idex->extra1 = 0;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
        new_idex->rs = 0;
        new_idex->rt = 0;
        new_idex->rd = 0;
        new_idex->rsVal = 0;
        new_idex->rtVal = 0;
        new_idex->imm16 = 0;
        new_idex->imm32 = 0;
    }
    // ADD
    else if (op == 0x00 && funct == 0x20) {
        new_idex->ALUsrc = 0;
        new_idex->ALU.bNegate = 0;
        new_idex->ALU.op = 2;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 1;
        new_idex->regWrite = 1;
        new_idex->extra1 = 0;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
    }
    // ADDU
    else if (op == 0x00 && funct == 0x21) {
        new_idex->ALUsrc = 0;
        new_idex->ALU.bNegate = 0;
        new_idex->ALU.op = 2;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 1;
        new_idex->regWrite = 1;
        new_idex->extra1 = 0;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
    }
    // SUB
    else if (op == 0x00 && funct == 0x22) {
        new_idex->ALUsrc = 0;
        new_idex->ALU.bNegate = 1;
        new_idex->ALU.op = 2;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 1;
        new_idex->regWrite = 1;
        new_idex->extra1 = 0;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
    }
    // SUBU
    else if (op == 0x00 && funct == 0x23) {
        new_idex->ALUsrc = 0;
        new_idex->ALU.bNegate = 1;
        new_idex->ALU.op = 2;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 1;
        new_idex->regWrite = 1;
        new_idex->extra1 = 0;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
    }
    // ADDI
    else if (op == 0x08) {
        new_idex->ALUsrc = 1;
        new_idex->ALU.bNegate = 0;
        new_idex->ALU.op = 2;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 0;
        new_idex->regWrite = 1;
        new_idex->extra1 = 0;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
    }
    // ADDIU
    else if (op == 0x09) {
        new_idex->ALUsrc = 1;
        new_idex->ALU.bNegate = 0;
        new_idex->ALU.op = 2;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 0;
        new_idex->regWrite = 1;
        new_idex->extra1 = 0;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
    }
    // AND
    else if (op == 0x00 && funct == 0x24) {
        new_idex->ALUsrc = 0;
        new_idex->ALU.bNegate = 0;
        new_idex->ALU.op = 0;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 1;
        new_idex->regWrite = 1;
        new_idex->extra1 = 0;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
    }
    // OR
    else if (op == 0x00 && funct == 0x25) {
        new_idex->ALUsrc = 0;
        new_idex->ALU.bNegate = 0;
        new_idex->ALU.op = 1;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 1;
        new_idex->regWrite = 1;
        new_idex->extra1 = 0;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
    }
    // NOR 
    else if (op == 0x00 && funct == 0x27) {
        new_idex->ALUsrc = 0;
        new_idex->ALU.bNegate = 0;
        new_idex->ALU.op = 1;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 1;
        new_idex->regWrite = 1;
        new_idex->extra1 = 1;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
    }
    // XOR
    else if (op == 0x00 && funct == 0x26) {
        new_idex->ALUsrc = 0;
        new_idex->ALU.bNegate = 0;
        new_idex->ALU.op = 4;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 1;
        new_idex->regWrite = 1;
        new_idex->extra1 = 0;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
    }
    // SLT
    else if (op == 0x00 && funct == 0x2a) {
        new_idex->ALUsrc = 0;
        new_idex->ALU.bNegate = 1;
        new_idex->ALU.op = 3;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 1;
        new_idex->regWrite = 1;
        new_idex->extra1 = 0;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
    }
    // SLTI
    else if (op == 0x0a) {
        new_idex->ALUsrc = 1;
        new_idex->ALU.bNegate = 1;
        new_idex->ALU.op = 3;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 0;
        new_idex->regWrite = 1;
        new_idex->extra1 = 0;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
    }
    // LW
    else if (op == 0x23) {
        new_idex->ALUsrc = 1;
        new_idex->ALU.bNegate = 0;
        new_idex->ALU.op = 2;
        new_idex->memRead = 1;
        new_idex->memWrite = 0;
        new_idex->memToReg = 1;
        new_idex->regDst = 0;
        new_idex->regWrite = 1;
        new_idex->extra1 = 0;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
    }
    // SW
    else if (op == 0x2b) {
        new_idex->ALUsrc = 1;
        new_idex->ALU.bNegate = 0;
        new_idex->ALU.op = 2;
        new_idex->memRead = 0;
        new_idex->memWrite = 1;
        new_idex->memToReg = 0;
        new_idex->regDst = 0;
        new_idex->regWrite = 0;
        new_idex->extra1 = 0;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
    }
    // BEQ
    else if (op == 0x04) {
        new_idex->ALUsrc = 0;
        new_idex->ALU.bNegate = 0;
        new_idex->ALU.op = 0;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 0;
        new_idex->regWrite = 0;
        new_idex->extra1 = 0;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
        new_idex->rs = 0;
        new_idex->rt = 0;
        new_idex->rd = 0;
        new_idex->rsVal = 0;
        new_idex->rtVal = 0;
    }
    // BNE
    else if (op == 0x05) {
        new_idex->ALUsrc = 0;
        new_idex->ALU.bNegate = 0;
        new_idex->ALU.op = 0;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 0;
        new_idex->regWrite = 0;
        new_idex->extra1 = 0;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
        new_idex->rs = 0;
        new_idex->rt = 0;
        new_idex->rd = 0;
        new_idex->rsVal = 0;
        new_idex->rtVal = 0;
    }
    // J
    else if (op == 0x02) {
        new_idex->ALUsrc = 0;
        new_idex->ALU.bNegate = 0;
        new_idex->ALU.op = 0;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 0;
        new_idex->regWrite = 0;
        new_idex->extra1 = 0;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
        new_idex->rs = 0;
        new_idex->rt = 0;
        new_idex->rd = 0;
        new_idex->rsVal = 0;
        new_idex->rtVal = 0;
    }
    // ANDI
    else if (op == 0x0c) {
        new_idex->ALUsrc = 2;
        new_idex->ALU.bNegate = 0;
        new_idex->ALU.op = 0;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 0;
        new_idex->regWrite = 1;
        new_idex->extra1 = 0;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
    }
    // ORI
    else if (op == 0x0d) {
        new_idex->ALUsrc = 2;
        new_idex->ALU.bNegate = 0;
        new_idex->ALU.op = 1;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 0;
        new_idex->regWrite = 1;
        new_idex->extra1 = 0;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
    }
    // LUI
    else if (op == 0x0f) {
        new_idex->ALUsrc = 1;
        new_idex->ALU.bNegate = 0;
        new_idex->ALU.op = 2;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 0;
        new_idex->regWrite = 1;
        new_idex->extra1 = 0;
        new_idex->extra2 = 1;
        new_idex->extra3 = 0;
    }
    // NOP
    else if (op == 0x00 && funct == 0x00) {
        new_idex->ALUsrc = 0;
        new_idex->ALU.bNegate = 0;
        new_idex->ALU.op = 5;
        new_idex->memRead = 0;
        new_idex->memWrite = 0;
        new_idex->memToReg = 0;
        new_idex->regDst = 1;
        new_idex->regWrite = 1;
        new_idex->extra1 = 0;
        new_idex->extra2 = 0;
        new_idex->extra3 = 0;
        new_idex->rs = 0;
        new_idex->rt = 0;
        new_idex->rd = 0;
        new_idex->rsVal = 0;
        new_idex->rtVal = 0;
        new_idex->imm16 = 0;
        new_idex->imm32 = 0;
    }
    else {
        return 0;
    }
    return 1;
}


/*
* Function for getting the value which should be delivered to input 1
* of the ALU. Return the value.
*/
WORD EX_getALUinput1(ID_EX* in, EX_MEM* old_exMem, MEM_WB* old_memWb) {

    // data forwarding from old EX_MEM
    if (old_exMem->regWrite && old_exMem->writeReg == in->rs) {
        return old_exMem->aluResult;
    }

    // data forwarding from old MEM_WB
    else if (old_memWb->regWrite && old_memWb->writeReg == in->rs) {
        return old_memWb->aluResult;
    }
    
    return in->rsVal;
}



/*
* Function for getting the value which should be delivered to input 2
* of the ALU. Return the value.
*/
WORD EX_getALUinput2(ID_EX* in, EX_MEM* old_exMem, MEM_WB* old_memWb) {
    
    // For I-instructions:
    // used for ANDI, ORI (16-bit imm)
    if (in->ALUsrc == 2) {
        return in->imm16;
    }
    // For other I-instructions (32-bit imm)
    else if (in->ALUsrc == 1) {
        return in->imm32;
    }
    // For R-instructions:
    // If needs forwarding
    if (old_exMem->regWrite && old_exMem->writeReg == in->rt) {
        return old_exMem->aluResult;
    }
    else if (old_memWb->regWrite && old_memWb->writeReg == in->rt) {
        return old_memWb->aluResult;
    }
    return in->rtVal;
}


/*
* Function for executing the EX phase, based on ID_EX data and ALU inputs.
* Update EX_MEM register accordingly.
*/
void execute_EX(ID_EX* in, WORD input1, WORD input2, EX_MEM* new_exMem) {
    
    // copy ID_EX to next EX_MEM
    new_exMem->memRead = in->memRead;
    new_exMem->memWrite = in->memWrite;
    new_exMem->memToReg = in->memToReg;
    new_exMem->regWrite = in->regWrite;
    new_exMem->rtVal = in->rtVal;
    new_exMem->rt = in->rt;
    new_exMem->extra1 = in->extra1;
    new_exMem->extra2 = in->extra2;
    new_exMem->extra3 = in->extra3;
    new_exMem->aluResult = 0;

    if (in->ALUsrc >= 1) {
        // I format
        new_exMem->writeReg = in->rt;
    }
    else {
        // R format
        new_exMem->writeReg = in->rd;
    }
    // LUI
    if (in->extra2 == 1) {
        new_exMem->aluResult = in->imm16 << 16;
        return;
    }
    // bNegate
    if (in->ALU.bNegate) {
        input2 *= -1;
    }
    // ALU AND
    if (in->ALU.op == 0) {
        new_exMem->aluResult = input1 & input2;
    }
    // ALU OR
    else if (in->ALU.op == 1) {
        // NOR
        if (in->extra1 == 1) {
            new_exMem->aluResult = ~(input1 | input2);
        }
        // OR
        else {
            new_exMem->aluResult = input1 | input2;
        }
    }
    // ALU ADD
    else if (in->ALU.op == 2) {
        new_exMem->aluResult = input1 + input2;
    }
    // ALU LESS
    else if (in->ALU.op == 3) {
        new_exMem->aluResult = (input1 + input2) < 0;
    }
    // ALU XOR
    else if (in->ALU.op == 4) {
        new_exMem->aluResult = (input1 ^ input2);
    }
}


/*
* Function for executing the MEM phase, which will handle data forwarding 
* which may come from MEM_WB of a previous instruction.
*/
void execute_MEM(EX_MEM* in, MEM_WB* old_memWb, WORD* mem, MEM_WB* new_memwb) {
    
    // copy EX_MEM to MEM_WB
    new_memwb->aluResult = in->aluResult;
    new_memwb->memToReg = in->memToReg;
    new_memwb->regWrite = in->regWrite;
    new_memwb->writeReg = in->writeReg;
    new_memwb->extra1 = in->extra1;
    new_memwb->extra2 = in->extra2;
    new_memwb->extra3 = in->extra3;

    // In case of LW
    if (in->memToReg == 1) {
        // read address computed by ALU and read data from the address in memory
        new_memwb->memResult = mem[in->aluResult / 4];
    }
    else {
        // In case of SW
        if (in->memWrite == 1) {
            // data forwarding from old MEM_WB
            if (old_memWb->writeReg == in->rt && old_memWb->regWrite) {
                if (old_memWb->memToReg) {
                    mem[in->aluResult / 4] = old_memWb->memResult;
                }
                else {
                    mem[in->aluResult / 4] = old_memWb->aluResult;
                }
            }
            else {
                // read address computed by ALU and save data into the address in memory
                mem[in->aluResult / 4] = in->rtVal;
            }
        }
        new_memwb->memResult = 0;
    }

}


/*
* Function for executing the WB phase. Update the appropriate register if needed.
*/
void execute_WB(MEM_WB* in, WORD* regs) {
    if (in->regWrite == 1) {
        if (in->memToReg == 1) {
            regs[in->writeReg] = in->memResult;
        }
        else {
            regs[in->writeReg] = in->aluResult;
        }
    }
}


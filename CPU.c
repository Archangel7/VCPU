/**********************************
 * Author: Michael Cox
 * Creation date: Feburary 27, 2015
 ***********************************/
#include <stdio.h>
#include <stdint.h>
#include "functionDef2.h"
#define numOfRegisters 16
#define memlocation 0x00003FFF
#define MSBmask 0xFF000000
#define numOfShifts 4
#define MEMSIZE 16384
#define regMSB 0x80
#define maskvalue 0xE000
#define RDmask 0x000F
#define MAX32 32
#define RNmask 0x00F0
#define Rmask 0x100
#define valuemask 0x0FF0
#define opcodemask 0x3000
#define loadmask 0x800
#define bitmask 0x3FFF
#define operationmask 0xF00
#define Hmask 0x400
#define conditionmask 0xF00
#define memmask 0x00FF
#define offsetmask 0xFFF
#define linkbitmask 0x1000
#define reglistmask 0x00FF
#define bytemask 0x400
#define PC registers[15]
#define SP registers[13]
#define LR registers[14]
#define sizeofreg 4
#define registersize 32
#define dataprocess 0x0000
#define immediateoperations 0x0002
#define immediateoperationss 0x0003
#define conditionalbranch 0x0004
#define pushpull 0x0005
#define ubranch 0x0006
#define stop 0x0007
#define AND 0x0000
#define EOR 0x0001
#define SUB 0x0002
#define SXB 0x0003
#define ADD 0x0004
#define ADC 0x0005
#define LSR 0x0006
#define LSL 0x0007
#define TST 0x0008
#define TEQ 0x0009
#define CMP 0x0010
#define ROR 0x0011
#define ORR 0x0012
#define MOV 0x0013
#define BIC 0x0014
#define MVN 0x0015
#define zset 0x0000
#define zclear 0x0001
#define cset 0x0002
#define cclear 0x0003
#define nset 0x0004
#define nclear 0x0005
#define cset_zclear 0x0008
#define cclear_zset 0x0009
#define ignored 0x000E
#define imm_MOV 0x0000
#define imm_ADD 0x0002
#define imm_SUB 0x0003
#define imm_CMP 0x0001
#define push 0
#define pull 1
#define bits 8
#define IR1mask 0xFFFF
#define IR0shift 16

typedef struct
{
    int stopp;
    int carry;
    int sign;
    int IRactive;
    int zero;
}FLAGS;

FLAGS flags;
char branch;
short rel_address;
uint32_t MBR;
uint32_t MAR;
uint32_t IR = 0;
unsigned short IR0 = 0;
unsigned short IR1 = 0;
unsigned short RD = 0;
int counter = 0;
unsigned short RN = 0;
unsigned short L = 0;
unsigned short B = 0;
unsigned memaddress = 0;
uint32_t ALU = 0;
int regcounterr = 0;
unsigned R = 0;
unsigned condition = 0;
unsigned linkbit = 0;
unsigned offset = 0;
unsigned operation = 0;
unsigned short byte;
unsigned H = 0;
uint16_t reglist = 0;
uint8_t regcounter = 0;
unsigned long registers[numOfRegisters-1] = {0};

/************************************
* Function name: clearFlags
* Description: 
*   This function clears the flags.
************************************/
void clearFlags()
{
    flags.stopp  = 0;
    flags.carry = 0;
    flags.IRactive = 0;
    flags.sign  = 0;
    flags.zero  = 0;
}
/***********************************************
 * Function name: clearRegisters
 * Description: 
 *    This function resets all the registers
 *    to zero
 ***********************************************/
void clearRegisters()
{
    int count = 0;
    for( count = 0; count <= numOfRegisters; count ++)
    {
		registers[count] = 0;
    }
    MBR = 0;
    MAR = 0;
    IR  = 0;
}
/******************************************************
 * Function name: displayRegisters
 * Description:
 *     This function displays all the 
 *     registers and flags
 ******************************************************/
void displayRegisters()
{
    int count;

    for (count = 0; count <= numOfRegisters-4; count ++)
    {
	printf("r[%i]:%08lX   ",count, registers[count]);
    }
    printf("\b\b ");
    printf("r[13](SP):%08X            r[14](LR):%08X r[15](PC):%08X",SP,LR,PC);

    printf("\nMAR:%08lX MBR:%08lX IR:%08lX", MAR,MBR,IR);
    printf("\nStop:%i IRactive:%i",flags.stopp,flags.IRactive);

    printf("\t\t\tCSZ:%i%i%i",flags.carry,flags.sign,flags.zero);
    printf("\n");
}
/*************************************
 *  Function name: reset
 *  Description: 
 *       This function clears both registers
 *		and flags(called on startup)
 **************************************/
void reset()
{
    clearRegisters();
    clearFlags();
}
/************************************************
 * Function name: fetch
 * Description:  
 *    This function packs data from memory
 *    into the MAR and MBR registers by using
 *    the shift operatior
 ************************************************/
void fetch(void *memory)
{
    int count = 0;
    MAR = PC;

    for (count = 0; count < numOfShifts; count ++)  //shifts 32 bits to the left
    {
		MBR = MBR << 8;
		MBR += ((unsigned char *)memory)[MAR + count];
    }
	PC = PC + sizeof(numOfRegisters);
    IR = MBR;
}
/****************************************************************
* Function name: split
*  Description:
*    This function masks and shifts the IR register and spilts
*    it into 2 16 bit register (IR0,IR1)
****************************************************************/
void split()
{
	IR0 = IR >> IR0shift;
	IR1 = IR & IR1mask;
}
/****************************************************
* Function name: decode
* Description:
*  This function takes a pointer to memory, 
*  checks the IRactive flag and while its not set
*  it fetches an instruction, splits and decodes
*  if the IRactive flag is set then it decodes the 
*  2nd part of the instruction.
****************************************************/
void decode(void *memory)
{
    if (flags.IRactive == 0)
    {
		fetch(memory);	
		split();	
		flags.IRactive = 1;
		execute(IR0, memory);
    }
    else
    {
		split();	
		flags.IRactive = 0;
		execute(IR1, memory);
    }
}
/**************************************
* Function name: go
* Description: 
*   This function checks the stop flag
*   and while its not set it decodes the 
*   instruction
***************************************/
void go(void *memory)
{
    while (flags.stopp != 1)
    {
		decode(memory);
    }
}

/*******************************************
* Function name: dataproc
* Description: 
*   This function performs the specific operation
*   according to the instruction it takes.
********************************************/
void dataproc()
{
	int count = 0;
	switch(operation)
	    {
				case AND:  
					ALU = (registers[RD] & registers[RN]); 
					issign(ALU);
					registers[RD] = ALU;
					break;

				case EOR: 
					ALU = registers[RD] ^ registers[RN]; 
					issign(ALU);
					registers[RD] = ALU;
					break;

				case SUB:
					ALU -= registers[RN];
					issign(ALU);
					registers[RD] = ALU;
					break;

				case SXB: 
					registers[RD] = (signed)registers[RN];
					issign(registers[RD]);
					break;

				case ADD:  
					ALU = registers[RD] + registers[RN];

					issign(ALU);
					flags.carry = iscarry(ALU,registers[RN],flags.carry);
					registers[RD] = ALU;
					break;

				case ADC: 
					ALU = registers[RD] + registers[RN] + flags.carry;
					flags.carry = iscarry(ALU,registers[RN],flags.carry);
					break;

				case LSR: 
					registers[RD] >> registers[RN];
					flags.carry = iscarry(registers[RD],registers[RN],flags.carry);
					issign(registers[RD]);
					break;

				case LSL:
					registers[RD] << registers[RN];
					flags.carry = iscarry(registers[RD],registers[RN],flags.carry);
					issign(registers[RD]);
					break;

				case TST: 
					ALU = registers[RD] & registers[RN];
					issign(ALU);					
					break;

				case TEQ: 
					registers[RD] ^ registers[RN] + 1;
					issign(registers[RD]);
					break;

				case CMP: 
					ALU = registers[RD] + ~ registers[RN] +1;
					issign(ALU);
					flags.carry = (ALU,registers[RN],flags.carry);
					registers[RD] = ALU;
					break;

				case ROR:

					for (count = 0; count < registers[RN] ;count ++)
					{
						flags.carry = registers[RN] & 0x0001;
						if(flags.carry)
						{
							ALU = (registers[RD] >> 1) | 0x80000000;
						}
					}
					issign(ALU);
					flags.carry = iscarry(ALU,registers[RN],flags.carry);
					registers[RD] = ALU;
					break;

				case ORR: 
					ALU = registers[RD] | registers[RN];
					issign(ALU);
					registers[RD] = ALU;
					break;

				case MOV:  
					registers[RD] = registers[RN];
					issign(registers[RD]);
					break;

				case BIC: 
					ALU = registers[RD] & ~registers[RN];
					issign(registers[RD]);
					registers[RD] = ALU;
					break;

				case MVN: 
					registers[RD] = ~registers[RN];
					issign(registers[RD]);
					break;
	    }
}

/****************************************************
* Function name: execute
* Description:
*    This function takes in a 16-bit instruction
*    and a pointer to memory and interprets the 
*    the instruction and performs the corresponding
*    task.
****************************************************/
void execute(unsigned short IR2, void *memory)
{
    unsigned val = (IR2 & valuemask) >> 4;
    unsigned short shift  = (IR2 & maskvalue) >> 13;
    unsigned short opcode;
	unsigned conditions;
    int count = 0;	
    int regcount = 0;
	uint32_t MSmask;
	conditions = ((IR2 & 0xF00) >> 8);
	branch = (IR2 & 0x00FF);
	rel_address = branch;
    byte = (bytemask & IR2) >> 10;
    L  = (loadmask & IR2) >> 11;
    RN = (IR2 & RNmask) >> 4;
    RD = (IR2  & RDmask);
	R = (IR2 & Rmask) >> 8;
    memaddress = (IR2 & memmask);
    linkbit = (IR2 & linkbitmask) >> 12;
    offset = (IR2 & offsetmask);
    operation = (IR2 & operationmask) >> 8;
    H = (IR2 & Hmask) >> 10;
	MSmask = MSBmask;
	reglist = (IR2 & reglistmask);  

    switch(shift)
    {
		case dataprocess: 
				dataproc();
				break;
			case 1:						
				switch(L)
				{
					case 0:
							
							store(memory,MSmask);
						break;
					case 1: 
							load(memory);		
						break;
				}
				break;
			case immediateoperations:
			case immediateoperationss:					
					opcode = (IR2 & opcodemask) >> 12;
					immediateopp(opcode,val);
				break;

			case conditionalbranch: 
					cbranch(conditions);
				break;

			case pushpull:
				printf("push/pull\n");

				switch(L)
				{
				case push:								
					   printf("push\n");
					   //MAR = SP;
					   
					   if(H == 0)  //psh low registers
					   {						   
									printf("push low registers\n");
										
									for(regcount = (bits-1); regcount >= 0; regcount --)
									{								
										if(((reglist >> regcount)&1) == 1)
										{
											MBR = registers[regcount];
											for (count = 0; count < 4; count ++)
											{
												--SP;
												SP = SP & memlocation;
												((uint8_t *)memory)[SP] = (MBR >> bits*count);
											}
										}
									}																	
						   if(R == 1)  //extra push
						   {
							 //  printf("extra push\n");
							    LR=PC;
								MBR = LR;
								for (count = 0; count < 4; count ++)
								{
									--SP;
									SP = SP & memlocation;
									((uint8_t *)memory)[SP] = (MBR >> bits * count);
								}
						   }						   
					   }
						else if(H==1)  //psh high registers
						{
							printf("push high reg\n");
							for(regcounterr = bits; regcounterr >= 0; regcounterr --)  //high registers
							{								
								if(((reglist >> regcounterr)&1) == 1)
								{
									//printf("reaches here");
									MBR = registers[regcounterr + bits];
									for (count = 0; count < 4; count ++)
									{
										--SP;
										SP = SP & memlocation;
										((uint8_t *)memory)[SP] = (MBR >> bits*count);
									}
								}
							}									
						}					   
					break;
				case pull:
					printf("pull\n");	
					
					   if(H == 0)
					   {						   
						    if(R == 1)  //extra pull
							{
								printf("extra pull\n");
								
								MBR = ((uint16_t*)memory)[SP];
								for (count = 0; count < 4; count ++)
								{
									MBR = (MBR << bits);
									MBR += ((uint8_t *)memory)[SP ++];
								}
								PC = MBR;
								flags.IRactive = 0;
							 }
							
						         printf("pull low registers\n");
								 for (counter = 0; counter < bits; counter ++) //pulling in reverse
								 {
									 if(((reglist >> counter)&1) == 1)
									 {
										MBR = ((uint16_t*)memory)[SP];
										for (count = 0; count < 4; count ++)
										{
											MBR = (MBR << bits);
											MBR += ((uint8_t *)memory)[SP ++];
										}
										registers[counter] = MBR;
									 }
								 }								 
					    }
					   else if(H==1)
					   {						   
							printf("pull high registers\n");						
								for (counter = bits; counter < numOfRegisters; counter ++) //pulling in reverse
								{
									 if(((reglist >> counter)&1) == 1)
									 {
										MBR = ((uint16_t*)memory)[SP];
										for (count = 0; count < 4; count ++)
										{
											MBR = MBR << bits;
											MBR += ((uint8_t *)memory)[SP ++];
										}
									 }
									 registers[counter + bits] = MBR;
								 }
						    }       
				
					break;
				}
				break;
			case ubranch:  //unconditional branch
	   
				flags.IRactive =0;
				if(linkbit == 0)
				{
					PC = offset;
				}
				else
				{
					LR = PC;
					PC = offset;
				}
				break;
			case stop:
				flags.stopp = 1;
				break;
			default:
				printf("Not a valid type\n");
				break;
			}
}

/****************************************************
* Function name: issign
* Description: 
*   This function checks the most
*   significant bit in an arithmetic operation
*   if its set then it sets the sign flag and 
*   reset the zero flag, if its not set then
*   if sets the zero flag and resets the sign flag  
*****************************************************/
void issign(uint32_t ALU)
{
    if( (ALU & 0xF0000000) == 0)
    {
		flags.sign = 1;
		flags.zero = 0;
    }
	else
    {
		flags.sign = 0;
		flags.zero = 1;
    }
}
/********************************************
* Function name: cbranch
* Description:
*   This function takes in a condition code
*   and from that determines which condition matches
*   and performs the specific task.
********************************************/
void cbranch(unsigned conditions)
{
	    switch(conditions)
	    {					
				case zset:
					if(flags.zero == 1)
					{
						PC += rel_address;						
						IR = 0;
						flags.IRactive = 0;
					}
					break;

				case zclear:
					if(flags.zero == 0)
					{
						PC += rel_address;					
						IR = 0;
						flags.IRactive = 0;
					}
					break;
				case cset:
					if(flags.carry == 1)
					{
						PC += rel_address;						
						IR = 0;
						flags.IRactive = 0;
					}
					break;
				case cclear:
					if(flags.carry == 0)
					{
						PC += rel_address;						
						IR = 0;
						flags.IRactive = 0;
					}
					break;
				case nset:
					if(flags.sign == 1)
					{
						PC += rel_address;						
						IR = 0;
						flags.IRactive = 0;
					}
				case nclear:
					 if(flags.sign == 0)
					 {
						PC += rel_address;						
						IR = 0;
						flags.IRactive = 0;
					 }
					 break;
				case cset_zclear:
					 if(flags.carry == 1 && flags.zero == 0)
					 {
						PC += rel_address;						
						IR = 0;
						flags.IRactive = 0;
					 }
					 break;
				case cclear_zset:
					 if(flags.carry == 0 && flags.zero == 1)
					 {
						PC += rel_address;
						IR = 0;
						flags.IRactive = 0;
					 }
					 break;

				case ignored:
						PC += rel_address;						
						IR = 0;
						flags.IRactive = 0;
					 break;
	    }
}
/*********************************************
*  Function name: immediateopp
*  Description: 
*    This function takes in an opcode and a 8-bit masked
*    value and from that it matches the cases and performs
*    the appropriate task.    
**********************************************/
void immediateopp(unsigned short opcode,unsigned val)
{
	switch(opcode)
	    {
		case imm_MOV:
		
		    registers[RD] = val;
		    break;
		case imm_CMP:
		  
		    if((registers[RD] - val) == 0)
			flags.zero = 1;
		    else
			flags.zero = 0;
		    break;
		case imm_ADD:
		 
		    ALU = registers[RD] + val;		  
		    issign(ALU);  //if the result is a negative, set the sign flag
		    flags.carry = iscarry(ALU,val, flags.carry);
			registers[RD] = ALU;
		    break;
		case imm_SUB:
		//    printf("SUB\n");
		    ALU = registers[RD] + (~val+1);
		    flags.carry = iscarry(ALU,val,flags.carry);
		    issign(ALU); //if the result is a negative, set the sign flag
		    registers[RD] = ALU;
		    break;
	    }
}
/**********************************************
* Function name: store
*  Description: 
*     This function takes a pointer to memory
*     and a mask an then stores data onto memory
************************************************/
void store(void *memory,uint32_t MSmask)
{
	int count = 0;			
				//printf("store\n");
				if(byte == 0)
				{
					//printf("word\n");
					MAR = registers[RN];
					MBR = registers[RD];
					for(count = 0; count < 4; count ++)
					{
						((unsigned char*)memory)[MAR ++] = (MBR & MSmask) >> (8 * (3- count));
						MSmask = MSmask >> 8;
					}
				}
				else if(byte == 1)
				{
				//	printf("byte\n");							
					MBR = registers[RD];
					((unsigned char*)memory)[MAR] = MBR;
				}
}
/******************************************************
* Function name: load
* Description:
*    This function takes in a pointer to 
*    memory and loads data from memory onto the registers
*********************************************************/
void load(void *memory)
{
			int count = 0;
		//	printf("load\n");
				if(byte == 1)
				{
				//printf("byte\n");
					MAR = registers[RN];
					MBR = ((unsigned char*)memory)[MAR];
					registers[RD] = MBR;
				}
				else if(byte == 0)
				{
					//printf("word\n");
					MAR = registers[RN];
					MBR = registers[RD];
					for(count = 4; count > 0; count --)
					{
						MBR = MBR << 8;
						MBR += ((unsigned char*)memory)[MAR ++];	
					}
				}
				registers[RD] = MBR;			
}


/********************************************************************
  iscarry()- determine if carry is generated by addition: op1+op2+C
  C can only have value of 1 or 0.
 *******************************************************************/
int iscarry(unsigned long op1, unsigned long op2, unsigned c)
{
    if((op2 == MAX32) && (c==1))
    {
		return 1;
    }
    else
		return ((op1 > (MAX32 - op2 -c)) ? 1:0);
}

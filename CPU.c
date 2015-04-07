/**********************************
* Author: Michael Cox
* Creation date: Feburary 27, 2015
***********************************/
#include <stdio.h>
#include <stdint.h>
#define numOfRegisters 15
#define numOfShifts 4
#define MEMSIZE 16384
#define maskvalue 0xE000
#define RDmask 0x000F
#define MAX32 32
#define RNmask 0x00F0
#define valuemask 0x0FF0
#define opcodemask 0x3000
#define loadmask 0x800
#define bytemask 0x400  //word bit mask
#define PC registers[15]
#define SP registers[13]
#define LR registers[14]

typedef struct
{
	int stop; 
	int carry;
	int sign;
	int IRactive;
	int zero;
}FLAGS;

FLAGS flags;
unsigned long MBR;
unsigned long MAR;
unsigned long IR = 0;
unsigned short IR0 = 0;
unsigned short IR1 = 0;
unsigned short RD = 0;
unsigned short RN = 0;
unsigned short L = 0;
unsigned short B = 0;
uint32_t ALU = 0;
unsigned short byte =0;

unsigned long registers[numOfRegisters] = {0};
enum regNames{r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r13,r14,r15}; // register names

/**********************************************
* Function name: clearFlags
* Description: 
*   This function resets the flags to zero
***********************************************/
void clearFlags()
{
	flags.stop  = 0;
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
	
	for (count = 0; count <= numOfRegisters-3; count ++)
	{
		printf("r[%i]:%08lX   ",count, registers[count]);
	}
	printf("\b\b ");
	printf("r[13](SP):%08X            r[14](LR):%08X r[15](PC):%08X",SP,LR,PC);

	printf("\nMAR:%08lX MBR:%08lX IR:%08lX", MAR,MBR,IR);
	printf("\nStop:%i IRactive:%i",flags.stop,flags.IRactive);

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
		PC = PC + sizeof(float);
		IR = MBR;
}
void split()
{
	IR0 = IR >> 16;
	IR1 = IR >> 32;
}

void exec(void *memory)
{
	if (flags.IRactive == 0)
	{
		fetch(memory);
		split();
		//printf("%04x\n",IR0);
		execute(IR0, memory);
		flags.IRactive = 1;
	}
	else
	{
		split();
		//printf("%04x\n",IR1);
		execute(IR1, memory);
		flags.IRactive = 0;
	}
}
void execute(unsigned short IR2, void *memory)
{
	unsigned val = (IR2 & valuemask) >> 4;
	unsigned short shift  = (IR2 & maskvalue) >> 13;
	unsigned short shift2;
	int count = 0;

	byte = (bytemask & IR2) >> 10;
	L  = (loadmask & IR2) >> 11;
	RN = (IR2 & RNmask) >> 4;
	RD = (IR2  & RDmask) ;

	//printf("%04x\n",shift);
	
	switch(shift)
	{
			case 0:
						printf("data processing\n");
						break;
			case 1:
						//printf("load/store\n");
						//printf("%04X\n",L);
						switch(L)
						{
							//printf("%x\n\n\n",L);
							case 0:
									printf("store\n");
									MAR = registers[RN];
									
									if(byte == 0)
									{
										printf("word\n");
										MBR = registers[RD];
										MAR = registers[RN];

										for(count = 0; count <= 1; count ++)
										{
											((unsigned char*)memory)[MAR + count] = MBR;
											MBR >> 8;
										}
									}
									else if(byte == 1)
									{
										printf("byte\n");								
										MBR = registers[RD];
										((unsigned char*)memory)[MAR] = MBR;
									}
									break;
							case 1:
									printf("load\n");
									if(byte == 1)
									{
										printf("byte\n");										
										MAR = registers[RN];
										MBR = ((unsigned char*)memory)[MAR];
										registers[RD] = MBR;
									}
									else if(byte == 0)
									{
										printf("word\n");
										MAR = registers[RN];
										
										for(count = 1; count >= 0; count --)
										{
											MBR += ((unsigned char*)memory)[MAR + count];
											MBR = MBR >>8;
										}
										registers[RD] = MBR;						
									}
								break;
							}
						break;
			case 2:
			case 3:
						//printf("immediate operations\n");
						shift2 = (IR2 & opcodemask) >> 12;						
						switch(shift2)
						{
							case 0:
								//	printf("MOV\n");
									registers[RD] = val;
									break;
							case 1:
									//printf("CMP\n");
									if((registers[RD] - val) == 0)
										flags.zero = 1;
									else
										flags.zero = 0;

									break;
							case 2:
								//	printf("ADD\n");
									registers[RD]  += val;
									flags.carry = iscarry(registers[RD],val,flags.carry);									
									break;
							case 3:
									//printf("SUB\n");
									ALU = registers[RD] + (~val+1);
									flags.carry = iscarry(registers[RD],val,flags.carry);
									registers[RD] = ALU;
									break;
						}
						break;
			case 5:
						printf("push/pull\n");
						break;
			case 6:
						printf("unconditional branch\n");



						break;
			case 7:
						printf("stop\n");
						flags.stop = 1;
						break;
			default:
						printf("Not a valid type\n");
						break;
	}
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

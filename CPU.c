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
#define Rmask 0x100
#define valuemask 0x0FF0
#define opcodemask 0x3000
#define loadmask 0x800
#define bitmask 0x3FFF
#define operationmask 0xF00
#define Hmask 0x400
#define conditionmask 0xF00
#define memmask 0xFF
#define offsetmask 0xFFF
#define linkbitmask 0x1000
#define reglistmask 0xFF00
#define bytemask 0x400 
#define PC registers[15]
#define SP registers[13]
#define LR registers[14]
#define registersize 32
typedef struct
{
	int stop; 
	int carry;
	int sign;
	int IRactive;
	int zero;
}FLAGS;

FLAGS flags;
uint32_t MBR;
uint32_t MAR;
uint32_t IR = 0;
//unsigned long MBR;
//unsigned long MAR;
//unsigned long IR = 0;
unsigned short IR0 = 0;
unsigned short IR1 = 0;
unsigned short RD = 0;
unsigned short RN = 0;
unsigned short L = 0;
unsigned short B = 0;
unsigned memaddress = 0;
uint32_t ALU = 0;
unsigned short byte =0;
unsigned condition = 0;
unsigned linkbit = 0;
unsigned offset = 0;
unsigned operation = 0;
unsigned R = 0;
unsigned H = 0;
uint16_t reglist = 0;
int regcounter = 0;
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

void decode(void *memory)
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
void go(void *memory)
{
	while (flags.stop != 1)
	{
		decode(memory);
	}
}


void execute(unsigned short IR2, void *memory)
{
	unsigned val = (IR2 & valuemask) >> 4;
	unsigned short shift  = (IR2 & maskvalue) >> 13;
	unsigned short shift2;
	int count = 0;
	int regcount = 0;
	byte = (bytemask & IR2) >> 10;
	L  = (loadmask & IR2) >> 11;
	RN = (IR2 & RNmask) >> 4;
	RD = (IR2  & RDmask);
	condition = (IR2 & conditionmask) >>8;
	memaddress = (IR2 & memmask);
	linkbit = (IR2 & linkbitmask) >> 12;
	offset = (IR2 & offsetmask);
	operation = (IR2 & operationmask) >> 12;
	R = (IR2 & Rmask) >> 8;
	H = (IR2 & Hmask) >> 10;
	reglist = (IR2 & reglistmask) >> 8;
	//printf("%04x\n",shift);
	
	switch(shift)
	{
			case 0:
						//printf("data processing\n");
						switch(operation)
						{
							case 0:  //AND
								ALU = (registers[RD] & registers[RN]); 
								issign(ALU);
								registers[RD] = ALU;
								break;

							case 1: //EOR
								ALU = registers[RD] ^ registers[RN];  //EOR
								issign(ALU);
								registers[RD] = ALU;
								break;

							case 2: //SUB
								ALU -= registers[RN];
								issign(ALU);
								registers[RD] = ALU;
								break;

							case 3: //SXB
								registers[RD] = (signed)registers[RN];
								issign(registers[RD]);
								break;

							case 4:  //ADD
								ALU = registers[RD] + registers[RN];
								issign(registers[RD]);
								flags.carry = iscarry(ALU,val,flags.carry);
								registers[RD] = ALU;
								break;

							case 5: //ADC
								registers[RD] += registers[RN] + flags.carry;
								flags.carry = iscarry(ALU,val,flags.carry);
								break;

							case 6: //LSR
								registers[RD] >> registers[RN];
								flags.carry = iscarry(registers[RD],val,flags.carry);
								issign(registers[RD]);
								break;

							case 7: //LSL
								registers[RD] << registers[RN];
								flags.carry = iscarry(registers[RD],val,flags.carry);
								issign(registers[RD]);
								break;

							case 8: //TST
									ALU = registers[RD] & registers[RN];
									issign(ALU);					
								break;

							case 9: //TEQ
								registers[RD] ^ registers[RN] + 1;
								issign(registers[RD]);
								break;

							case 10: //CMP
								ALU = registers[RD] + ~ registers[RN] +1;
								issign(ALU);
								flags.carry = (ALU,val,flags.carry);
								registers[RD] = ALU;
								break;

							case 11: //ROR

								for (count = 0; count < registers[RN] ;count ++)
								{
									flags.carry = registers[RN] & 0x0001;
									if(flags.carry)
									{
										ALU = (registers[RD] >> 1) | 0x80000000;
									}
								}
								issign(ALU);
								flags.carry = iscarry(ALU,val,flags.carry);
								registers[RD] = ALU;
								break;

							case 12: //ORR
									ALU = registers[RD] | registers[RN];
									issign(ALU);
									registers[RD] = ALU;
								break;

							case 13:  //MOV
								registers[RD] = registers[RN];
								issign(registers[RD]);
								break;

							case 14: //BIC
								ALU = 	registers[RD] & ~registers[RN];
								issign(registers[RD]);
								registers[RD] = ALU;
								break;

							case 15: //MVN
								registers[RD] = ~registers[RN];
								issign(registers[RD]);
								break;
							}
						break;
			case 1:
						//printf("load/store\n");
						//printf("%04X\n",L);
						switch(L)
						{
							//printf("%x\n\n\n",L);
							case 0:
									//printf("store\n");
									MAR = registers[RN];
									
									if(byte == 0)
									{
									//	printf("word\n");
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
									//	printf("byte\n");								
										MBR = registers[RD];
										((unsigned char*)memory)[MAR] = MBR;
									}
									break;
							case 1:
								//	printf("load\n");
									if(byte == 1)
									{
									//	printf("byte\n");										
										MAR = registers[RN];
										MBR = ((unsigned char*)memory)[MAR];
										registers[RD] = MBR;
									}
									else if(byte == 0)
									{
									//	printf("word\n");
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
									registers[RD] += val;
									issign(registers[RD]);  //if the result is a negative, set the sign flag
									flags.carry = iscarry(registers[RD],val,flags.carry);
									break;
							case 3:
									//printf("SUB\n");
									ALU = registers[RD] + (~val+1);
									flags.carry = iscarry(ALU,val,flags.carry);
									issign(ALU); //if the result is a negative, set the sign flag
									registers[RD] = ALU;
									break;
						}
						break;

			case 4:
				printf("conditional branch\n");
				switch(condition)
				{
					case 0:
							if(flags.zero = 1)
							{
								PC = memaddress;
							}
						break;

					case 1:
							if(flags.zero = 0)
							{
								PC = memaddress;
							}
						break;

					case 2:
							if(flags.carry = 1)
							{
								PC = memaddress;
							}
						break;
					case 3:
							if(flags.carry = 0)
							{
								PC = memaddress;
							}
							break;
					case 4:
							if(flags.sign= 1)
							{
								PC = memaddress;
							}
						break;
				}
				break;

			case 5:
						printf("push/pull\n");
						
						switch(L)
						{
						case 0:
							//push
							printf("push\n");
							if(H == 0)
							{
								printf("push low registers\n");

								SP = SP - 1;
								SP = (MAR & bitmask);

								for(regcount = 0; regcount < 8; regcount++)
								{
									//if(((reglist>>regcount)&1) == 1)
								//	{
										for (regcount = 0; regcount < registersize; regcount ++)
										{
											((uint8_t *)memory)[SP] = registers[regcounter] >> 1;
										}
									//}
								}
								printf("%04x\n",SP);
								printf("%04x\n",((uint8_t *)memory)[SP]);
							}
							else if(H==1)
							{
								SP = SP - 1;
								SP = (MAR & bitmask);
								printf("push high registers\n");
																
								for(regcount = 0; regcount < 8; regcount++)
								{
									//if(((reglist>>regcount)&1) == 1)
								//	{
										for (regcount = 0; regcount < registersize; regcount ++)
										{
											((uint8_t *)memory)[SP] = registers[(regcounter + 8)] >> 1;
										}
									//}
								}
								//printf("%04x\n",registers[(regcounter+8)]);
							}
							break;

						case 1:
							printf("pull\n");
							
							if(H == 0)
							{
								printf("pull low registers\n");

							}
							else if(H==1)
							{
								printf("pull high registers\n");

							}
							break;
						}
						break;
			case 6:
						printf("unconditional branch\n");
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
			case 7:
						//printf("stop\n");
						flags.stop = 1;
						break;
			default:
						printf("Not a valid type\n");
						break;
	}
}

void issign(unsigned long ALU)
{
	if( (ALU & 0xF000000) == 1)
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

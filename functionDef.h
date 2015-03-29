/***********************************
* File: functionDef.h
* Author: Michael Cox
* Date Created: January 30th, 2015
*
***********************************/

// function definitions
void help();
void memDump(void *memptr, unsigned offset, unsigned length);
int loadFile(void *, unsigned int);
void writeFile(void *);
void input(void *memory, unsigned int max);
void memModify(void *memory);
unsigned getinput(char *format);
void resetRegisters();
void trace(void *);
void clearRegisters();
unsigned maskvalues(unsigned short valueToMask, int start, int finish);
void clearFlags();
void reset();
void split();
void displayRegisters();
void fetch(void *memory);
void exec(void *);
void execute(unsigned short*);
int iscarry(unsigned long op1, unsigned long op2, unsigned c);

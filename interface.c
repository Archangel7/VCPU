/****************************************
*This program is a basic user-interface
* for a virtual CPU 
*
* Creation Date: january 30, 2015
*
*****************************************/
#include "functionDef2.h"
#define memsize 16384

unsigned char MEMORY[memsize]; // 16KBytes for computer memory

int main()
{
	input(&MEMORY,sizeof MEMORY);
}

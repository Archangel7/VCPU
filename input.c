/*********************************
* File: input.h
* Author: Michael Cox
* Date Created: January 30th, 2015
*********************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "functionDef.h"
#define inputMax 10
#define memMax 0x4000
/**************************************
* Function name: input
* Description: 
* This function displays a prompt for 
* the user to enter commands and calls
* the appropriate function depending on
* which command is entered
*
* input/return values:
* inputs - This function takes in the 
* memory buffer and the max size of the buffer
*
* no return value
*
**************************************/
void input(void *memory,unsigned int max)
{
	int size =0;
	int i = 0;
	unsigned offset;
	unsigned length;
	char input[inputMax];
	char input2;
	char userChoice;
	time_t c_time;
	time(&c_time);
	
	printf("Author:  Michael Cox\n");
	printf("Project: Virtual CPU\n");
	printf("Memory:  16KBytes\n");
	printf("Date:    %s\n",ctime(&c_time));
	help();
	reset(); // resets the registers and flags

	while(1)
	{
			printf("prompt>> ");
			fgets(input,sizeof input,stdin);
			
			sscanf(input,"%c",&userChoice);
			fseek(stdin,0,SEEK_SET);
			input[strlen(input)-1]  ='\0';

	    	if(strlen(input) > 1)
			{
				printf("Unknown command\n");
			}
		else
		{
						switch(userChoice)
						{
								case'd':
								case'D':
											printf("Enter an offset to start: ");
backoffset:
											offset = getinput("%X"); //gets the userinput according to the format given
											
											if(offset > memMax)
											{
												printf("Value ranges must be from 0 to 4CE0\nPlease try again: ");
												goto backoffset;
											}
											printf("Enter the length to print out: ");
backlength:
											length = getinput("%X"); //gets the userinput according to the format given

											if((int)length <= 0)
											{
												printf("Value must be more than 0\n");
												printf("Please try again: ");
												goto backlength;
											}
											
											else if(length > memMax)
											{
												printf("Value ranges must be  from 1 to 4CE0\nPlease try again: ");
												goto backlength;
											}
											
											memDump(memory,offset,length);
											break;
								case 'l':
								case 'L':
											size = loadFile(memory, max);
											printf("Number of bytes read in is: %d (hex %X)\n",size,size);
											break;
								case'q':
								case'Q':
											goto EXIT;  //exits both the switch statement and the while loop
								case'g':
								case'G':
											printf("go not implemented yet\n");
											break;
								case'w':
								case'W':
											writeFile(memory);
											break;
								case'm':
								case'M':
											memModify(memory);
											break;
								case't':
								case'T':
											trace(memory);
											break;
								case'z':
								case'Z':
											printf("Rgisters have been reset\n");
											clearRegisters();
											break;
								case'r':
								case'R':
											displayRegisters();
											break;
								case'h':
								case'H':
								case'?': 
											 help();
											 break;
								default:
										  printf("Unknown Command\n");
										  break;
						} // end of switch statement
		} //end of else

	} //end of while loop

EXIT:	return;  //returns to caller

}  // end of input function

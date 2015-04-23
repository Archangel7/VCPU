/*************************************
 * File: functionBody.h
 * Author: Michael Cox
 * Date Created: January 30th, 2015
 *
 *************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include "CPU.c"

#define inputMax 10
#define memsize 16384        //size of computer memory 
#define fileNameLength 90
#define cols 16
#define nameLength 100

/********************************************
 * Function name: memModify
 *
 * Decription:
 *     This function asks the user for a starting
 *     in memory that needs to be modified, displays
 *     the current value at that address and prompts
 *     the user for a new value.
 *
 * Inputs:
 *  This function take in a pointer to the
 *  memory buffer
 ********************************************/
void memModify(void *memory)
{
    unsigned start_addr;
    unsigned newValue=0;
    int count;
    unsigned length = 0x1;
    char input[inputMax];
    char *dot;
    int correct = 0;

    dot = ".";
    printf("Enter the starting address: ");
    start_addr = getinput("%X");  //gets the userinput according to the format given

start:
    memDump(memory,start_addr,length);

    printf("Enter a new value: ");

hexcheck:
    correct = 0;
    fgets(input,sizeof input,stdin);
    input[strlen(input)-1] = '\0';

    if((strcmp(input,dot))==0)
    {
	goto EXIT;
    }
    else
    {
	for (count = 0; count < strlen(input); count ++)
	{
	    if(isxdigit(input[count])==0)
		correct = 1;    // incorrect
	}

	if(correct == 1)
	{
	    printf("Please enter a valid hex digit: ");
	    goto hexcheck;
	}
	else
	{
	    sscanf(input,"%X",&newValue);						
	    ((unsigned char*)memory)[start_addr] = newValue;
	    start_addr++;
	    goto start;
	}
    }
EXIT:
    return;
}
/*****************************************************************
 * Function name: memDump
 *
 * Description:
 *    This function displays the contents of memory as hex bytes
 *    from a given offset to a speicifed number. 
 *
 * Inputs: 
 *     The first paramater takes in a buffer to memory, the second
 *     takes in a specified offset to start from and the last paramater
 *     takes in the specified number of bytes to display.
 * 
 *
 *****************************************************************/
void memDump(void *memptr, unsigned offset, unsigned length)
{
    int count;
    int counter;

    for (count = offset; count < (offset + length); count += cols)
    {
	printf("\n\r");
	printf(" %04x: ",count);    // displays the offset

	if(length== 0x4000)
	{
	    break;
	}

	// didsplays the raw hex values of the file contents
	counter = count;
	while(counter <(count+cols))
	{					 
	    if(counter == (offset + length))
		break;

	    printf("%02x ", (( unsigned char*)memptr)[counter]);
	    counter++;
	}

	printf("\n     ");

	// displays the contents under the hex values
	counter = count;
	while(counter < (count +  cols ))
	{
	    if(counter == (offset + length))
		break;

	    printf("  %c",isprint(((unsigned char*)memptr)[counter]) ? (((unsigned char*)memptr)[counter]): '.');  //if the char is printable then print it else print a '.'

	    counter ++;
	}
	if(counter == length)
	    break;
    }
    printf("\n");
}
/**********************************************
 * Function name: getinput
 * 
 * Description:
 *     This function is a user input function
 *	  using fgets and loops until the user 
 *	  has entered the correct format of the 
 *	  value asked for.
 *
 * Input value:
 *     This function takes in the format of the
 *     value that the caller requires.
 * 
 * return value:
 *      This function returns the value to the 
 *      caller when the correct input is given.
 *
 ************************************************/
unsigned getinput(char *format)
{
    int count = 0;
    int incorrect = 0;
    unsigned retvalue =0;
    char input[nameLength];
check:
    incorrect = 0;
    fgets(input,sizeof input,stdin);
    input[strlen(input)-1] = '\0';

    if(strcmp(input,"") ==0)
    {
	printf("Enter a value: ");
	goto check;
    }

    if(format == "%X")  //deals with hexadecimal values
    {
	for (count = 0; count < strlen(input); count ++)
	{
	    if(isxdigit(input[count])==0)
		incorrect = 1;
	}

	if(incorrect == 1)
	{
	    printf("Please enter a valid hex digit: ");
	    goto check;
	}
	sscanf(input,"%X",&retvalue);
    }

    if(format == "%i")  // deals with integer values
    {
	for (count = 0; count < strlen(input); count ++)
	{
	    if(isdigit(input[count])==0)
		incorrect = 1;
	}
	if(incorrect == 1)
	{
	    printf("Please enter a valid integer: ");
	    goto check;
	}
	sscanf(input,"%i",&retvalue);
    }
    return retvalue;
}
/***************************************************
 *Function name: writeFile
 *Decription: 
 * This function prompts the user for a file name
 * and how much bytes to write to the file
 * then it write that amount of bytes entered
 * from memory to the file.
 *inputs- it takes in the memory buffer
 ****************************************************/
void writeFile(void *memory)
{
    char fileName[nameLength];
    char ifoverride[nameLength];
    char lowercase[nameLength];
    int numOfBytes = 0;
    int count = 0;
    int i;
    FILE *fp;

    printf("Enter the name of a file: ");
    fgets(fileName,sizeof fileName,stdin);
    fileName[strlen(fileName)-1] = '\0';  //null terminate

    if(strlen(fileName) > fileNameLength)
    {
	printf("Filename too long!\n");
    }
    else
    {
	printf("Enter the number of bytes to write(max 16384): ");
	numOfBytes = getinput("%i");   //gets the userinput according to the format given

	if(numOfBytes >= memsize)
	{
	    printf("Max size is 16384\nPlease try again\n");
	}
	else
	{
	    if ( (fp=fopen(fileName,"rb")) != NULL) // file exists
	    {
		printf("File already exists\ndo you wish to override? ");
		fgets(ifoverride,sizeof ifoverride, stdin);

		for(count = 0; count < sizeof ifoverride; count ++)
		{
		    lowercase[count] = tolower(ifoverride[count]);
		}
		if(strcmp(lowercase,"no")==0)
		{
		    fclose(fp);
		    return;
		}
		else
		{
		    printf("File has been overriden\n");
		}
	    }
	}
	fp = fopen(fileName,"wb");
	fwrite(memory,1,numOfBytes,fp);
	fclose(fp);
    }
}
/*******************************************
 *Function name: loadFile
 *
 * Description: 
 * This function prompts the user 
 * for the name of a file, it then reads
 * the file into memory and displays the
 * amount of bytes read in decimal and hex
 * and if the file was truncated
 * if the file is too big it gets truncated.
 *
 * inputs: 
 * This function takes two inputs, the
 * first input points to a buffer for 
 * storage. The second input takes in the 
 * max number of bytes that can be stored
 *
 * return: 
 * This function returns the number 
 * of bytes read in by the file.
 *********************************************/
int loadFile(void *memory, unsigned int max)
{
    char fileName[nameLength] = "";
    char trunc[nameLength]=" ";
    size_t bytesRead = 0;
    size_t fileSize = 0;
    FILE *fp;

    printf("Enter the name of a file: ");
    fgets(fileName,sizeof(fileName),stdin);

    fileName[strlen(fileName)-1] = '\0'; //null terminate

    if(strlen(fileName) > fileNameLength)
    {
	printf("Filename too long!\n");
    }
    else
    {
	fp = fopen(fileName,"rb"); // opens the file in reading mode and bytes mode

	if(fp == NULL)
	{
	    printf("File Not found!\n");
	}
	else
	{
	    fseek(fp,0L,SEEK_END);
	    fileSize = ftell(fp);   // gets the file size

	    if(fileSize > max)  // if the file is too big then truncate
	    {
		sprintf(trunc,"%s was truncated",fileName);
	    }
	    else
	    {
		sprintf(trunc,"%s was not truncated",fileName);
	    }
	    fseek(fp,0, SEEK_SET);
	    fread(memory,max,1,fp);

	    printf("%s\n",trunc);
	    bytesRead = ftell(fp);  // gets the number of bytes read in
	}
    }
    return (int)bytesRead;
}
/****************************************
 * Function name: help
 * Description:
 * This function displays a help menu
 ***************************************/
void help()
{
    printf("d   dump memory\n");
    printf("g   go- run entire program\n");
    printf("l   load a file into memory\n");
    printf("m   memory modify\n");
    printf("q   quit\n");
    printf("r   display registers\n");
    printf("t   trace - execute one instruction\n");
    printf("w   write file\n");
    printf("z   reset all registers to zero\n");
    printf("?, h display list of commands\n");
}
void resetRegisters()
{
    reset();
}
void trace(void *memory)
{
    decode(memory);
    displayRegisters();
}

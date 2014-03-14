#include"Cache.h"
#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<fstream>

void printInputParameters(int x,int y,int z,int a,int b,int n1,int m1,int n2,int m2,char *fileName)
{
	printf("===== Simulator configuration =====\n");
	printf("BLOCKSIZE:			%d\n",x);
	printf("L1_SIZE:			%d\n",y);
	printf("L1_ASSOC:			%d\n",z);
	printf("L1_PREF_N:			%d\n",n1);
	printf("L1_PREF_M:			%d\n",m1);
	printf("L2_SIZE:			%d\n",a);
	printf("L2_ASSOC:			%d\n",b);
	printf("L2_PREF_N:			%d\n",n2);
	printf("L2_PREF_M:			%d\n",m2);
	printf("trace_file:			%s\n",fileName);
}

int main(int argc, char *argv[]) {
int x= atoi(argv[1]);
int y= atoi(argv[2]);
int z= atoi(argv[3]);
int n1=atoi(argv[4]);
int m1=atoi(argv[5]);
int a=atoi(argv[6]);
int b=atoi(argv[7]);
int n2=atoi(argv[8]);
int m2=atoi(argv[9]);
char operation;
unsigned int address;
ifstream fp;
int memoryTraffic=0;


//Initialize Cache

	Cache CacheLevels(x,y,z,a,b,n1,m1,n2,m2);
	
	fp.open(argv[10],ios::in|ios::binary);
	fp>>operation;
	fp >>hex>> address;
	//Level.setCacheParams(x,y,z);
	while(!fp.eof())
	{
		memoryTraffic=CacheLevels.processCommands(address,operation);
		//Level.printBuf();
		fp>>operation;
		fp >>hex>> address;
	}
	printInputParameters(x,y,z,a,b,n1,m1,n2,m2,argv[10]);
	CacheLevels.printCacheBlockData(memoryTraffic);
	fp.close();
	return 0;	
}

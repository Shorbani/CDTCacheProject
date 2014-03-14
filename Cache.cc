#include<iostream>
#include<stdio.h>
#include"Cache.h"
#include<math.h>

using namespace std;


	Cache::Cache(int x,int y,int z,int a,int b,int n1,int m1,int n2,int m2)
	{
	
		blockSize =x;
		sizeL1 = y;
		sizeL2 = a;
		associativityL1=z;
		associativityL2=b;
		NL1=n1;
		ML1=m1;
		NL2=n2;
		ML2=m2;
		if(associativityL1!=0 && blockSize!=0)
		{
			cacheSetsL1 = sizeL1/(blockSize * associativityL1);
			blockDataL1 = new cacheBlock*[cacheSetsL1];
			for(int i=0;i<cacheSetsL1;i++)
			{
				blockDataL1[i] = new cacheBlock[associativityL1];
				for(int j=0;j<associativityL1;j++)
				{
					blockDataL1[i][j].counter=j;
				}
			}
		}
		else
			cacheSetsL1=0;
		if(associativityL2!=0 && blockSize!=0)
		{
			cacheSetsL2 = sizeL2/(blockSize * associativityL2);
			blockDataL2 = new cacheBlock*[cacheSetsL2];

			for(int i=0;i<cacheSetsL2;i++)
			{
				blockDataL2[i] = new cacheBlock[associativityL2];
				for(int j=0;j<associativityL2;j++)
				{
					blockDataL2[i][j].counter=j;
				}
			}
		}
			
		else 
			cacheSetsL2=0;
		
		
		L1writeBack=0,L1Read=0,L1ReadMisses=0,L1Write=0,L1WriteMisses=0;
		L2writeBack=0,L2Read=0,L2ReadMisses=0,L2Write=0,L2WriteMisses=0,L2ReadPrefetch=0,L2ReadMissesPrefetch=0;
		L2PrefetchCount=0,L1ReadPrefetch=0;

		//L1 Stream Buffer
		if(NL1!=0 && ML1!=0)
		{
			streamBufL1 = new cacheBlock*[NL1];
			for(int i=0;i<NL1;i++)
			{
				streamBufL1[i] = new cacheBlock[ML1];
				for(int j=0;j<ML1;j++)
				{
					streamBufL1[i][j].counter=i;	 //for LRU
				}
			}
		}

		//L2 Stream Buffer
		if(NL2!=0 && ML2!=0)
		{
			streamBufL2 = new cacheBlock*[NL2];
			for(int i=0;i<NL2;i++)
			{
				streamBufL2[i] = new cacheBlock[ML2];
				for(int j=0;j<ML2;j++)
				{
					streamBufL2[i][j].counter=i;	 //for LRU
				}
			}
		}

		indexBitsL1=log(cacheSetsL1)/log(2);
		blockOffsetBitsL1=log(blockSize)/log(2);
		indexBitsL2=log(cacheSetsL2)/log(2);
		blockOffsetBitsL2=log(blockSize)/log(2);

	}

//Stream Buffer Params

	void Cache::setStreamBufParams(int counter,bool validityBit,int index,int pos,unsigned int address,cacheBlock **streamBlock)
	{
		streamBlock[index][pos].counter=0;
		streamBlock[index][pos].validityBit =validityBit;
		streamBlock[index][pos].address =address;
	}


	void Cache::setCacheBlockParamsL1(int counter,bool validityBit,unsigned int tagData,int index,int pos,unsigned int address)
	{

	 
		blockDataL1[index][pos].counter=0;
		blockDataL1[index][pos].validityBit =validityBit;
		blockDataL1[index][pos].tag =tagData;
		blockDataL1[index][pos].address =address;
	}

	void Cache::setCacheBlockParamsL2(int counter,bool validityBit,unsigned int tagData,int index,int pos,unsigned int address)
	{	 
		blockDataL2[index][pos].counter=0;
		blockDataL2[index][pos].validityBit =validityBit;
		blockDataL2[index][pos].tag =tagData;
		blockDataL2[index][pos].address =address;
	}

	//Sort MRU to LRU

	void sortMRUToLRU(cacheBlock dataBlock[],int n)
	{
		int i,j,key;
		cacheBlock keyData;
		for (j = 1; j < n; j++) 
		{ 
			key = dataBlock[j].counter;	
			keyData=dataBlock[j];
			i = j-1;
			while (i >= 0 && dataBlock[i].counter > key) { 
			dataBlock[i+1] = dataBlock[i]; 
			i--; 
			}
			dataBlock[i+1] = keyData; 
		}
	}

//For Stream Buffer
	void Cache ::sortMRUToLRUStream(cacheBlock **dataBlock,int n,int m)
	{
		int i,j,key;
		cacheBlock *keyData=new cacheBlock[m];
		for (j = 1; j < n; j++) 
		{ 
			key = dataBlock[j][0].counter;
			for(int k=0;k<m;k++)
			{	
				keyData[k]=dataBlock[j][k];
			}
			i = j-1;
			while (i >= 0 && dataBlock[i][0].counter > key) { 
				for(int k=0;k<m;k++)
				{
					dataBlock[i+1][k] = dataBlock[i][k];
				} 
				i--; 
			}
			for(int k=0;k<m;k++)
			{
				dataBlock[i+1][k] = keyData[k]; 
			}
		}
	}

	void Cache::printCacheBlockData(int memoryTraffic)
	{
		printf("===== L1 contents =====\n");
		for(int i =0;i<cacheSetsL1;i++)
		{
			printf("Set\t%d:",i);
			for(int j =0;j<associativityL1;j++)
			{
				sortMRUToLRU(blockDataL1[i],associativityL1);
				printf("\t%x",blockDataL1[i][j].tag);
				if(blockDataL1[i][j].dirtyBit==1)
					printf(" D");
			}
			printf("\n");
		}
		if(NL1!=0 && ML1!=0)
		{
			printf("===== L1-SB contents =====\n");
			printBuf(NL1,ML1,streamBufL1);
		}

		if(associativityL2 !=0 && sizeL2!=0)
		{
			printf("===== L2 contents =====\n");
			for(int i =0;i<cacheSetsL2;i++)
			{
				printf("Set\t%d:",i);
				for(int j =0;j<associativityL2;j++)
				{
					sortMRUToLRU(blockDataL2[i],associativityL2);
					printf("\t%x",blockDataL2[i][j].tag);
					if(blockDataL2[i][j].dirtyBit==1)
						printf(" D");
				}
				printf("\n");
			}
			if(NL2!=0 && ML2!=0)
			{
				printf("===== L2-SB contents =====\n");
				printBuf(NL2,ML2,streamBufL2);
			}
		}
	printRawSimulation(memoryTraffic);

	}
	void Cache :: printBuf(int n,int m, cacheBlock ** streamBuf)
	{
		sortMRUToLRUStream(streamBuf,n,m);
		for(int i=0;i<n;i++)
		{
			for(int j=0;j<m;j++)
			{
				printf("\t%x",streamBuf[i][j].address);
			}
			printf("\n");
		}
	}
	void Cache::printRawSimulation(int memoryTraffic)
	{
		int L1Misses = L1ReadMisses+L1WriteMisses;
		int L1References = L1Read+L1Write;
		double L1MissRate,L2MissRate;
		if(L1References!=0)
			 L1MissRate=(double)L1Misses /(double)L1References;
		else 
			 L1MissRate=0;
		if(L2Read!=0)
			 L2MissRate=(double)L2ReadMisses/(double)L2Read;
		else
			 L2MissRate=0;
		printf("===== Simulation results (raw) =====\n");
		printf("a. number of L1 reads:\t%d\n",L1Read);
		printf("b. number of L1 read misses:\t%d\n",L1ReadMisses);
		printf("c. number of L1 writes:\t%d\n",L1Write);
		printf("d. number of L1 write misses:\t%d\n",L1WriteMisses);
		if(L1References!=0)
			printf("e. L1 miss rate:\t%f\n",L1MissRate);
		else
			printf("e. L1 miss rate:\t%d\n",(int)L1MissRate);
		printf("f. number of L1 writebacks:\t%d\n",L1writeBack);
		printf("g. number of L1 prefetches:\t%d\n",L1ReadPrefetch);
		printf("h. number of L2 reads that did not originate from L1 prefetches:\t%d\n",L2Read);
		printf("i. number of L2 read misses that did not originate from L1 prefetches:\t%d\n",L2ReadMisses);
		printf("j. number of L2 reads that originated from L1 prefetches:\t%d\n",L2ReadPrefetch);
		printf("k. number of L2 read misses that originated from L1 prefetches:\t%d\n",L2ReadMissesPrefetch);
		printf("l. number of L2 writes:\t%d\n",L2Write);
		printf("m. number of L2 write misses:\t%d\n",L2WriteMisses);
		if(L2Read!=0)
			printf("n. L2 miss rate:\t%f\n",L2MissRate);
		else
			printf("n. L2 miss rate:\t%d\n",(int)L2MissRate);
		printf("o. number of L2 writebacks:\t%d\n",L2writeBack);
		printf("p. number of L2 prefetches:\t%d\n",L2PrefetchCount);
		printf("q. total memory traffic:\t%d\n",memoryTraffic);
		
	}

	int Cache:: findIndex(unsigned int address,int cacheSets,int blockOffsetBits)
	{
		int index=address>>blockOffsetBits;
		index=index&(cacheSets-1);
		return index;
	}

	int Cache::findTag(unsigned int address,int cacheSets,int blockOffsetBits,int indexBits)
	{
		int index=address>>blockOffsetBits;
		index=index&(cacheSets-1);
		int shiftVal= indexBits+blockOffsetBits;
		int tagData=address>>shiftVal;
		return tagData;
	}

	int Cache :: findLruBlock(int index,int associativity,cacheBlock **blockData)
	{
		int lruBlock=-1,maxCounter=-1;
		for(int i=0;i<associativity;i++)
		{
			if(blockData[index][i].counter>maxCounter)
			{
				maxCounter=blockData[index][i].counter;
				lruBlock=i;
			}
		}
		return lruBlock;
	}

	void Cache :: updateLruBlock(int index,int lruBlock,int associativity,cacheBlock **blockData)
	{
		for(int i=0 ;i<associativity;i++)
		{
			if(blockData[index][i].counter<blockData[index][lruBlock].counter)
			{
				blockData[index][i].counter++;
			}
		}	
	}
	//Update LRU blocks of Stream Buffer
	void Cache :: updateStreamBufLRU(int n,int m,cacheBlock **streamBuf,int hitStream)
	{
		for(int i=0;i<n;i++)
		{
			if(streamBuf[i][0].counter<streamBuf[hitStream][0].counter)
			{
				for(int j=0;j<m;j++)
				{
					streamBuf[i][j].counter++;
				}
			}
		}
	}
	
	//Find LRU Block in the stream buffer
	int Cache :: findLRUStreamblock(int n,cacheBlock **streamBlock)
	{
		int streamLruBlock=-1,maxCounter=-1;
		for(int i=0;i<n;i++)
		{
			if(streamBlock[i][0].counter>maxCounter)
			{
				maxCounter=streamBlock[i][0].counter;
				streamLruBlock=i;
			}
		}
		//printf("Stream = %d\n",streamLruBlock);
		return streamLruBlock;
	}

	//Shift Block and get new Data
	void Cache :: shiftStreamBufData(int n,int streamIndex,int m,cacheBlock **streamBuf,int blockOffsetBits,bool cacheLevel)
	{
		
		unsigned int newAddress =streamBuf[streamIndex][m-1].address>>blockOffsetBits;
		newAddress=newAddress+1;
		newAddress=newAddress<<blockOffsetBits;
		for(int i=0;i<m-1;i++)
		{
			streamBuf[streamIndex][i]=streamBuf[streamIndex][i+1];
		}
		if(cacheLevel)
		{	
			L1ReadPrefetch++;
			if(associativityL2!=0)					
				readWriteL2(newAddress,'r',true); //Read the next address into the stream buffer
		}
		else
			L2PrefetchCount++;
		updateStreamBufLRU(n,m,streamBuf,streamIndex);
		for(int j=0;j<m;j++)
		{
			streamBuf[streamIndex][j].counter=0;
		}
		setStreamBufParams(0,1,streamIndex,m-1,newAddress,streamBuf); //Get the address to the last location
	}
		
	void Cache :: fetchDataToStream(int n,int m,cacheBlock **streamBuf,unsigned int address,int blockOffsetBits,bool cacheLevel)
	{
		unsigned int newAddress;
		//Data not there in stream buffer. fetch the next address + m blocks
		int streamNo=findLRUStreamblock(n,streamBuf);
		updateStreamBufLRU(n,m,streamBuf,streamNo);
		for(int i=1;i<=m;i++)
		{
			newAddress =address>>blockOffsetBits;
			newAddress=newAddress+1*i;
			newAddress=newAddress<<blockOffsetBits;
			if(cacheLevel)
			{	
				L1ReadPrefetch++;
				if(associativityL2!=0)					
					readWriteL2(newAddress,'r',true); //Read the next address into the stream buffer
			}
			else
				L2PrefetchCount++;
			setStreamBufParams(0,1,streamNo,i-1,newAddress,streamBuf); //Get the address to the last location
		}
	}
	int Cache :: checkStreamBuf(int n,int m,cacheBlock **streamBuf,unsigned int address,int blockOffsetBits)
	{
		address=(address>>blockOffsetBits)<<blockOffsetBits;
		for(int i=0;i<n;i++)
		{
			if(streamBuf[i][0].address==address && streamBuf[i][0].validityBit==1)
			{
				return i;
			}			
		}
		return -1;		
	}

	//Invalidate Stream Buffer due to write Back
	void Cache :: invalidateStreamBuf(unsigned int address,int n,int m,cacheBlock **streamBuf,int blockOffsetBits )
	{
		address=(address>>blockOffsetBits)<<blockOffsetBits;
		for(int i=0;i<n;i++)
		{
			for(int j=0;j<m;j++)
			{
				if(streamBuf[i][j].address==address)
				{
					streamBuf[i][j].validityBit=0;
				}
			}
		}
	}

	void Cache :: readOperation(int index,unsigned int tag,unsigned int address)
	{
		int flag=0;
		int lruBlock=-1;
		int retVal=-1;
			
		for(int i=0 ;i<associativityL1;i++)
		{
			if(blockDataL1[index][i].tag==tag && blockDataL1[index][i].validityBit==1)
			{
				L1Read++;
				flag=1;
				lruBlock=i;
				break;
			}
		}
		if(flag==0)
		{
			L1Read++;
			lruBlock=findLruBlock(index,associativityL1,blockDataL1);
			if(blockDataL1[index][lruBlock].dirtyBit==1)
			{
				L1writeBack++;
				if(associativityL2!=0)
					readWriteL2(blockDataL1[index][lruBlock].address,'w',false);
				blockDataL1[index][lruBlock].dirtyBit=0;
				//Invalidate Stream Buffer
				invalidateStreamBuf(blockDataL1[index][lruBlock].address,NL1,ML1,streamBufL1,blockOffsetBitsL1);
			}
			//Before issuing read to L2 check in stream Buffer if address there
			retVal=checkStreamBuf(NL1,ML1,streamBufL1,address,blockOffsetBitsL1);
			//printf("retval = %d\n ",retVal);
			if(retVal==-1)
			{
				L1ReadMisses++;
				if(associativityL2!=0)
					readWriteL2(address,'r',false);
				
			}
		}
		updateLruBlock(index,lruBlock,associativityL1,blockDataL1);
		setCacheBlockParamsL1(0,1,tag,index,lruBlock,address); //write data in any case. In case of hit it will just overwrite the data
		if(retVal!=-1)
		{
			
			shiftStreamBufData(NL1,retVal,ML1,streamBufL1,blockOffsetBitsL1,true);
			//printf("ADD = %x",address+ML1*16);
		}
		else if(retVal==-1 && flag==0)
		{
			fetchDataToStream(NL1,ML1,streamBufL1,address,blockOffsetBitsL1,true);
		}
	}


	void Cache :: writeOperation(int index,unsigned int tag,unsigned int address)
	{
		int flag=0;
		int lruBlock=-1;
		int retVal=-1;
		
		for(int i=0 ;i<associativityL1;i++)
		{
			if(blockDataL1[index][i].tag==tag && blockDataL1[index][i].validityBit==1)
			{
				//printf("ADD = %x\n",address);	
				L1Write++;
				flag=1;
				lruBlock=i;
				blockDataL1[index][i].dirtyBit=1;
				break;
			}
		}
		if(flag==0)
		{
			L1Write++;
			lruBlock=findLruBlock(index,associativityL1,blockDataL1);
			if(blockDataL1[index][lruBlock].dirtyBit==1)
			{
				L1writeBack++;
				if(associativityL2!=0)
					readWriteL2(blockDataL1[index][lruBlock].address,'w',false);
				//Invalidate Stream Buffer
				invalidateStreamBuf(blockDataL1[index][lruBlock].address,NL1,ML1,streamBufL1,blockOffsetBitsL1);
			}
			//Before issuing read to L2 check in stream Buffer if tag there
			
			 retVal=checkStreamBuf(NL1,ML1,streamBufL1,address,blockOffsetBitsL1);
			//printf("retval = %d\n ",retVal);
			if(retVal==-1)
			{
				L1WriteMisses++;
				if(associativityL2!=0)			
					readWriteL2(address,'r',false);
			}
			
			blockDataL1[index][lruBlock].dirtyBit=1;
		}
		updateLruBlock(index,lruBlock,associativityL1,blockDataL1);
		//write data in any case. In case of hit it will just overwrite the 						    
                //data.Set dirty bit to 1
		setCacheBlockParamsL1(0,1,tag,index,lruBlock,address); 
		if(retVal!=-1)
		{		
				//Shift Block and get one more data
				shiftStreamBufData(NL1,retVal,ML1,streamBufL1,blockOffsetBitsL1,true);
		}
		else if(retVal==-1 && flag==0)
		{
			fetchDataToStream(NL1,ML1,streamBufL1,address,blockOffsetBitsL1,true);
		}
		
	}
	int Cache::processCommands(unsigned int address,char operation)
	{
		int index = findIndex(address,cacheSetsL1,blockOffsetBitsL1);
		unsigned int tag = findTag(address,cacheSetsL1,blockOffsetBitsL1,indexBitsL1);


		if(operation == 'r')
		{
			readOperation(index,tag,address);
		}
		if(operation == 'w')
		{
			writeOperation(index,tag,address);
		}
		int memorytraffic;
		if(associativityL2==0)
			memorytraffic = L1ReadMisses+L1WriteMisses+L1writeBack+L1ReadPrefetch;
		else
			memorytraffic = L2ReadMisses+L2WriteMisses+L2writeBack+L2ReadMissesPrefetch+L2PrefetchCount;
		return memorytraffic;
		
	}

	void Cache :: readOperationL2(int index,unsigned int tag,unsigned int address,bool prefetchReq)
	{
		int flag=0;
		int lruBlock=-1;
		int retVal =-1;

		for(int i=0 ;i<associativityL2;i++)
		{
			if(blockDataL2[index][i].tag==tag && blockDataL2[index][i].validityBit==1)
			{
				
				flag=1;
				lruBlock=i;
				if(prefetchReq)
				{
					L2ReadPrefetch++;
				}
				else
				 	L2Read++;	
				break;
			}
		}
		if(flag==0)
		{
			
			if(!prefetchReq)
			{
				L2Read++;
				
			}
			else
			{
				L2ReadPrefetch++;
				
			}

			lruBlock=findLruBlock(index,associativityL2,blockDataL2);
			if(blockDataL2[index][lruBlock].dirtyBit==1)
			{
				L2writeBack++;
				invalidateStreamBuf(blockDataL2[index][lruBlock].address,NL2,ML2,streamBufL2,blockOffsetBitsL2);
				blockDataL2[index][lruBlock].dirtyBit=0;
			}
			 retVal=checkStreamBuf(NL2,ML2,streamBufL2,address,blockOffsetBitsL2);
			if(retVal==-1)
			{
				
				if(!prefetchReq)
				{
					L2ReadMisses++;
									
				}
				else
				{
					L2ReadMissesPrefetch++;
				}
				
			}
		}
		updateLruBlock(index,lruBlock,associativityL2,blockDataL2);
		setCacheBlockParamsL2(0,1,tag,index,lruBlock,address); //write data in any case. In case of hit it will just overwrite the data
		if(retVal!=-1)
		{		
				//Shift Block and get one more data
				shiftStreamBufData(NL2,retVal,ML2,streamBufL2,blockOffsetBitsL2,false);
		}
		else if(retVal==-1 && flag==0)
		{
			fetchDataToStream(NL2,ML2,streamBufL2,address,blockOffsetBitsL2,false);
		}

	}
	void Cache :: writeOperationL2(int index,unsigned int tag,unsigned int address)
	{
		int flag=0;
		int lruBlock=-1;
		int retVal=-1;

				
		for(int i=0 ;i<associativityL2;i++)
		{
			if(blockDataL2[index][i].tag==tag && blockDataL2[index][i].validityBit==1)
			{
				L2Write++;
				flag=1;
				lruBlock=i;
				blockDataL2[index][i].dirtyBit=1;
				break;
			}
		}
		if(flag==0)
		{
			L2Write++;
			
			lruBlock=findLruBlock(index,associativityL2,blockDataL2);
			if(blockDataL2[index][lruBlock].dirtyBit==1)
			{
				L2writeBack++;
				invalidateStreamBuf(blockDataL2[index][lruBlock].address,NL2,ML2,streamBufL2,blockOffsetBitsL2);
			}
			blockDataL2[index][lruBlock].dirtyBit=1;
			retVal=checkStreamBuf(NL2,ML2,streamBufL2,address,blockOffsetBitsL2);
			if(retVal==-1)
			{
				L2WriteMisses++;
			}
		}
		updateLruBlock(index,lruBlock,associativityL2,blockDataL2);
		//write data in any case. In case of hit it will just overwrite the 						    
                //data.Set dirty bit to 1
		setCacheBlockParamsL2(0,1,tag,index,lruBlock,address); 
		if(retVal!=-1)
		{		
			//Shift Block and get one more data
			shiftStreamBufData(NL2,retVal,ML2,streamBufL2,blockOffsetBitsL2,false);
		}
		else if(retVal==-1 && flag==0)
		{
			fetchDataToStream(NL2,ML2,streamBufL2,address,blockOffsetBitsL2,false);
		}
	}
	void Cache :: readWriteL2(unsigned int address,char operation,bool prefetchReq)
	{
		int index = findIndex(address,cacheSetsL2,blockOffsetBitsL2);
		int tag = findTag(address,cacheSetsL2,blockOffsetBitsL2,indexBitsL2);
		//printf("tag = %x\n",tag);
		if(operation=='w')
		{
			writeOperationL2(index,tag,address);
		}
		else if(operation == 'r')
		{
			readOperationL2(index,tag,address,prefetchReq);			
		}

	}

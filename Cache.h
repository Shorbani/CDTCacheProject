

using namespace std;

class cacheBlock{

public:	
	int counter;
	bool validityBit;
	bool dirtyBit;
	unsigned int tag;
	unsigned int address;
	
	
 
	cacheBlock()
	{
		counter=-1;
		validityBit=0;
		dirtyBit=0;
		tag=-1;
		address=0;
	}
};

class Cache{
	
 
	int blockSize;
	int sizeL1,sizeL2; //Size of L1,L2
	int associativityL1,associativityL2;
	int cacheSetsL1,cacheSetsL2;
	cacheBlock **blockDataL1,**blockDataL2;
	unsigned int address;
	int L1writeBack,L1Read,L1ReadMisses,L1Write,L1WriteMisses;
	int L2writeBack,L2Read,L2ReadMisses,L2Write,L2WriteMisses;
	int L2PrefetchCount;
	int L2ReadPrefetch,L2ReadMissesPrefetch,L1ReadPrefetch;
	//for Stream buffers
	int NL1,NL2,ML1,ML2;
	cacheBlock **streamBufL1,**streamBufL2;
	int blockOffsetBitsL1,blockOffsetBitsL2;
	int indexBitsL1,indexBitsL2;
		
	
public:	
	// Add Stream Buffer data here.

	Cache(int,int,int,int,int,int,int,int,int);
	void setCacheBlockParamsL1(int,bool,unsigned int,int,int,unsigned int);
	void setCacheBlockParamsL2(int,bool,unsigned int,int,int,unsigned int);
	void printCacheBlockData(int);
	int processCommands(unsigned int,char);
	int findIndex(unsigned int,int,int);
	int findTag(unsigned int,int,int,int);
	int findLruBlock(int,int,cacheBlock**);
	void updateLruBlock(int ,int,int,cacheBlock** );
	void readOperation(int ,unsigned int,unsigned int);
	void writeOperation(int ,unsigned int,unsigned int);
	void readOperationL2(int ,unsigned int,unsigned int,bool);
	void writeOperationL2(int ,unsigned int,unsigned int );
	void readWriteL2(unsigned int,char,bool);
	void printRawSimulation(int);
	void invalidateStreamBuf(unsigned int ,int ,int ,cacheBlock **,int);
	int checkStreamBuf(int ,int ,cacheBlock **,unsigned int,int);
	void shiftStreamBufData(int,int ,int ,cacheBlock **,int,bool);
	int findLRUStreamblock(int ,cacheBlock **);
	void updateStreamBufLRU(int,int,cacheBlock **,int);
	void setStreamBufParams(int ,bool ,int ,int ,unsigned int ,cacheBlock **);
	void printBuf(int,int,cacheBlock **);
	void fetchDataToStream(int ,int ,cacheBlock **,unsigned int,int,bool );	
	void sortMRUToLRUStream(cacheBlock **,int,int);
	  };

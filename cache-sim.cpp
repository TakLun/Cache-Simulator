#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sstream>
#include <math.h>
#include <sys/time.h>

using namespace std;


//Store data about the cache inputs
typedef struct{

	bool valid;
	int tag;
	int longevity;

}CacheData;

typedef struct{

	bool valid;
	bool newEntry;
	int longevity;
	int frequency;
	int tag;	

}NewReplace;

//Store data about the address
typedef struct{

	string strAddress;
	int numAddress;
	int result;

}Instruction;

//Convert address from string to an int address
int translateAddress(string strAddress){

	unsigned int address;
	char parseAddress[8];
	stringstream ss;
	
	size_t length = strAddress.copy(parseAddress, 8,10);
	parseAddress[length] = '\0';
	
	//convert number from string to hex to int
	ss << hex << parseAddress;
	ss >> address;
	
	return address;
	
}

Instruction parseAddress(string line){

	Instruction info;
	
	char address[18];
	int result;
	int hexAddress;
	char loadstore[line.length()-19];
	
	//Store result of memory instruction
	line.copy(loadstore, 1,0);
	size_t length = line.copy(address, 19,2);
	address[length] = '\0';
	
	//If result of instruction is Load, then return 1
	//else return 0
	if(loadstore[0] == 'L'){
		result = 1;
	}else{
		result = 0;
	}
	
	//translate address
	hexAddress = translateAddress(address);
	
	//store information to data structs
	info.strAddress = address;
	info.numAddress = hexAddress;
	info.result = result;	
	

	return info;
}

void directMappedCache(ifstream *inputfile, ofstream *outputfile, int cacheSize, int lineSize){

	string line;
	int percent;
	
	int shift;
	int numOfLines;
	int index;
	int tag;
	CacheData *cache;
	
	int correct = 0;
	int instructions = 0;
	
	Instruction instInfo;
	
	//Calculate the number of sets needed for the cache
	numOfLines = cacheSize/lineSize;	
	cache = new CacheData[numOfLines];

	
	while(!inputfile->eof()){
		getline(*inputfile, line);
		if(line.empty())
			break;
	
		//translate lines from file
		instInfo = parseAddress(line);
		
				
		//get index
		index = instInfo.numAddress >> 5;
		index = (index % numOfLines);
		
		//get tag
		shift = (log(numOfLines)/log(2))+5;
		tag = instInfo.numAddress >> shift;
		
		
		//checks for validity
		if(cache[index].valid && (cache[index].tag == tag)){
			correct++;
			//cout << "Cache Hit!!!" << endl << endl;
		}else{
			cache[index].tag = tag;
			cache[index].valid = true;
			
			/*
			cout << "Cache Miss!!" << endl;
			cout << "index: " << index << endl;
			cout << "New Tag: " << tag << endl << endl;
			*/
		}
		
		//count the number of instructions in file
		instructions++;
		
	}
	
	//calculate cache hit rate
	percent = ((float)correct/(float)instructions)*100+.5;
	//print out results to output file
	*outputfile << percent << " ";
		
}

void setAssociative(ifstream *inputfile, ofstream *outputfile, int cacheSize, 
					int lineSize, int associ){

	string line;
	int percent;
	
	int shift;
	int numOfLines;
	int index;
	int tag;
	CacheData **cache;
	
	int correct = 0;
	int instructions = 0;
	
	Instruction instInfo;
	
	//Calculate number of sets in cache
	numOfLines = (cacheSize/lineSize)/associ;	
	
	//Create sets and ways
	cache = new CacheData*[associ];
	for(int i=0; i<associ; i++){
	
		cache[i] = new CacheData[numOfLines];
	
	}

	while(!inputfile->eof()){
		getline(*inputfile, line);
		if(line.empty())
			break;
	
		//translate line from file
		instInfo = parseAddress(line);
		
				
		//get index
		index = instInfo.numAddress >> 5;
		index = (index % numOfLines);
		
		//get tag
		shift = (log(numOfLines)/log(2))+5;
		tag = instInfo.numAddress >> shift;
		
		//track the number of valid entries in cache
		int validAddr = 0;
		
		//track if the entry is a cache hit
		bool hit = false;
		//Insert entry into cache
		for(int way=0; way<associ; way++){
		
			if(cache[way][index].valid){
				
				validAddr++;
				
				//determine if cache hit			
				if(cache[way][index].tag == tag){
				
					//Increase the time each entry has spent in cache
					for(int i=0;i<associ;i++){
						cache[i][index].longevity++;	
					}
				
					correct++;
					cache[way][index].longevity = 0;
					
					hit = true;
				
					break;
				}		
			}
		}
		
		//find an empty entry in cache if cache miss and insert
		for(int way=0; way<associ; way++){
			if(validAddr < associ && !hit && !cache[way][index].valid){
		
				//Increase the time each entry has spent in cache
				for(int i=0;i<associ;i++){
					cache[i][index].longevity++;	
				}
			
				cache[way][index].tag = tag;
				cache[way][index].valid = true;
				cache[way][index].longevity = 0;			
			
				break;

			}
		}
		
		//If cache is full and has a cache miss use LRU to replace an entry
		if(validAddr == associ && !hit){
		
			//Find oldest entry in cache
			CacheData oldest = cache[0][index];
			int replacementWay = 0;
			for(int i=1;i<associ;i++){
			
				if((cache[i][index].longevity) > oldest.longevity){
					oldest = cache[i][index];
					replacementWay = i;
				}
				cache[i][index].longevity++;	
			}
			
			//Replace oldest entry in cache
			cache[replacementWay][index].tag = tag;
			cache[replacementWay][index].valid = true;
			cache[replacementWay][index].longevity = 0;		
			
		}
		
		//count number of instructions in file
		instructions++;
	}
	cout << "Set Associ Correct: " << correct << endl;
	//calculate cache hit rate
	percent = ((float)correct/(float)instructions)*100+.5;
	//print out results to output file
	*outputfile << percent << " ";
	
}

void fullyAssociative(ifstream *inputfile, ofstream *outputfile, int cacheSize, 
					int lineSize, bool LRU){
					
					
	string line;
	int percent;
	
	int shift;
	int numOfWays;
	int tag;
	CacheData *cache;
	
	int correct = 0;
	int instructions = 0;
	
	Instruction instInfo;
	
	numOfWays = cacheSize/lineSize;	
	
	//Create ways
	cache = new CacheData[numOfWays];

	while(!inputfile->eof()){
		getline(*inputfile, line);
		if(line.empty())
			break;
	
		//translate line from file
		instInfo = parseAddress(line);
		
				
		//get tag
		shift = 5;
		tag = instInfo.numAddress >> shift;
		
		//track the number of valid entries in cache
		int validAddr = 0;
		
		//track if the entry is a cache hit
		bool hit = false;
		//Insert entry into cache
		for(int way=0; way<numOfWays; way++){
		
			if(cache[way].valid){
				
				validAddr++;
				
				//determine if cache hit			
				if(cache[way].tag == tag){
				
					//Increase the time each entry has spent in cache
					for(int i=0;i<numOfWays;i++){
						cache[i].longevity++;	
					}
				
					correct++;
					cache[way].longevity = 0;
					
					hit = true;
				
					break;
				}		
			}
		}
		
		//find an empty entry in cache if cache miss and insert
		for(int way=0; way<numOfWays; way++){
			if(validAddr < numOfWays && !hit && !cache[way].valid){
		
				//Increase the time each entry has spent in cache
				for(int i=0;i<numOfWays;i++){
					cache[i].longevity++;	
				}
			
				cache[way].tag = tag;
				cache[way].valid = true;
				cache[way].longevity = 0;			
			
				break;

			}
		}
		
		
			//If cache is full and has a cache miss use LRU to replace an entry
		if(validAddr == numOfWays && !hit){
			if(LRU){
				//Find oldest entry in cache
				CacheData oldest = cache[0];
				int replacementWay = 0;
				for(int i=1;i<numOfWays;i++){
			
					if((cache[i].longevity) > oldest.longevity){
						oldest = cache[i];
						replacementWay = i;
					}
					cache[i].longevity++;	
				}
			
				//Replace oldest entry in cache
				cache[replacementWay].tag = tag;
				cache[replacementWay].valid = true;
				cache[replacementWay].longevity = 0;		
			
			}else{
	
				//get random way to replace
				struct timeval time; 
			     gettimeofday(&time,NULL);
				srand((time.tv_sec) + (time.tv_usec));
				int replacementWay;
				replacementWay = rand() % numOfWays;
				
				for(int i=1;i<numOfWays;i++){
					cache[i].longevity++;	
				}
				
				//Replace oldest entry in cache
				cache[replacementWay].tag = tag;
				cache[replacementWay].valid = true;
				cache[replacementWay].longevity = 0;
			
			}			
		}
		
			
			
		
		
		//count number of instructions in file
		instructions++;
	}

	//calculate cache hit rate
	percent = ((float)correct/(float)instructions)*100+.5;				
	//print out results to output file
	*outputfile << percent << " ";
	
}

void setAssociativeWrite(ifstream *inputfile, ofstream *outputfile, int cacheSize, 
					int lineSize, int associ){

	string line;
	int percent;
	
	int shift;
	int numOfLines;
	int index;
	int tag;
	CacheData **cache;
	
	int correct = 0;
	int instructions = 0;
	
	Instruction instInfo;
	
	//Calculate number of sets in cache
	numOfLines = (cacheSize/lineSize)/associ;	
	
	//Create sets and ways
	cache = new CacheData*[associ];
	for(int i=0; i<associ; i++){
	
		cache[i] = new CacheData[numOfLines];
	
	}

	while(!inputfile->eof()){
		getline(*inputfile, line);
		if(line.empty())
			break;
	
		//translate line from file
		instInfo = parseAddress(line);
		
				
		//get index
		index = instInfo.numAddress >> 5;
		index = (index % numOfLines);
		
		//get tag
		shift = (log(numOfLines)/log(2))+5;
		tag = instInfo.numAddress >> shift;
		
		//track the number of valid entries in cache
		int validAddr = 0;
		
		//track if the entry is a cache hit
		bool hit = false;
		//Insert entry into cache
		for(int way=0; way<associ; way++){
		
			if(cache[way][index].valid){
				
				validAddr++;
				
				//determine if cache hit			
				if(cache[way][index].tag == tag){
				
					//Increase the time each entry has spent in cache
					for(int i=0;i<associ;i++){
						cache[i][index].longevity++;	
					}
				
					correct++;
					cache[way][index].longevity = 0;
					
					hit = true;
				
					break;
				}		
			}
		}
		
		//find an empty entry in cache if cache miss and insert
		//Do not do anything when a store is encountered
		for(int way=0; way<associ; way++){
			if(validAddr < associ && !hit && !cache[way][index].valid && instInfo.result == 1){
		
				//Increase the time each entry has spent in cache
				for(int i=0;i<associ;i++){
					cache[i][index].longevity++;	
				}
			
				cache[way][index].tag = tag;
				cache[way][index].valid = true;
				cache[way][index].longevity = 0;			
			
				break;

			}
		}
		
		//If cache is full and has a cache miss use LRU to replace an entry
		//Do not do anything if a store is encountered
		if(validAddr == associ && !hit && instInfo.result == 1){
		
			//Find oldest entry in cache
			CacheData oldest = cache[0][index];
			int replacementWay = 0;
			for(int i=1;i<associ;i++){
			
				if((cache[i][index].longevity) > oldest.longevity){
					oldest = cache[i][index];
					replacementWay = i;
				}
				cache[i][index].longevity++;	
			}
			
			//Replace oldest entry in cache
			cache[replacementWay][index].tag = tag;
			cache[replacementWay][index].valid = true;
			cache[replacementWay][index].longevity = 0;		
			
		}
		
		//count number of instructions in file
		instructions++;
	}

	//calculate cache hit rate
	percent = ((float)correct/(float)instructions)*100+.5;
	
	//print out results to output file
	*outputfile << percent << " ";
	
}

void quickSort(NewReplace arr[], int left, int right){
	  int i = left, j = right;
	  NewReplace tmp;
	  NewReplace pivot = arr[(left + right) / 2];

	  /* partition */
	  while (i <= j) {
		while (arr[i].longevity < pivot.longevity)
			i++;
		while (arr[j].longevity > pivot.longevity)
			j--;
		if (i <= j) {
			tmp = arr[i];
			arr[i] = arr[j];
			arr[j] = tmp;
			i++;
			j--;
	    }
	}
	/* recursion */
	if (left < j)
		quickSort(arr, left, j);
	if (i < right)
		quickSort(arr, i, right);
}

void improvedReplacementMechanism(ifstream *inputfile, ofstream *outputfile, 
			int cacheSize, int lineSize, int associ){

	string line;
	int percent;
	
	int shift;
	int numOfLines;
	int index;
	int tag;
	NewReplace **cache;
	
	int correct = 0;
	int instructions = 0;
	
	Instruction instInfo;
	
	//Calculate number of sets in cache
	numOfLines = (cacheSize/lineSize)/associ;	
	
	//Create sets and ways
	cache = new NewReplace*[associ];
	for(int i=0; i<associ; i++){
	
		cache[i] = new NewReplace[numOfLines];
	
	}
	
	NewReplace set[associ];
	NewReplace timeRecord[associ/2];
	NewReplace leastFreq;	
		
	while(!inputfile->eof()){
		getline(*inputfile, line);
		if(line.empty())
			break;

		//translate line from file
		instInfo = parseAddress(line);
		
		//get index
		index = instInfo.numAddress >> 5;
		index = (index % numOfLines);
		
		//get tag
		shift = (log(numOfLines)/log(2))+5;
		tag = instInfo.numAddress >> shift;
		
		//track the number of valid entries in cache
		int validAddr = 0;

		//track if the entry is a cache hit
		bool hit = false;
		
		//Check for Cache Hits
		for(int way=0; way<associ; way++){
		
			if(cache[way][index].valid){
				
				validAddr++;
				
				//determine if cache hit			
				if(cache[way][index].tag == tag){
				
					//Increase the time each entry has spent in cache
					//Evict all entries from new section
					for(int i=0;i<associ;i++){
						cache[i][index].longevity++;
						cache[i][index].newEntry = 0;
					}
				
					correct++;
					
					/*Increase the access freqency of the entry 
					* Renew the longivity of the entry
					* Insert entry to new section
					*/
					cache[way][index].frequency++;
					cache[way][index].longevity = 0;
					cache[way][index].newEntry = 1;
					
					hit = true;
				
					break;
				}		
			}
		}
	
		//Check for emptry spaces 
		for(int way=0; way<associ; way++){
			if(validAddr < associ && !hit && !cache[way][index].valid){
		
				//Increase the time each entry has spent in cache
				for(int i=0;i<associ;i++){
					cache[i][index].longevity++;	
					cache[i][index].newEntry = 0;
				}
			
				cache[way][index].tag = tag;
				cache[way][index].valid = true;
				cache[way][index].frequency = 0;
				cache[way][index].longevity = 0;			
				cache[way][index].newEntry = 1;				
			
				break;

			}
		}
		
		
		//If cache is full and has a cache miss use LRU to replace an entry
		if(validAddr == associ && !hit){
		
			int replacementWay = 0;
			int oldSectionIndex = 0;
			for(int i=0;i<associ;i++){
				set[i] = cache[i][index];
					
				cache[i][index].longevity++;
				cache[i][index].newEntry = 0;
			}
			
			quickSort(set, 0, associ-1);
			
			
			/*
			for(int i=0;i<associ/2;i++){
				cout << set[i+(associ/2)].longevity << " ";
			}
			cout << endl;
			*/
			
			for(int i=0;i<associ/2;i++){
				timeRecord[i] = set[i+(associ/2)];
			
			}

			leastFreq = timeRecord[0];
	
			for(int i=1;i<associ/2;i++){
			
				if(timeRecord[i].frequency < leastFreq.frequency){
					leastFreq = timeRecord[i];
				}
				
			}
			
			for(int i=0;i<associ;i++){
				if(leastFreq.tag == cache[i][index].tag){
					replacementWay = i;
					break;
				}				
			}
			
			//Replace oldest entry in cache
			cache[replacementWay][index].tag = tag;
			cache[replacementWay][index].valid = true;
			cache[replacementWay][index].frequency = 0;
			cache[replacementWay][index].longevity = 0;
			cache[replacementWay][index].newEntry = 1;	
			
		}
		
		instructions++;
	}
	
	cout << "New Replace Correct: " << correct << endl;
	percent = ((float)correct/(float)instructions)*100+.5;
	
	//print out results to output file
	*outputfile << percent << " ";

}

int main(int argc, char *argv[]){

	ofstream outputfile;
	ifstream inputfile;
	

	//Check is there is sufficient number of parameters
	if(argc != 3){
		printf("Error: Incorrect number of inputs\n");
		exit(0);
	}
	
	//Check if input file exists	
	inputfile.open(argv[1]);
	if(!inputfile.is_open()){
		perror("Error\nUnable to open input file");
		exit(0);	
	}

	//Create output file to print results
	outputfile.open(argv[2]);
	if(!outputfile.is_open()){
		perror("Error\nUnable to open output file");
		exit(0);	
	}
	
	
	
	//Perform Direct Mapped Cache
	for(int cacheSize=1024;cacheSize<=16384;cacheSize*=4){
	
		directMappedCache(&inputfile, &outputfile, cacheSize, 32);
		inputfile.clear();
		inputfile.seekg(0, std::ios_base::beg);
	
	}
	
	directMappedCache(&inputfile, &outputfile, 32768, 32);
	inputfile.clear();
	inputfile.seekg(0, std::ios_base::beg);
	
	
	
	outputfile << endl;
	
	
	
	//Peform Set Associative Cache
	for(int associativity = 2; associativity <= 16; associativity *= 2){
		setAssociative(&inputfile, &outputfile, 16384, 32, associativity);
		inputfile.clear();
		inputfile.seekg(0, std::ios_base::beg);
	}
	
	
	outputfile << endl;

	
	//Perform Fully Associative Cache
	fullyAssociative(&inputfile, &outputfile, 16384, 32, 1);
	inputfile.clear();
	inputfile.seekg(0, std::ios_base::beg);
	
	outputfile << endl;
	
	fullyAssociative(&inputfile, &outputfile, 16384, 32, 0);
	inputfile.clear();
	inputfile.seekg(0, std::ios_base::beg);

	outputfile << endl;	
	
	
	// Set­Associative  Cache  with  no  Allocation  on  a  Write  Miss
	for(int associativity = 2; associativity <= 16; associativity *= 2){
		setAssociativeWrite(&inputfile, &outputfile, 16384, 32, associativity);
		inputfile.clear();
		inputfile.seekg(0, std::ios_base::beg);
	}
	
	outputfile << endl;

	
	//Peform New Cache Replacement 
	for(int associativity = 2; associativity <= 16; associativity *= 2){
		improvedReplacementMechanism(&inputfile, &outputfile, 16384, 32, associativity);
		//improvedReplacementMechanism(&inputfile, &outputfile, 16384, 32, 16);
		inputfile.clear();
		inputfile.seekg(0, std::ios_base::beg);
	}
	
	
	outputfile << endl;
	
	outputfile.close();
	inputfile.close();
	
	return 0;
}

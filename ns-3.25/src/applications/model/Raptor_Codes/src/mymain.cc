
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <ctime>
#include <iterator>
#include <string>
#include <vector>
#include <math.h>

#include "libraptor/RandNum_Generator.h"
#include "libraptor/Degree_Generator.h"
#include "libraptor/rfc5053_config.h"
#include "libraptor/Partition.h"
#include "libraptor/Array_Data_Types.h"
#include "libraptor/Inter_Symbol_Generator.h"
#include "libraptor/R10_Decoder.h"
#include "libraptor/LT_Encoding.h"


#include "libraptor/Utility.h"


#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>

#include "libraptor/storage_adaptors.h"


#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/undirected_graph.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/graph_utility.hpp>


using namespace std;
using namespace boost::numeric::ublas;

int SYMBOL_LEN = 0;
int SYMBOL_SIZE = 0;
int overhead = 0;
int P = 1000;
int Kmin = 1024;
int Al = 4;
int Kmax = 8192;
int Gmax = 10;

//.............Function for Encoding...........//

class Array_Data_Symbol Raptor_Encoding(class Array_Data_Symbol testing_symbol, int SYMBOL_SIZE, int SYMBOL_LEN, int overhead ){

	
	class LT_Encoding encoder(&testing_symbol); //goes to line 27, LT_Encoding.cc
	
	
	class Array_Data_Symbol D(testing_symbol.K, SYMBOL_LEN);
	
    std::vector<uint32_t> ESI;
	
    		for (int i = 0; i < testing_symbol.K + overhead; ++i)
    		{
    			ESI.push_back(i);
    			D.ESIs.push_back(i);
    		}
	
	//final encoded sequence
	D.symbol = encoder.LTEnc_Generate(ESI); //D.symbol contains ecoded symbol
	
		
	//printf("\nSize of D after encoding %d ",(int)D.symbol.size());

	return D;

}


//..........Function for Decoding..............//

class Array_Data_Symbol Raptor_Decoding(int K, int SYMBOL_LEN, int overhead)	{
	
		
		//Assigning class S for receiving the encoded stream from file
			
			class Array_Data_Symbol S(K, SYMBOL_LEN); 

		 	std::vector<uint32_t> ESI;
	
    				for (int i = 0; i < S.K + overhead; ++i)
    				{
    					ESI.push_back(i);
    					S.ESIs.push_back(i);
    				}


			ifstream::pos_type size;
			char * memblock;

			//........Reading the encoded stream......
			

			ifstream file ("somefile.bin", ios::in|ios::binary|ios::ate);
  
  				if (file.is_open())
  				{
    				size = file.tellg();
    				memblock = new char [size];
    				file.seekg (0, ios::beg);
    				file.read (memblock, size);

    				//cout << "\nthe complete file content is in memory";

    				/*for (int l=0; l<size; l++){
    				cout << " memblock[] is = " << (unsigned int)memblock[l] << " index was l = " << l << endl;
    				}*/
   					file.close();
				}
 


		 //Copying the encoded stream to S 
    	 int l = 0;
        	//printf("\nReading Encoded Data Stream from file....");
   			for (int i = 0; i < S.K + overhead; ++i)
    		{
    			for (int j = 0; j < SYMBOL_LEN; ++j)
    			{
    		 		S.symbol[i].s[j] = memblock[l];
    		 		l++;
    			}
       
     		}

		 delete[] memblock;

	// drop some symbols for testing
    int loss = 5;
   
    S.symbol.erase(S.symbol.begin() + 93);
    S.ESIs.erase(S.ESIs.begin() + 93);

    S.symbol.erase(S.symbol.begin() + 13);
    S.ESIs.erase(S.ESIs.begin() + 13);

    S.symbol.erase(S.symbol.begin() + 11);
    S.ESIs.erase(S.ESIs.begin() + 11);

    S.symbol.erase(S.symbol.begin() + 15);
    S.ESIs.erase(S.ESIs.begin() + 15);

    S.symbol.erase(S.symbol.begin() + 98);
    S.ESIs.erase(S.ESIs.begin() + 98);
  

   
		/*printf("\nBefore Decoding, the Data is- ");

   			for (int i = 0; i < S.K + overhead; ++i)
    		{
    			for (int j = 0; j < SYMBOL_LEN; ++j)
    			{
    		 	printf("%c", S.symbol[i].s[j] );
    			}
       
     		}*/


	//.......Decoding begin..........//
	class R10_Decoder decoder(S.K + overhead - loss, SYMBOL_LEN);

	//class Array_Data_Symbol C = decoder.Get_Inter_Symbols(D, K); //intermediate symbols
	
	//decoder gets back the source symbols	
   	class Array_Data_Symbol source = decoder.Get_Source_Symbol(S, S.K + overhead - loss);
	
	
	return source;
}






//.... Main begins....//



int main(int argc, char* argv[])
{
	std::cout << "Raptor Codes Testing !" << std::endl;	

 //cout << "number of arguments: " << argc << endl;
	
	if(argc == 3)
	{
		
			
		if( (strcmp(argv[1],"raptor")==0) && (strcmp(argv[2],"enc")==0) )
		{
			
			//Parameters derivation
			
					int src_sym, sym_length, append; 
    
					std::cout << "Enter the number of source symbols" << endl ;
					std::cin >> src_sym;

					std::cout << "Enter the overhead you want to add for the data" << endl ;
					std::cin >> append;

	
    				overhead = append;
    				SYMBOL_SIZE = src_sym;

    			ifstream myReadFile;
 					myReadFile.open("file_100.txt");
 	 	
 					stringstream strStream;
					strStream << myReadFile.rdbuf();//read the file
					string str = strStream.str();//str holds the content of the file


					float len = (float) str.size()/SYMBOL_SIZE;
 	
					SYMBOL_LEN = ceil(len);
    				cout<< "K = " << SYMBOL_SIZE << endl;
   					cout<< "T = " << SYMBOL_LEN  << endl;
   					cout << "overhead = " << overhead << endl;

   					//Calculate the K values by equations defined in rfc 5053
   					int P1 = (P*Kmin/ str.size());
   					int P2 = P/Al;

   					int G1 = ceil(P1);
   					int G = std::min(std::min(G1, P2), Gmax);

   					cout << "G = " << G << endl;

   					int T = floor((P/(Al*G))*Al);
   					cout << "Derived T = "<< T << endl;

   					int Kt = ceil( str.size()/ T);

   					cout << "Kt is =" << Kt << endl;

   					cout << "Z = " << ceil(Kt/Kmax);

   					//Write derived values in a file 
   					ofstream stream1;
					stream1.open("Param_calculated.txt", ios::binary);

					stream1 << G << endl;
					stream1 << T << endl;
					stream1 << Kt << endl;
					stream1 <<  "Z " << ceil(Kt/Kmax) << endl;
					stream1.close();

   					// Writing K, T, overhead values given by user into file Param.txt
   					ofstream stream;
					stream.open("Param.txt", ios::binary);

					stream << SYMBOL_SIZE << endl;
					stream << SYMBOL_LEN << endl;
					stream << overhead << endl;
					stream.close();


					
    					class Array_Data_Symbol testing_symbol(SYMBOL_SIZE, SYMBOL_LEN);
    
   						 int f = 0;
   							 for (int j = 0; j < SYMBOL_SIZE; ++j)
   							 {
    	  						for(int k = 0; k < SYMBOL_LEN; ++k)
    							{
    								if(f<str.length())
    								{
    								testing_symbol.symbol[j].s[k] = str[f];
    								f++;
    								}
    							}
							}
				
				int data_len = testing_symbol.K*SYMBOL_LEN;
			    char array2[data_len];
				int count1 = 0;
				
				cout << "Testing symbols are:" << endl;
				for (int i = 0; i < testing_symbol.K; ++i)
    			{
    			 for (int j = 0; j < SYMBOL_LEN; ++j)
    			 {
    		 		//printf("%c", testing_symbol.symbol[i].s[j]);
    		 		array2[count1] = testing_symbol.symbol[i].s[j];
    		 		count1++;
    			 }
       
     			}

     			std::ofstream original_out("original_data.bin", std::ios::binary);
							original_out.write(array2, sizeof(array2));
							original_out.close();

     		int start_s = clock();

					class Array_Data_Symbol D = Raptor_Encoding(testing_symbol, SYMBOL_SIZE, SYMBOL_LEN, overhead);

			int stop_s = clock();
			
	
					int len1 = D.symbol.size()*SYMBOL_LEN;
	
					char array[len1];
					int g = 0;	

					printf("\nEncoded Data - ");
   							for (int i = 0; i < D.symbol.size(); ++i)
    						{
    							for (int j = 0; j < SYMBOL_LEN; ++j)
    							{
    		 						//printf("%c", D.symbol[i].s[j] );
    		 						array[g] = D.symbol[i].s[j];
    		 						g++;
    							}
       
     						}

     						//Writing the data to file somfile.bin
     						std::ofstream out("somefile.bin", std::ios::binary);
							out.write(array, sizeof(array));
							out.close();

							cout << endl;
							cout << "Encoding time: " << (stop_s-start_s)/double(CLOCKS_PER_SEC)*1000 << endl;

			
			}	

			else if( (strcmp(argv[1],"raptor")==0) && (strcmp(argv[2],"dec")==0) )
			{
					
					int data[3];
					
					ifstream myfile;
  					 myfile.open("Param.txt");

   					 for(int i = 0; i < 3; i++)
        					myfile >> data[i];

   					 myfile.close();

   					 SYMBOL_SIZE = data[0];
   					 SYMBOL_LEN = data[1];
   					 overhead = data[2];

   					 cout<< "K = " << SYMBOL_SIZE << endl;
   					cout<< "T = " << SYMBOL_LEN  << endl;
   					cout << "Overhead = " <<  overhead << endl;

   					cout << "Overhead bytes " << SYMBOL_LEN*overhead << endl;

   				int start_d = clock();

					class Array_Data_Symbol R = Raptor_Decoding(SYMBOL_SIZE, SYMBOL_LEN, overhead );
    
    			int stop_d=clock();
    				//std::cout << "\ndecode successfully!" << std::endl;
    				

    				int length = R.K*SYMBOL_LEN;
     				char array1[length];
     				int count = 0;
  	 				printf("\n\nDecoded Data - ");

   					for (int i = 0; i < R.symbol.size(); ++i)
    				{
    					for (int j = 0; j < SYMBOL_LEN; ++j)
    					{
    		 				//printf("%c", R.symbol[i].s[j] );
    		 				array1[count] = R.symbol[i].s[j];
    		 				count++;
    					}
       
     				}


     						std::ofstream decode_out("decoded_data.bin", std::ios::binary);
							decode_out.write(array1, sizeof(array1));
							decode_out.close();




     				cout << "Decoding time: " << (stop_d-start_d)/double(CLOCKS_PER_SEC)*1000 << endl;
   					 std::cout <<  "\ntotal symbols: " << R.K <<"  redundancy rate: " <<  (float)overhead / (float)SYMBOL_SIZE <<std::endl;

			}
	}
	
				

	else{
			cout << "Enter valid arguments" << endl;
			exit(0);
	}

}
	
//commands to run
// ./main raptor enc --- for encoding
// ./main raptor dec --- for decoding




#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <ctime>

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
//using namespace boost::numeric::ublas::;
using namespace boost::numeric::ublas;




//   enum { A, B, C, D, E, F, N };
//   const char* name = "ABCDEF";

// typedef boost::adjacency_matrix<boost::undirectedS> UGraph;
//  UGraph ug(N);




int main(int argc, char const *argv[])
{
	std::cout << "Raptor Codes Testing !" << std::endl;	

	const int SYMBOL_SIZE = 1024;
    const int SYMBOL_LEN = 1000;
    
    const int overhead = 25;


	class Array_Data_Symbol testing_symbol(SYMBOL_SIZE, SYMBOL_LEN);
	
	std::cout << "Testing symbols size " << testing_symbol.K  << std::endl;

	for (int i = 0; i < testing_symbol.K; ++i)
	{
		for (int j = 0; j < SYMBOL_LEN; ++j){
		// if (i%2)
		// {
		// 	a.symbol[i].s[0] = 1;
		// }
		if (i%3)
		{
			testing_symbol.symbol[i].s[j] = 1;
		}
		}		
	}

	std::cout << "Testing symbols size " << testing_symbol.symbol.size() << std::endl;


    class LT_Encoding encoder(&testing_symbol);
	
	

   	class Array_Data_Symbol D(testing_symbol.K, SYMBOL_LEN);
   	
   	std::cout << "D size is " << D.K <<  std::endl;

    std::vector<uint32_t> ESI;

    for (int i = 0; i < testing_symbol.K + overhead; ++i)
    {
    	ESI.push_back(i);
    	D.ESIs.push_back(i);
    	//std::cout << i << std::endl;
    }

	std::cout << "D size after assigning ESI is " << D.ESIs.size() <<  std::endl;
	
    // for (int i = 0; i < ESI.size(); ++i)
    // {
    // 	printf("ESI: %d\n", ESI[i]);
    // }

	//printf("ESI: %d\n", ESI.size());
	int start_e=clock();
    D.symbol = encoder.LTEnc_Generate(ESI);
    int stop_e=clock();
    std::cout << "time for encoding: " << (stop_e-start_e)/double(CLOCKS_PER_SEC)*1000 << std::endl;
    
    std::cout << "Encoded symbols size " << D.symbol.size() << std::endl;


	class R10_Decoder decoder(testing_symbol.K, SYMBOL_LEN);

	// class Array_Data_Symbol C = decoder.Get_Inter_Symbols(D, testing_symbol.K);


 //    std::cout << "testing_symbol: " << std::endl;
 //    for (int i = 0; i < testing_symbol.K; ++i)
 //    {
 //        printf("%d ", testing_symbol.symbol[i].s[0]);
 //    }
 //    std::cout << std::endl;

 //     printf("D: \n");

 //    for (int i = 0; i < D.symbol.size(); ++i)
 //    {
 //        printf("%d ", D.symbol[i].s[0]);
 //    }
 //    printf("\n");

	// printf("C: \n");

	// for (int i = 0; i < C.symbol.size(); ++i)
	// {
	// 	printf("%d ", C.symbol[i].s[0]);
	// }
	// printf("\n");


    // decoder.Inter_Symbols_Decoding(C);

    // drop some symbols for testing
     int loss = 1;
    D.symbol.erase(D.symbol.begin() + 3);
    D.ESIs.erase(D.ESIs.begin() + 3);

	std::cout << "Encoding symbol size K and symbol size() " << D.K << "\t" << D.symbol.size() << std::endl;

   int start_s=clock();
	class Array_Data_Symbol source = decoder.Get_Source_Symbol(D, testing_symbol.K + overhead - loss);
	int stop_s=clock();
    std::cout << "time for decoding: " << (stop_s-start_s)/double(CLOCKS_PER_SEC)*1000 << std::endl;
   


    // printf("Source Symbol: \n");

    // for (int i = 0; i < source.symbol.size(); ++i)
    // {
    //     printf("%d ", source.symbol[i].s[0]);
    // }
    // printf("\n");


    // std::cout << "Testing Symbol: " << std::endl;
    // for (int i = 0; i < testing_symbol.K; ++i)
    // {
    //     printf("%d ", testing_symbol.symbol[i].s[0]);
    // }
    // std::cout << std::endl;


   for (int i = 0; i < source.symbol.size(); ++i)
   {
   		for (int j = 0; j < SYMBOL_LEN; ++j){
        if (source.symbol[i].s[j] ^ testing_symbol.symbol[i].s[j] != 0)
        {

             std::cout << "decode fail!" << std::endl;
             return -1;

        }
        }

   }


    std::cout << "decode successfully!" << std::endl;
    std::cout <<  "total symbols: " << source.K <<"  redundancy rate: " <<  (float)overhead / (float)SYMBOL_SIZE <<std::endl;


	return 0;
}



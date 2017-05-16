
SET (GCC_COVERAGE_COMPILE_FLAGS "-fPIC")

SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS})
SET(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS})

SET (LIB_SRCS RandNum_Generator.cc Degree_Generator.cc Triple_Generator.cc Partition.cc Inter_Symbol_Generator.cc Array_Data_Types.cc LT_Encoding.cc Utility.cc R10_Decoder.cc)



ADD_LIBRARY (raptor ${LIB_SRCS})



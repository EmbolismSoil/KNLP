cmake_minimum_required(VERSION 3.5)
project(KLP)

set(CMAKE_CXX_STANDARD 11)
add_subdirectory(Test)
add_executable(KLP main.cpp Segmenter/DAGSegmenter.h LanguageModel/BigramLanguaModel.h Segmenter/SentenceDAG.h LanguageModel/UnigramLanguageModel.h Segmenter/HMMSegmenter.h)

cmake_minimum_required(VERSION 3.5)
project(KLP)

set(CMAKE_CXX_STANDARD 11)
add_executable(KLP main.cpp Segmenter/BigramDAGSegmenter.h LanguageModel/BigramLanguaModel.h Utils/DAG.h)

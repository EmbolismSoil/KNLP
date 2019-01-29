#include <iostream>
#include "LanguageModel/BigramLanguaModel.h"
#include "LanguageModel/UnigramLanguageModel.h"
#include "Segmenter/SentenceDAG.h"
#include "Segmenter/DAGSegmenter.h"
#include "Segmenter/HMMSegmenter.h"
#include "Segmenter/MixedSegmenter.h"

int main(int argc, const char* argv[]) {
    if (argc != 4){
        std::cout << "Usage: " << argv[0] << " <corpus> <sentence>" << std::endl;
        return -1;
    }

    std::vector<std::wstring> words;
    std::wstring_convert<std::codecvt_utf8<wchar_t>> codec;


    HMMSegmenter hmm{};
    hmm.fit(argv[1]);
    hmm.segment(codec.from_bytes(argv[3]), words);

    std::cout << "hmm segment: ";
    for (auto const& w: words){
        std::cout << codec.to_bytes(w) << " ";
    }
    std::cout << std::endl;

    auto unigram_lm = UnigramLanguageModel{};
    unigram_lm.fit(argv[2]);
    auto dag_segmenter = DAGSegmenter<UnigramLanguageModel>(unigram_lm);
    
    words.clear();
    dag_segmenter.segment(codec.from_bytes(argv[3]), words);

     std::cout << "dag segment: ";
    for (auto const& w: words){
        std::cout << codec.to_bytes(w) << " ";
    }
    std::cout << std::endl;   

    MixedSegmenter<DAGSegmenter<UnigramLanguageModel>, HMMSegmenter> mixed_segmenter(dag_segmenter, hmm);
    words.clear();

    mixed_segmenter.segment(codec.from_bytes(argv[3]), words);
    std::cout << "mixed segment: ";
    for (auto const& w: words){
        std::cout << codec.to_bytes(w) << " ";
    }
    std::cout << std::endl;      
    return 0;
}

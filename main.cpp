#include <iostream>
#include "LanguageModel/BigramLanguaModel.h"
#include "LanguageModel/UnigramLanguageModel.h"
#include "Segmenter/SentenceDAG.h"
#include "Segmenter/DAGSegmenter.h"
#include "Segmenter/HMMSegmenter.h"

int main(int argc, const char* argv[]) {
    if (argc != 3){
        std::cout << "Usage: " << argv[0] << " <corpus> <sentence>" << std::endl;
        return -1;
    }

    std::vector<std::wstring> words;
    std::wstring_convert<std::codecvt_utf8<wchar_t>> codec;


    HMMSegmenter hmm{};
    hmm.fit(argv[1]);
    hmm.segment(codec.from_bytes(argv[2]), words);


    for (auto const& w: words){
        std::cout << codec.to_bytes(w) << " ";
    }
    std::cout << std::endl;

    return 0;
}
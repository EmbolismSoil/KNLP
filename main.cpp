#include <iostream>
#include "LanguageModel/BigramLanguaModel.h"
#include "Utils/DAG.h"
#include "Segmenter/BigramDAGSegmenter.h"

int main() {
    auto model = std::make_shared<BigrameLanguageModel<std::wstring>>();
    model->fit("/home/lee/github/KLP/corpus.txt");

    BigramDAGSegmenter segmenter(model);
    for(int i = 0; i < 100000; ++i){
        std::wstring_convert<std::codecvt_utf8<wchar_t>> codec;
        std::vector<std::wstring> words;
        segmenter.segment(L"研究生物学", words);
        for (auto const& w: words){
            std::cout << codec.to_bytes(w) << " ";
        }
        std::cout << std::endl;
    }

    return 0;

}
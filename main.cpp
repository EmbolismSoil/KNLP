#include <iostream>
#include "LanguageModel/BigramLanguaModel.h"
#include "LanguageModel/UnigramLanguageModel.h"
#include "Segmenter/SentenceDAG.h"
#include "Segmenter/DAGSegmenter.h"

int main(int argc, const char* argv[]) {
    if (argc != 3){
        std::cout << "Usage: " << argv[0] << " <corpus> <sentence>" << std::endl;
        return -1;
    }

    auto model = std::make_shared<UnigramLanguageModel>();
    model->fit(argv[1]);

    DAGSegmenter<UnigramLanguageModel> segmenter(model);
    std::wstring_convert<std::codecvt_utf8<wchar_t>> codec;

    std::vector<std::wstring> words;
    segmenter.segment(codec.from_bytes(argv[2]), words);

    for (auto const& w: words){
        std::cout << codec.to_bytes(w) << " ";
    }
    std::cout << std::endl;

    return 0;
}
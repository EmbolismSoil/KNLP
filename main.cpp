#include <iostream>
#include "LanguageModel/BigramLanguaModel.h"
#include "LanguageModel/UnigramLanguageModel.h"
#include "Segmenter/SentenceDAG.h"
#include "Segmenter/DAGSegmenter.h"
#include "Segmenter/HMMSegmenter.h"
#include "Segmenter/MixedSegmenter.h"
#include "PosTag/HMMPosTag.h"
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
	//词性标注接口
	std::vector<std::wstring> words;
	double alpha = 0.02;//平滑参数
	HMMPosTag hmm_tag;
	hmm_tag.fit("PosTag/train.conll.txt", alpha);
	//	hmm_tag.appraise("PosTag/dev.conll.txt"); //对测试集的评价。
	tags = hmm_tag.pos_tag(words);//词性标注接口
	for (auto & w : tags) {
		std::cout << codec.to_bytes(w) << " ";
	}

    return 0;
}

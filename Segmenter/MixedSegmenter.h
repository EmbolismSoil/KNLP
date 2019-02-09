#ifndef KLP_MIXEDSEGMENTER_H
#define KLP_MIXEDSEGMENTER_H
#include "DAGSegmenter.h"
#include "HMMSegmenter.h"
#include <boost/algorithm/string.hpp>
#include <cmath>

template<class DIC_SEGMENTER, class LABEL_SEGMENTER>
class MixedSegmenter
{
public:
    MixedSegmenter(DIC_SEGMENTER &dic_segmenter, LABEL_SEGMENTER &label_segmenter):
        _dic_segmenter(dic_segmenter),
        _label_segmenter(label_segmenter)
    {

    }

    MixedSegmenter(MixedSegmenter const&) = delete;
    MixedSegmenter const& operator=(MixedSegmenter const&) = delete;

    void segment(std::wstring const& sentence, std::vector<std::wstring> &words)
    {
        std::vector<std::wstring> dag_words;
        _dic_segmenter.segment(sentence, dag_words);

        std::vector<std::wstring> single_words;
        for (auto pos = dag_words.begin(); pos != dag_words.end(); ++pos)
        {
            if(pos->length() == 1){
                single_words.push_back(std::move(*pos));
            }else if (pos->length() > 1 && !single_words.empty()){
                std::wstring s = boost::join(single_words, "");
                single_words.clear();
                _label_segmenter.segment(s, single_words);
                for(auto & w: single_words){
                    words.push_back(std::move(w));
                }
                words.push_back(std::move(*pos));
                single_words.clear();
            }else{
                words.push_back(std::move(*pos));
            }
        }

        if (single_words.empty()){
            return;
        }

        std::wstring s = boost::join(single_words, "");
        single_words.clear();
        _label_segmenter.segment(s, single_words);
        for(auto & w: single_words){
            words.push_back(std::move(w));
        }
    }

private:
    DIC_SEGMENTER& _dic_segmenter;
    LABEL_SEGMENTER& _label_segmenter;
};
#endif

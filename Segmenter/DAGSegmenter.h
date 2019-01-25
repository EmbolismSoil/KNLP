//
// Created by lee on 19-1-11.
//

#ifndef KLP_BIGRAMDAGSEGMENTER_H
#define KLP_BIGRAMDAGSEGMENTER_H

#include "../LanguageModel/BigramLanguaModel.h"
#include "SentenceDAG.h"
#include <memory>
#include <string>

template <class T>
class DAGSegmenter
{
public:
    DAGSegmenter(std::shared_ptr<T> lm):
        _lm(lm)
    {

    }

    void segment(std::wstring const& sentence, std::vector<std::wstring> &words)
    {
        SentenceDAG<T> dag(sentence, _lm.get());
        dag.shortest_path(words);
    }

private:
    std::shared_ptr<T> _lm;
};


#endif //KLP_BIGRAMDAGSEGMENTER_H

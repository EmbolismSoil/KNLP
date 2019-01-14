//
// Created by lee on 19-1-11.
//

#ifndef KLP_BIGRAMDAGSEGMENTER_H
#define KLP_BIGRAMDAGSEGMENTER_H

#include "../LanguageModel/BigramLanguaModel.h"
#include "../Utils/DAG.h"
#include <memory>
#include <string>

class BigramDAGSegmenter
{
public:
    BigramDAGSegmenter(std::shared_ptr<BigrameLanguageModel<std::wstring>> lm):
        _lm(lm)
    {

    }

    void segment(std::wstring const& sentence, std::vector<std::wstring> &words)
    {
        DAG dag(sentence, _lm);
        dag.shortest_path(words);
    }

private:
    std::shared_ptr<BigrameLanguageModel<std::wstring>> _lm;
};


#endif //KLP_BIGRAMDAGSEGMENTER_H

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
        DAG dag;
        std::unordered_map<std::int64_t, std::wstring> dic;
        DAG::build_from_str(sentence, *_lm, dic, dag);
        std::vector<std::int64_t > path;
        dag.shortest_path(path);

        std::transform(path.rbegin(), path.rend(),
                       std::back_inserter(words), [&dic](std::int64_t idx){ return dic[idx];});
    }

private:
    std::shared_ptr<BigrameLanguageModel<std::wstring>> _lm;
};


#endif //KLP_BIGRAMDAGSEGMENTER_H

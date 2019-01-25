
//
// Created by lee on 19-1-12.
//

#ifndef KLP_BIGRAMELANGUAGEMODEL_H
#define KLP_BIGRAMELANGUAGEMODEL_H

#include <vector>
#include <tuple>
#include <unordered_set>
#include <boost/algorithm/string.hpp>
#include "../Utils/HashDictionary.h"

using wspair = std::pair<std::wstring, std::wstring>;
namespace std
{
    template<>
    struct hash<wspair>: public std::unary_function<wspair, std::size_t>
    {
        hash(): _comp()
        {

        }

        std::size_t operator()(wspair const& key)
        {
            std::wstring concat(key.first + key.second);
            return _comp(concat);
        }

    private:
        std::hash<std::wstring> _comp;
    };
}

template <class S>
class __BigrameLanguageModelBase
{
public:
    std::double_t p(S const& w, S const& cond)
    {
        if (_dic.find(w) == _dic.end() || _dic.find(cond) == _dic.end()){
            return _alpha;
        }

        std::int64_t widx = _dic[w];
        std::int64_t cidx = _dic[cond];

        auto pos = _p.find(cidx);
        if (pos != _p.end() && pos->second.find(widx) != pos->second.end())
        {
            return _p[cidx][widx];
        }else{
            return _alpha;
        }
    }

    bool has(std::wstring const& w)
    {
        return _dic.find(w) != _dic.end();
    }

    std::int64_t id(S const& s)
    {
        if (_dic.find(s) != _dic.end())
            return _dic[s];
        else
            return -1;
    }

protected:
    __BigrameLanguageModelBase(std::double_t alpha=10e-10): _alpha(alpha){}
    std::unordered_map<std::int64_t, std::unordered_map<std::int64_t , std::double_t >> _p;
    //HashDictionary<S, std::int64_t > _dic;
    //trie<S, std::int64_t > _dic;
    std::unordered_map<S, std::int64_t > _dic;
    std::double_t const _alpha;
};


class BigrameLanguageModel : public __BigrameLanguageModelBase<std::wstring>
{
public:
    BigrameLanguageModel():__BigrameLanguageModelBase<std::wstring>(){}

    std::double_t lnp(std::wstring const& w, std::wstring const& cond)
    {
        std::log(p(w, cond));
    }

    void fit(std::string const &path, std::wstring sep=std::wstring()) {
        std::ifstream fin(path);
        std::string line;
        std::wstring_convert<std::codecvt_utf8<wchar_t >> codec;
        std::unordered_map<std::wstring, std::unordered_map<std::wstring, std::uint64_t >> counter;
        std::unordered_map<std::wstring, std::uint64_t > occur;

        if (sep.empty()) {
            sep = codec.from_bytes(" ");
        }

        while (std::getline(fin, line)) {
            std::wstring wline(codec.from_bytes(line));
            boost::trim(wline);
            std::vector<std::wstring> words;
            boost::split(words, wline, boost::is_any_of(sep), boost::token_compress_on);

            if (words.size() <= 1) {
                continue;
            }

            occur[words[0]] += 1;
            for (std::size_t i = 0; i < words.size() - 1; ++i) {
                std::wstring const &cond = words[i];
                std::wstring const &w = words[i + 1];
                std::uint64_t n = counter[cond][w];
                counter[cond][w] = n + 1;

                occur[w] += 1;
            }

        }

        //build vocab
        _build_vocab(occur);
        //normalized p
        _normalize_p(counter);
    }

private:
    void _build_vocab(std::unordered_map<std::wstring, std::uint64_t > & occur)
    {
        std::vector<std::pair<std::wstring, std::uint64_t >> vocab;
        for (auto & c: occur)
        {
            vocab.push_back(std::move(c));
        }

        std::sort(vocab.begin(), vocab.end(),
                  [](std::pair<std::wstring, std::uint64_t > const& lhs,
                     std::pair<std::wstring, std::uint64_t > const& rhs)
                  {
                      return lhs.second > rhs.second;
                  });

        for (std::uint64_t i = 0; i < vocab.size(); ++i)
        {
            _dic[vocab[i].first] = i;
        }
    }

    void _normalize_p(std::unordered_map<std::wstring, std::unordered_map<std::wstring, std::uint64_t >> const& counter)
    {
        std::uint64_t sum = 0;
        for (auto const& condw: counter)
        {
            int64_t condidx = _dic[condw.first];

            for(auto const& w: condw.second)
            {
                sum += w.second;
            }

            for (auto const& w: condw.second)
            {
                std::double_t p = std::double_t(w.second) / sum;
                int64_t widx = _dic[w.first];
                _p[condidx][widx] = p;
            }
        }
    }
};


#endif //KLP_BIGRAMELANGUAGEMODEL_H

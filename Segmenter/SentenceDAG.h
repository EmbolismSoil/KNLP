//
// Created by lee on 19-1-12.
//

#ifndef KLP_DAG_H
#define KLP_DAG_H

#include <vector>
#include <string>
#include "../LanguageModel/BigramLanguaModel.h"
#include <algorithm>

struct Word {
    Word(std::int64_t const s, std::int64_t const e, std::int64_t const i):
        start(s),
        len(e),
        idx(i)
    {

    }

    Word(Word const& w):
        start(w.start),
        len(w.len),
        idx(w.idx)
    {

    }

    std::int64_t const start;
    std::int64_t const len;
    std::int64_t const idx;
};

template<class T>
struct isBigram
{
private:
    template<class U, double_t (U::*)(std::wstring const&, std::wstring const&)> struct __Helper;

    template<class U>
    static uint8_t __has(__Helper<U, &U::lnp> *){}

    template<class U>
    static uint16_t __has(...){}

public:
    enum{value=sizeof(__has<T>(0)) == sizeof(uint8_t)};
};

template <class T, bool bigram>
struct __weight;

template <class T>
struct __weight<T, true>
{
    __weight(T* lm):
        _lm(lm)
    {

    }

    double_t lnp(std::wstring const& cur, std::wstring const& next)
    {
        return _lm->lnp(cur, next);
    }

private:
    T* _lm;
};

template <class T>
struct __weight<T, false>
{
    __weight(T* lm):
            _lm(lm)
    {

    }

    double_t lnp(std::wstring const& cur, std::wstring const& root)
    {
        return _lm->lnp(root);
    }

private:
    T* _lm;
};

template <class T>
class SentenceDAG
{
public:
    SentenceDAG() = default;
    SentenceDAG(SentenceDAG const&) = delete;
    SentenceDAG const& operator=(SentenceDAG const&) = delete;


    void shortest_path(std::vector<std::wstring > &path)
    {
        //init graph, 可以并行化
        __weight<T, isBigram<T>::value> lm(_lm);
        std::int64_t const end = _all_words.size();
        std::vector<double_t > dis(end, 0.0);
        std::vector<std::int64_t > pre(end, 0);

        for (std::int64_t i = 0; i < _all_words.size(); ++i){
            Word const& w = _all_words[i];
            std::int64_t start = w.start;
            std::int64_t const root_idx = w.idx;
            std::wstring const& root_w = _dic[w.idx];

            if (_suffix_table.find(start) == _suffix_table.end()){
                continue;
            }

            double_t min_dis = std::numeric_limits<double_t >::max();
            for(auto pos = _suffix_table[start].begin(); pos != _suffix_table[start].end(); ++pos) {
                std::int64_t const j = pos->idx;
                std::wstring const& cur_w = _dic[j];
                double_t new_dis = dis[j] - lm.lnp(cur_w, root_w);
                if (new_dis < min_dis){
                    min_dis = new_dis;
                    pre[j] = i;
                }
            }

            dis[i] = min_dis;
        }

        //std::int64_t i = end - 1;
        std::int64_t i = 0;
        while(true)
        {
            i = pre[i];
            if (i == end - 1){
                break;
            }
            path.push_back(_dic[i]);
        }
    }

    SentenceDAG(std::wstring const& s, T* lm):
        _lm(lm)
    {
        if (s.empty()){
            return;
        }
        std::wstring_convert<std::codecvt_utf8<wchar_t>> codec;


        std::vector<std::wstring> words;

        words.push_back(codec.from_bytes("<s>"));
        std::transform(s.begin(), s.end(), std::back_inserter(words),
                       [](wchar_t const w){return std::wstring(1, w);});
        words.push_back(codec.from_bytes("</s>"));

        std::wstring src = codec.from_bytes("<s>") + s + codec.from_bytes("</s>");

        std::vector<Word> all_words;
        std::unordered_map<std::int64_t , std::vector<Word>> suffix_table;
        std::unordered_map<std::int64_t, std::wstring> dic;

        //find all dic words
        std::int64_t idx = 0;
        std::int64_t const src_len = src.size();
        for (std::int64_t i = 0, start=0; i < words.size(); start+=words[i].size(), ++i){
            std::wstring const& w = words[i];
            if (w.empty()){
                continue;
            }

            all_words.emplace_back(start, w.size(), idx);
            suffix_table[start+w.size()].emplace_back(start, w.size(), idx);
            dic[idx++] = w;

            for(std::int64_t j = w.size() + 1; j < src_len - start; ++j){
                std::wstring const& extw = src.substr(start, j);
                if (!_lm->has(extw)){
                    break;
                }

                all_words.emplace_back(start, extw.size(), idx);
                suffix_table[start+extw.size()].emplace_back(start, extw.size(), idx);
                dic[idx++] = extw;
            }
        }


        set_dic(std::move(dic));
        set_words(std::move(all_words));
        set_suffix_table(std::move(suffix_table));
    }


    void set_words(std::vector<Word> && words)
    {
        _all_words.swap(words);
    }

    void set_dic(std::unordered_map<std::int64_t, std::wstring> const& dic)
    {
        _dic = dic;
    }

    void set_dic(std::unordered_map<std::int64_t, std::wstring> && dic)
    {
        _dic.swap(dic);
    }

    void set_suffix_table(std::unordered_map<std::int64_t, std::vector<Word>> const& table)
    {
        _suffix_table = table;
    }

    void set_suffix_table(std::unordered_map<std::int64_t, std::vector<Word>> && table)
    {
        _suffix_table.swap(table);
    }


private:
    std::vector<Word> _all_words;
    std::unordered_map<std::int64_t, std::vector<Word>> _suffix_table;
    std::unordered_map<std::int64_t, std::wstring> _dic;
    T* _lm;
};
#endif //KLP_DAG_H

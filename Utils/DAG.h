//
// Created by lee on 19-1-12.
//

#ifndef KLP_DAG_H
#define KLP_DAG_H

#include <vector>
#include "../lib/trie/trie.hpp"
#include <string>
#include "../LanguageModel/BigramLanguaModel.h"

struct Word {
    Word(std::int64_t const s, std::int64_t const e, std::int64_t const i):
        start(s),
        len(e),
        idx(i)
    {

    }

    std::int64_t const start;
    std::int64_t const len;
    std::int64_t const idx;
};

class DAG
{
public:
    DAG() = default;
    DAG(DAG const&) = delete;
    DAG const& operator=(DAG const&) = delete;

    DAG(std::vector<std::vector<std::double_t>> const& graph):
        _graph(graph)
    {

    }

    DAG(std::vector<std::vector<std::double_t>> && graph):
            _graph(std::move(graph))
    {

    }


    void shortest_path(std::vector<std::int64_t > &path)
    {
        std::int64_t end = _graph.size();
        std::vector<std::double_t > dis(end, 0.0);
        std::vector<std::int64_t > pre(end, 0);
        for (size_t i = 1; i < dis.size(); ++i){
            std::double_t min_dis = std::numeric_limits<std::double_t >::max();
            for (size_t j = 0; j < i; ++j){
                if (_graph[j][i] != 0.0){
                    if (dis[j] + _graph[j][i] < min_dis){
                        min_dis = dis[j] + _graph[j][i];
                        pre[i] = j;
                    }
                }
            }

            dis[i] = min_dis;
        }

        std::int64_t i = end - 1;
        while(true)
        {
            i = pre[i];
            if (i == 0){
                break;
            }
            path.push_back(i);
        }
    }

    static void build_from_str(std::wstring const& s,
                               BigrameLanguageModel<std::wstring> & lm,
                               std::unordered_map<std::int64_t, std::wstring>& dic, DAG & dag)
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
        std::unordered_map<std::int64_t , std::vector<Word>> prefix_table;
        //find all dic words
        std::int64_t idx = 0;
        std::int64_t const src_len = src.size();
        for (std::int64_t i = 0, start=0; i < words.size(); start+=words[i].size(), ++i){
            std::wstring const& w = words[i];

            all_words.emplace_back(start, w.size(), idx);
            prefix_table[start].emplace_back(start, w.size(), idx);
            dic[idx++] = w;

            for(std::int64_t j = w.size() + 1; j < src_len - start; ++j){
                std::wstring const& extw = src.substr(start, j);
                if (lm.id(extw) < 0){
                    break;
                }

                all_words.emplace_back(start, w.size(), idx);
                prefix_table[start].emplace_back(start, w.size(), idx);
                dic[idx++] = w;
            }
        }

        //init graph, 可以并行化
        std::vector<std::vector<std::double_t >> graph(all_words.size(), std::vector<std::double_t >(all_words.size(), 0.0));
        for (std::int64_t i = 0; i < all_words.size(); ++i){
            Word const& w = all_words[i];
            std::int64_t start = w.start;
            std::int64_t end = w.start + w.len;
            std::int64_t const root_idx = w.idx;
            std::wstring const& root_w = dic[w.idx];

            if (prefix_table.find(end) == prefix_table.end()){
                continue;
            }

            for(auto pos = prefix_table[end].begin(); pos != prefix_table[end].end(); ++pos) {
                std::wstring const& cur_w = dic[pos->idx];
                graph[root_idx][pos->idx] = -lm.lnp(cur_w, root_w);
            }
        }

        dag.set_graph(std::move(graph));
    }

    void set_graph(std::vector<std::vector<std::double_t >> const& graph)
    {
        _graph = graph;
    }

    void set_graph(std::vector<std::vector<std::double_t>> && graph)
    {
        _graph.swap(graph);
    }

    std::int64_t const end()
    {
        return _graph.size();
    }

private:
    std::vector<std::vector<std::double_t>> _graph;
};
#endif //KLP_DAG_H

//
// Created by lee on 19-1-12.
//

#ifndef KLP_DAG_H
#define KLP_DAG_H

#include <vector>
#include "../lib/trie/trie.hpp"
#include <string>
#include "../LanguageModel/BigramLanguaModel.h"

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

    static void build_from_str(std::wstring const& src,
                               BigrameLanguageModel<std::wstring> & lm,
                               std::unordered_map<std::int64_t, std::wstring>& dic, DAG & dag)
    {
        if (src.empty()){
            return;
        }
        std::wstring_convert<std::codecvt_utf8<wchar_t>> codec;


        trie<std::wstring, std::unordered_map<std::int64_t, std::int64_t> > local_dic;
        std::vector<std::wstring> words;

        std::transform(src.begin(), src.end(), std::back_inserter(words),
                       [](wchar_t const w){return std::wstring(1, w);});

        std::vector<std::pair<std::wstring, std::int64_t >> all_words;

        //std::copy(words.begin(), words.end(), std::back_inserter(all_words));

        //find all dic words
        all_words.push_back(std::make_pair(codec.from_bytes("<s>"), 0));
        for (std::int64_t i = 0; i < words.size(); ++i){
            std::wstring const& w = words[i];
            std::vector<std::wstring const*> ext;
            ext = lm.complete(w);
            if (ext.empty()){
                all_words.push_back(std::make_pair(w, i+3));
            }

            for (auto const& p : ext)
            {
                std::wstring const& extw = *p;
                if(src.find(extw, i) != std::wstring::npos){
                    all_words.push_back(std::make_pair(*p, i+3));
                }
            }
        }
        all_words.push_back(std::make_pair(codec.from_bytes("</s>"), words.size()+3));

        // init graph
        std::vector<std::vector<std::double_t >>
                graph(all_words.size(), std::vector<std::double_t >(all_words.size(), 0.0));


        // build local dic
        for (size_t i = 0; i < all_words.size(); ++i){
            std::pair<std::wstring, std::int64_t > const& p = all_words[i];
            std::wstring const& w = p.first;
            std::int64_t const pos = p.second;

            local_dic[w][pos] = i;
            dic[i] = w;
        }

        //build graph
        std::wstring extended_str = codec.from_bytes("<s>") + src + codec.from_bytes("</s>");
        __do_build(0, 3, extended_str, local_dic, lm, graph);
        dag.set_graph(std::move(graph));
    }

    static void __do_build(std::size_t const root,
                           std::size_t const len,
                           std::wstring const& s,
                           trie<std::wstring, std::unordered_map<std::int64_t, std::int64_t> > & dic,
                           BigrameLanguageModel<std::wstring> &lm,
                           std::vector<std::vector<std::double_t >> &graph)
    {
        std::size_t cur = root + len;
        if (cur >= s.size()){
            return;
        }

        std::wstring root_w(s.substr(root, len));
        std::wstring cur_w(1, s[cur]);


        std::vector<std::wstring const*> words = dic.complete(cur_w);
        auto root_idx = dic[root_w][root];

        for (auto const& pw: words)
        {
            std::wstring const& ew = *pw;
            if (s.find(ew, cur) != cur){
                continue;
            }
            auto idx = dic[ew][cur];

            if (graph[root_idx][idx] != 0.0){
                return;
            }

            std::double_t p = -lm.lnp(ew, root_w);
            graph[root_idx][idx] =  p;
            __do_build(cur, ew.size(), s, dic, lm, graph);
        }
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

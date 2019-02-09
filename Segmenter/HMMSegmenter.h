//
// Created by lee on 19-1-26.
//

#ifndef KLP_HMMSEGMENTER_H
#define KLP_HMMSEGMENTER_H


#include <boost/algorithm/string.hpp>
#include <unordered_map>
#include <fstream>
#include <codecvt>
#include <cmath>

typedef enum {B=0, M, E, S, UNK} label_t;
typedef enum {SS=0, SB, BM, BE, MM, ME, ES, EB, ERR} trans_t;

namespace std
{
    template <>
    struct hash<label_t>
    {
        std::size_t operator()(label_t const& key) const
        {
            return key;
        }
    };

    template <>
    struct hash<trans_t>
    {
        std::size_t operator()(trans_t const& key) const
        {
            return key;
        }
    };
}

label_t operator++(label_t & i)
{
    if (i == B){
        i = M;
        return M;
    }else if (i == M){
        i = E;
        return E;
    }else if (i == E){
        i = S;
        return S;
    }else{
        i = UNK;
        return UNK;
    }
}

class HMMSegmenter {
public:
    HMMSegmenter() = default;
    HMMSegmenter(HMMSegmenter const&) = delete;
    HMMSegmenter const& operator=(HMMSegmenter const&) = delete;

    void fit(std::string const& path, std::wstring sep=std::wstring())
    {
        std::ifstream fin(path);
        std::string line;
        std::wstring_convert<std::codecvt_utf8<wchar_t >> codec;
        std::unordered_map<std::wstring, std::unordered_map<std::wstring, std::uint64_t >> counter;
        std::unordered_map<std::wstring, std::uint64_t > occur;

        if (sep.empty()) {
            sep = codec.from_bytes(" ");
        }

        _pi[M] = 1.0;
        _pi[E] = 1.0;
        std::unordered_map<label_t, double_t > labels_count;
        std::unordered_map<trans_t, double_t > trans_count;

        while (std::getline(fin, line)) {
            std::wstring wline(codec.from_bytes(line));
            boost::trim(wline);
            std::vector<std::wstring> words;
            boost::split(words, wline, boost::is_any_of(sep), boost::token_compress_on);


            if (words.empty() || words.size() == 1) {
                continue;
            }

            auto const& firstw = words[0];
            if (firstw.length() == 1){
                _pi[S] += 1.0;
            } else{
                _pi[B] += 1.0;
            }

            std::vector<label_t> all_labels;

            for (const auto &w : words) {
                std::vector<label_t> labels;
                if (w.length() == 1){
                    all_labels.push_back(S);
                }else if (w.length() == 2){
                    all_labels.push_back(B);
                    all_labels.push_back(E);
                }else{
                    labels = {B};
                    std::vector<label_t> ms(w.length() - 2, M);
                    labels.insert(labels.end(), ms.begin(), ms.end());
                    labels.push_back(E);

                    all_labels.insert(all_labels.end(), labels.begin(), labels.end());
                }
            }

            std::wstring o =  boost::join(words, "");
            _update(all_labels, o, labels_count, trans_count);
        }

        _norm(labels_count, trans_count);
    }

    void segment(std::wstring sentence, std::vector<std::wstring> &words)
    {
        if (sentence.length() == 0){
            return;
        }

        if (sentence.length() == 1){
            words.push_back(sentence);
            return;
        }

        double_t min = -std::numeric_limits<double_t >::max();
        std::vector<std::vector<double_t >>
                sigma(sentence.length(), std::vector<double_t>(UNK, min));

        std::vector<std::vector<label_t>>
                phi(sentence.length(), std::vector<label_t >(UNK, B));

        std::vector<double_t >& init = sigma[0];
        std::vector<label_t >& init_phi = phi[0];
        wchar_t o1 = sentence[0];
        for(label_t i = B; i <= S; ++i)
        {
            init[i] = _pi[i] + emit_prob(i, o1);
            init_phi[i] = B;
        }

        for (auto i = 1; i < sentence.length(); ++i)
        {
            wchar_t const& o = sentence[i];
            for (auto j = B; j <= S; ++j){
                label_t pre = B;
                for (auto k = B; k <= S; ++k){
                    double_t p = sigma[i-1][k] + trans_prob(k, j) + emit_prob(j, o);
                    if (p > sigma[i][j])
                    {
                        pre = k;
                        sigma[i][j] = p;
                    }
                }
                phi[i][j] = pre;
            }
        }

        label_t max_label = B;
        auto const& sigma_t = *sigma.rbegin();
        for (auto i = B; i <= S; ++i)
        {
            if (sigma_t[i] > min){
                min = sigma_t[i];
                max_label = i;
            }
        }

        std::stack<label_t> path;
        path.push(max_label);
        for (int i = sentence.length() - 2; i >= 0; --i)
        {
            max_label = phi[i+1][max_label];
            path.push(max_label);
        }

        size_t idx = 0;
        size_t start = 0;
        size_t len = 0;
        while (!path.empty())
        {
            auto label = path.top();
            path.pop();
            switch (label){
                case B:
                    len += 1;
                    break;
                case M:
                    len += 1;
                    break;

                case E:
                case S:
                    len += 1;
                    words.push_back(sentence.substr(start, len));
                    start += len;
                    len = 0;
                    break;
                default:
                    assert(false);
            }
        }

        if (len != 0){
            words.push_back(sentence.substr(start, len));
        }
    }

    double_t emit_prob(label_t const label, wchar_t const& w)
    {
        auto pos = _emit_prob.end();
        if ((pos = _emit_prob.find(label)) != _emit_prob.end())
        {
            auto wpos = pos->second.end();
            if ((wpos = pos->second.find(w)) != pos->second.end())
            {
                return wpos->second;
            }
        }

        return std::log(1e-30);
    }

    double_t trans_prob(label_t const from, label_t const to)
    {
        trans_t trans = to_trans(from, to);
        auto pos = _trans_prob.begin();
        if ((pos = _trans_prob.find(trans)) != _trans_prob.end()){
            return pos->second;
        }else{
            return std::log(1e-30);
        }
    }

private:
    std::unordered_map<label_t, std::unordered_map<wchar_t, double_t >> _emit_prob;
    std::unordered_map<trans_t, double_t> _trans_prob;
    std::unordered_map<label_t, double_t > _pi;

    trans_t to_trans(label_t const& lhs, label_t const& rhs)
    {
        if (lhs == S && rhs == S){
            return SS;
        }else if (lhs == S && rhs == B){
            return SB;
        }else if (lhs == B && rhs == M){
            return BM;
        }else if (lhs == B && rhs == E){
            return BE;
        }else if (lhs == M && rhs == M){
            return MM;
        }else if (lhs == M && rhs == E){
            return ME;
        }else if (lhs == E && rhs == S){
            return ES;
        }else if (lhs == E && rhs == B){
            return EB;
        }else{
            return ERR;
        }
    }

    void _update(std::vector<label_t> const& labels,
                 std::wstring const& w,
                 std::unordered_map<label_t, double_t >& labels_count,
                 std::unordered_map<trans_t, double_t >& trans_count)
    {
        assert(labels.size() == w.length());
        label_t last = labels[0];
        _emit_prob[labels[0]][w[0]] += 1.0;


        for(auto i = 1; i < labels.size(); ++i){
            label_t const& label = labels[i];
            wchar_t c = w[i];
            _emit_prob[label][c] += 1.0;
            labels_count[label] += 1.0;

            trans_t trans = to_trans(last, label);
            assert(trans != ERR);
            _trans_prob[trans] += 1.0;
            trans_count[trans] += 1.0;
            last = label;
        }
    }

    void _norm(std::unordered_map<label_t, double_t >& labels_count,
                                std::unordered_map<trans_t, double_t >& trans_count)
    {
        for (auto &l : _emit_prob){
            for(auto &e: l.second){
                e.second = std::log(e.second / labels_count[l.first]);
            }
        }

        for (auto &t: _trans_prob){
            t.second = std::log(t.second / trans_count[t.first]);
        }

        double_t n = _pi[S] + _pi[B] + _pi[M] + _pi[E];
        for (auto &e : _pi){
            e.second = std::log(e.second/n);
        }
    }
};


#endif //KLP_HMMSEGMENTER_H

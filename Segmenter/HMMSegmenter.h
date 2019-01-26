//
// Created by lee on 19-1-26.
//

#ifndef KLP_HMMSEGMENTER_H
#define KLP_HMMSEGMENTER_H


#include <boost/algorithm/string.hpp>
#include <unordered_map>
#include <fstream>
#include <codecvt>

class HMMSegmenter {
public:
    typedef enum {B=0, M, E, S} label_t;
    typedef enum {SS=0, SB, BM, BE, MM, ME, ES, EB, ERR} trans_t;

    HMMSegmenter() = default;
    HMMSegmenter(HMMSegmenter const&) = delete;
    HMMSegmenter const& operator=(HMMSegmenter const&) = delete;

    void fit(std::string path, std::wstring sep)
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

        while (std::getline(fin, line)) {
            std::wstring wline(codec.from_bytes(line));
            boost::trim(wline);
            std::vector<std::wstring> words;
            boost::split(words, wline, boost::is_any_of(sep), boost::token_compress_on);


            if (words.empty() || words.size() == 1) {
                continue;
            }

            label_t last_label = S;
            auto const& firstw = words[0];
            if (firstw.length() == 1){
                last_label = S;
                _pi[S] += 1.0;
            } else{
                last_label = B;
                _pi[B] += 1.0;
            }

            for (auto pos = words.cbegin(); pos != words.cend(); ++pos)
            {
                auto const& w = *pos;
                std::vector<label_t> labels;
                if (w.length() == 1){
                    labels = {S};
                }else if (w.length() == 2){
                    labels = {B, E};
                }else{
                    labels = {B};
                    std::vector<label_t> ms(w.length() - 2, M);
                    labels.insert(labels.end(), ms.begin(), ms.end());
                    labels.push_back(E);
                }

                _update(last_label, labels, w);
            }
        }

        _norm();
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

        std::vector<std::double_t > alpha(sentence.length(), 0.0);
        std::vector<label_t> path;
        double_t ps = emit_prob(S, sentence[0]);
        double_t pb = emit_prob(B, sentence[0]);

        label_t pi = ps > pb ? S : B;
        alpha[0] = pi;
        path.push_back(pi);

        for (auto i = 1; i < sentence.length(); ++i)
        {
            
        }
    }

    std::double_t emit_prob(label_t const label, wchar_t const& w)
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

        return 0.0;
    }


private:
    std::unordered_map<label_t, std::unordered_map<wchar_t, std::double_t >> _emit_prob;
    std::unordered_map<trans_t, std::double_t> _trans_prob;
    std::unordered_map<label_t, std::double_t > _pi;
    std::unordered_map<label_t, std::double_t > _labels_count;
    std::unordered_map<trans_t, std::double_t > _trans_count;

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

    void _update(label_t& last, std::vector<label_t> const& labels, std::wstring const& w)
    {
        for(auto i = 0; i < labels.size(); ++i){
            label_t const& label = labels[i];
            wchar_t c = w[i];
            _emit_prob[label][c] += 1.0;
            _labels_count[label] += 1.0;

            trans_t trans = to_trans(last, label);
            assert(trans != ERR);
            _trans_prob[trans] += 1.0;
            _trans_count[trans] += 1.0;
            last = label;
        }
    }

    void _norm()
    {
        for (auto &l : _emit_prob){
            for(auto &e: l.second){
                e.second = std::log(e.second / _labels_count[l.first]);
            }
        }

        for (auto &t: _trans_prob){
            t.second = std::log(t.second / _trans_count[t.first]);
        }

        std::double_t n = _pi[S] + _pi[B] + _pi[M] + _pi[E];
        for (auto &e : _pi){
            e.second = std::log(e.second/n);
        }
    }
};


#endif //KLP_HMMSEGMENTER_H

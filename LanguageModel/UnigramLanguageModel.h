//
// Created by lee on 19-1-26.
//

#ifndef KLP_UNIGRAMLANGUAGEMODEL_H
#define KLP_UNIGRAMLANGUAGEMODEL_H

#include <string>
#include <fstream>
#include <codecvt>
#include <unordered_map>
#include <boost/algorithm/string.hpp>
#include <vector>
#include <cmath>

class UnigramLanguageModel
{
public:
    UnigramLanguageModel() = default;

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

        uint64_t N = 0;
        while (std::getline(fin, line)) {
            std::wstring wline(codec.from_bytes(line));
            boost::trim(wline);
            std::vector <std::wstring> words;
            boost::split(words, wline, boost::is_any_of(sep), boost::token_compress_on);

            if (words.size() != 3 ) {
                continue;
            }

            auto n = std::stoul(words[1]);
            N += n;
            _fre[words[0]] = (double_t)std::stoul(words[1]);
        }

        for (auto &w : _fre)
        {
            w.second = std::log(w.second/N);
        }
    }

    bool has(std::wstring const& w)
    {
        return _fre.find(w) != _fre.end();
    }

    double_t lnp(std::wstring const& w)
    {
        if (!has(w)){
            return 0.0;
        }

        return _fre[w];
    }
private:
    std::unordered_map<std::wstring, double_t > _fre;
};

#endif //KLP_UNIGRAMLANGUAGEMODEL_H

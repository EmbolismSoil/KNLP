//
// Created by lee on 19-1-11.
//

#ifndef KLP_HASHDICTIONARY_H
#define KLP_HASHDICTIONARY_H

#include <fstream>
#include <regex>
#include <iterator>
#include <unordered_map>
#include <codecvt>

template <typename T>
T lexical_cast(const std::string& str)
{
    T var;
    std::istringstream iss;
    iss.str(str);
    iss >> var;
    return var;
}

template <typename T>
T wlexical_cast(const std::wstring& str)
{
    T var;
    std::wstring_convert<std::codecvt_utf8<std::wstring>> convert;
    std::istringstream iss;
    iss.str(convert.to_bytes(str));
    iss >> var;
    return var;
}

template <typename T>
T wlexical_cast(const std::wstring& str, std::wstring_convert<std::codecvt_utf8<std::wstring>> & convert)
{
    T var;
    std::istringstream iss;
    iss.str(convert.to_bytes(str));
    iss >> var;
    return var;
}

template <class S, class N>
class __HashDictionaryBase
{
public:
    __HashDictionaryBase(){}
    __HashDictionaryBase(__HashDictionaryBase const& ) = delete;
    __HashDictionaryBase const& operator=(__HashDictionaryBase const&) = delete;

    bool contains(S const& w) const
    {
        return _dic.find(w) != _dic.end();
    }

    void insert(S const &w, N const n, bool replace=false)
    {
        if (replace || _dic.find(w) == _dic.end())
        {
            _dic[w] = n;
        }
    }

    N const get(S const& w)
    {
        if (_dic.find(w) != _dic.end()){
            return _dic[w];
        }

        throw std::invalid_argument("word does not exist");
    }

    void inc(S const& w, N const n, bool create=true)
    {
        if (_dic.find(w) != _dic.end()){
            _dic[w] = _dic[w] + n;
        }else if (create){
            _dic[w] = n;
        }
    }


private:
    std::unordered_map<S, N> _dic;
};

template <class S, class N>
class HashDictionary;

template <class N>
class HashDictionary<std::string, N> : public __HashDictionaryBase<std::string, N>  {
public:
    HashDictionary(){}
    HashDictionary(std::string const& path, std::string sep="\t")
    {
        std::string line;
        std::ifstream fin(path);
        std::regex re(sep);

        std::stringstream ss;

        while (std::getline(fin, line)) {
            if (line.empty()) {
                continue;
            }

            std::vector<std::string> items(std::sregex_token_iterator(line.begin(), line.end(), re), std::sregex_token_iterator());
            if (items.size() != 2) {
                continue;
            }

            std::string const& w = items[0];
            N const& n = lexical_cast<N>(items[1]);
            insert(w, n);
            ss.str("");
        }
    }
};

template <class N>
class HashDictionary<std::wstring, N> : public __HashDictionaryBase<std::wstring, N>  {
public:
    HashDictionary(){}
    HashDictionary(std::string const& path, std::wstring sep=std::wstring())
    {
        std::string line;
        std::ifstream fin(path);
        std::wstring_convert<std::codecvt_utf8<wchar_t >> convert;
        if (sep.empty()){
            sep = convert.from_bytes("\t");
        }
        std::wregex re(sep);

        while (std::getline(fin, line)) {
            if (line.empty()) {
                continue;
            }
            std::wstring words(convert.from_bytes(line));
            std::vector<std::wstring> items(std::wsregex_token_iterator(words.begin(), words.end(), re), std::wsregex_token_iterator());
            if (items.size() != 2) {
                continue;
            }

            std::wstring const& w = items[0];
            N const& n = items[1];

            insert(w, wlexical_cast<std::wstring>(n, convert));
        }
    }
};

#endif //KLP_HASHDICTIONARY_H

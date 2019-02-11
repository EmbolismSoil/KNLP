#ifndef KLP_DATASET_H
#define KLP_DATASET_H
#include<fstream>
#include<vector>
#include<string>
#include<unordered_set>
#include <locale>   
#include <codecvt>
#include<iostream>
struct sentence
{
	std::vector<std::wstring> word;
	std::vector<std::wstring> tag;
	std::vector<std::vector<std::wstring>> word_char;
};
class dataset
{
public:
	std::vector<sentence> sentences;
	std::string name;
	int sentence_count = 0, word_count = 0;
	void read_data(const std::string & file_name)
	{
		//	std::locale::global(std::locale("Chinese-simplified"));
		std::wstring_convert<std::codecvt_utf8<wchar_t>> codec;
		name = file_name;
		std::ifstream file(file_name);
		/*
		if (!file)
		{
			std::cout << "don't open " + file_name << std::endl;
		}
		*/
		std::string line;
		sentence sen;
		int t0, t1, t2, t3;
		std::wstring word, tag;
		std::wstring wline;
		while (getline(file, line))
		{
			wline=codec.from_bytes(line);
			if (line.size() == 0)
			{
				sentences.emplace_back(std::move(sen));
				sen.~sentence();
				sentence_count++;
				continue;
			}
			word_count++;
			t0 = wline.find(L"\t") + 1;
			t1 = wline.find(L"\t", t0);
			word = wline.substr(t0, t1 - t0);
			t2 = wline.find(L"\t", t1 + 1) + 1;
			t3 = wline.find(L"\t", t2);
			tag = wline.substr(t2, t3 - t2);
			sen.word.emplace_back(std::move(word));
			sen.tag.emplace_back(std::move(tag));
			//构建词的单个元素。
			std::vector<std::wstring> word_char;
			for (int i = 0; i < word.size(); ++i)
			{
				word_char.emplace_back(std::move(word.substr(i, 1)));
			}
			sen.word_char.emplace_back(std::move(word_char));
		}
		std::cout << name << "contains sentence count " << sentence_count << std::endl;
		std::cout << name << "contains word count " << word_count << std::endl;
	}
};


#endif KLP_DATASET_H
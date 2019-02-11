#ifndef KLP_HMMPOSTAG_H
#define KLP_HMMPOSTAG_H

#include <unordered_map>
#include <fstream>
#include <codecvt>
#include<unordered_map>
#include <limits>  
#include"dataset.h"
#include"assert.h"
using map_map_int = std::unordered_map<std::wstring, std::unordered_map<std::wstring, std::uint64_t>>;
using map_map_double = std::unordered_map<std::wstring, std::unordered_map<std::wstring, double_t>>;
using map_int=std::unordered_map<std::wstring, std::uint64_t>;
class HMMPosTag
{
public:
	HMMPosTag() = default;
	HMMPosTag(HMMPosTag const &) = delete;
	HMMPosTag const& operator=(HMMPosTag const&) = delete;

	//计算转移概率和发射概率。
	void fit(const std::string &path,double alpha)
	{
	//	std::locale::global(std::locale("Chinese-simplified"));
		//加载数据。
		dataset  train;
		train.read_data(path);
		//发射频率相关
		map_int all_tag;//词性及其对应个数。
		map_map_int word_tag_count;//词及其对应的词性的个数。
		//转移频率相关
		map_map_int current_former_count;//词及其对应的前一个词性的个数。
		map_int first_tag;
		//平滑相关参数
		int  all_word = 0;
		int all_first_tag = train.sentence_count;
		//中间参数
		map_int correspoding_tag;
		std::wstring current_word, former_tag, current_tag;

		//计算频数。
		
		for (int i=0; i< train.sentences.size(); i++)
		{
			first_tag[train.sentences[i].tag[0]]++;//第一个词性

			for (int j = 0; j < train.sentences[i].tag.size(); j++)
			{
				//发射频数
				std::wstring current_word = train.sentences[i].word[j];
				std::wstring current_tag = train.sentences[i].tag[j];
				all_tag[train.sentences[i].tag[j]]++;
				if (tag.find(current_tag) == tag.end())
				{
					tag.emplace(current_tag,tag.size());
					vector_tag.emplace(vector_tag.size(),current_tag);
				}
				auto w = word_tag_count.find(current_word);
				if (w!= word_tag_count.end())
				{
					auto w0 = w->second.find(current_tag);
					if (w0 != w->second.end())
					{
						w0->second++;
					}
					else
					{
						word_tag_count[current_word].emplace( current_tag,1 );
					}
				}
				else
				{
					std::unordered_map<std::wstring, std::uint64_t> correspoding_tag{ {current_tag,1 } };
					word_tag_count.emplace( current_word,correspoding_tag );
				}
			}
		}
		for (int i = 0; i < train.sentences.size(); i++)
		{				//转移频数
			for (int j = 1; j < train.sentences[i].tag.size()-1; j++)
			{
				std::wstring	current_tag = train.sentences[i].tag[j + 1];
				former_tag = train.sentences[i].tag[j];
				auto w = current_former_count.find(current_tag);
				if (w != current_former_count.end())
				{
					auto w0 = w->second.find(former_tag);
					if (w0 != w->second.end())
					{
						w0->second++;
					}
					else
					{
						current_former_count[current_tag].emplace(former_tag, 1);
					}
				}
				else
				{
					std::unordered_map<std::wstring, std::uint64_t> correspoding_tag{ {former_tag,1 } };
					current_former_count.emplace(current_tag, correspoding_tag);
				}

			}
		}
		all_word = word_tag_count.size();
		count_prob(word_tag_count, current_former_count, all_tag, first_tag, all_word, alpha, all_first_tag);
	}
	void count_prob(map_map_int &word_tag_count,map_map_int &current_former_count, map_int &all_tag, map_int &first_tag,int all_word,double alpha,int all_first_tag)
	{
		std::unordered_map<std::wstring, double_t> other;
		double_t prob = 0.0;
		//发射概率
		for (auto w = word_tag_count.begin(); w != word_tag_count.end(); ++w)
		{
			for (auto w0 = w->second.begin(); w0 != w->second.end(); ++w0)
			{
				prob = (w0->second + alpha) / double_t(all_tag[w0->first] + all_word * alpha);
				other.emplace(w0->first, prob);
			}
			for (auto w = all_tag.begin(); w != all_tag.end(); w++)
			{
				if (other.find(w->first) == other.end())
				{
					prob = alpha / double_t(w->second + all_word * alpha);
					other.emplace(w->first, prob);
				}
			}
			launch_prob.emplace(w->first, other);
			other.clear();
		}
		//给未登录词加上平滑。
		for (auto w = all_tag.begin(); w != all_tag.end(); w++)
		{
			prob = alpha / double_t(w->second + all_word * alpha);
			other.emplace(w->first, prob);
		}
		launch_prob.emplace(L"**", other);
		//转移概率
		other.clear();
		for (auto w = current_former_count.begin(); w != current_former_count.end(); w++)
		{
			for (auto w0 = w->second.begin(); w0 != w->second.end(); w0++)
			{
				prob = (w0->second + alpha) / double_t(all_tag[w0->first] + all_word * alpha);
				//	prob = (w0->second) / double(all_tag[w0->first]);
				other.emplace(w0->first, prob);
			}

			//当前词如果是第一个词的词性，那么前一个词性的频数没有统计在上面的频数。需要加平滑处理。
			if (first_tag.find(w->first) != first_tag.end())
			{
			//	prob = (first_tag[w->first]) / double(all_first_tag );
				prob = (first_tag[w->first] + alpha) / double_t(all_first_tag + all_word * alpha);
				other.emplace(L"start", prob);
			}
			else
			{
				//	prob =  alpha / double(all_first_tag + first_tag.size() * alpha);
				prob = alpha / double_t(all_first_tag + all_word * alpha);

				other.emplace(L"start", prob);
			}

			//平滑，加上当前词的前一个词性可能是其他词性。
			for (auto w1 = all_tag.begin(); w1 != all_tag.end(); w1++)
			{
				if (other.find(w1->first) == other.end())
				{
					prob = alpha / double_t(w1->second + all_word * alpha);
					other.emplace(w1->first, prob);
				}
			}

			transfer.emplace(w->first, other);
			other.clear();
		}
		//把只在第一个词性中，而不在除了第一个词性的其他词性的词加入。
		for (auto z = first_tag.begin(); z != first_tag.end(); z++)
		{
			if (transfer.find(z->first) == transfer.end())
			{
				std::unordered_map<std::wstring, double_t> other;
				other.emplace(L"start", (z->second + alpha) / double_t(all_first_tag + all_word * alpha));
				for (auto z = all_tag.begin(); z != all_tag.end(); z++)
				{
					if (other.find(z->first) == other.end())
					{
						other.emplace(z->first, alpha / double_t(z->second + all_word * alpha));
					}
				}
				transfer.emplace(z->first, other);
			}
		}
		for (auto w = transfer.begin(); w != transfer.end(); w++)
		{
			for (auto w0 = w->second.begin(); w0 != w->second.end(); w0++)
			{

				w0->second = std::log(w0->second);
			}
		}
		for (auto w = launch_prob.begin(); w != launch_prob.end(); w++)
		{
			for (auto w0 = w->second.begin(); w0 != w->second.end(); w0++)
			{

				w0->second = std::log(w0->second);
			}
		}

	}
	std::vector<std::wstring> pos_tag(const std::vector<std::wstring> &words)
	{
		std::vector<std::vector<double_t>> prob(words.size(), std::vector<double_t>(tag.size(), (std::numeric_limits<double_t>::min)()));
		std::vector<std::vector<std::uint64_t>> path(words.size(), std::vector<std::uint64_t>(tag.size(), -1));
		//处理第一个词:
		auto current_word_tag = launch_prob.find(words[0]);
		if (current_word_tag == launch_prob.end())
		{
			current_word_tag = launch_prob.find(L"**");
		}
		for (auto launch_tag_prob = current_word_tag->second.begin(); launch_tag_prob != current_word_tag->second.end(); launch_tag_prob++)
		{
			prob[0][tag[launch_tag_prob->first]] = launch_tag_prob->second + transfer[launch_tag_prob->first][L"start"];
		}
		//处理其他词：
		for (int i = 1; i < words.size(); ++i)
		{
			auto current_word_tag = launch_prob.find(words[i]);
			if (current_word_tag == launch_prob.end())
			{
				current_word_tag = launch_prob.find(L"**");
			}
			for (auto launch_tag_prob = current_word_tag->second.begin(); launch_tag_prob != current_word_tag->second.end(); launch_tag_prob++)
			{
				std::vector<double_t> local_prob(tag.size());
				for (int j=0;j<tag.size();++j)
				{
					local_prob[j] = launch_tag_prob->second + transfer[launch_tag_prob->first][vector_tag[j]] + prob[i - 1][j];

				}
				auto w = max_element(local_prob.begin(), local_prob.end());
				prob[i][tag[launch_tag_prob->first]] = *w ;
				path[i][tag[launch_tag_prob->first]] = distance(local_prob.begin(), w);
			}
		}

		//回溯
		std::vector<std::wstring> words_tag;
		auto w= max_element(prob[words.size()-1].begin(),prob[words.size()-1].end());

		std::uint64_t index = distance(prob[words.size() - 1].begin(), w);
		for (int j = words.size() - 1; j > 0; j--)
		{
			words_tag.emplace_back(vector_tag[index]);
			index = path[j][index];
		}
		words_tag.emplace_back(vector_tag[index]);
		reverse(words_tag.begin(),words_tag.end());

		return words_tag;
	}
	/*
	void appraise(const std::string &file_name)
	{
		dataset  sens;
		sens.read_data(file_name);
		int correct= 0;
		std::vector<std::wstring> max_tag;
		for (auto z = sens.sentences.begin(); z != sens.sentences.end(); z++)
		{
			max_tag=pos_tag(z->word);
			assert(max_tag.size() == z->tag.size());
			for (int i=0; i< max_tag.size(); i++)
			{
				if (max_tag[i] == z->tag[i])
				{	
					correct++;
				}
			}
		}
		std::cout << "标注词性的准确率为" << correct << "/" << sens.word_count << "=" << correct / double(sens.word_count) << std::endl;
	}
	*/
private:
	map_map_double transfer;
	map_map_double launch_prob;
	map_int tag;
	std::unordered_map<std::uint64_t, std::wstring> vector_tag;

};
#endif KLP_HMMPOSTAG_H
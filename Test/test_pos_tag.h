#ifndef KLP_TESTPOSTAG_H
#define KLP_TESTPOSTAG_H

#include"gtest/gtest.h"
#include"HMMPosTag.h"
#include<iostream>
class test_pos_tag :public testing::Test
{
protected:
	// You can remove any or all of the following functions if its body
	// is empty.

	test_pos_tag()
	{
		// You can do set-up work for each test here.
		_hmm_pos_tag = std::make_shared<HMMPosTag>();
		double alpha = 0.2;
		_hmm_pos_tag->fit("../../train.conll.txt", alpha);
		std::cout << "success" << std::endl;
	}

	virtual ~test_pos_tag() {
		// You can do clean-up work that doesn't throw exceptions here.
	}

	// If the constructor and destructor are not enough for setting up
	// and cleaning up each test, you can define the following methods:

	virtual void SetUp() {
		// Code here will be called immediately after the constructor (right
		// before each test).
	}

	virtual void TearDown() {
		// Code here will be called immediately after each test (right
		// before the destructor).
	}

	void test_hmm_pos_tag(std::string const& file_name)
	{
		dataset  sens;
		sens.read_data(file_name);
		int correct = 0;
		std::vector<std::wstring> max_tag;
		for (auto z = sens.sentences.begin(); z != (sens.sentences.begin()+5); z++)
		{
			max_tag = _hmm_pos_tag->pos_tag(z->word);
			assert(max_tag.size() == z->tag.size());
			for (int i = 0; i < max_tag.size(); i++)
			{
				EXPECT_STREQ(max_tag[i].c_str(), z->tag[i].c_str());
				if (max_tag[i] == z->tag[i])
				{
					correct++;
				}
			}
		}
		//std::cout << "标注词性的准确率为" << correct << "/" << sens.word_count << "=" << correct / double(sens.word_count) << std::endl;
	}

	// Objects declared here can be used by all tests in the test case for Foo.
	std::shared_ptr<HMMPosTag> _hmm_pos_tag;
};
TEST_F(test_pos_tag,HMM)
{
	test_hmm_pos_tag("../../dev.conll.txt");

};

#endif KLP_TESTPOSTAG_H

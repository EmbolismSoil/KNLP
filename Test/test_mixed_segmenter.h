#ifndef __TEST_MIXED_SEGMENTER_H__
#define __TEST_MIXED_SEGMENTER_H__

#include "../Segmenter/MixedSegmenter.h"
#include "../LanguageModel/UnigramLanguageModel.h"
#include "gtest/gtest.h"
#include <boost/algorithm/string.hpp>
#include <iostream>

class TestSegmenter : public testing::Test {
 protected:
  // You can remove any or all of the following functions if its body
  // is empty.
 
  TestSegmenter() {
    // You can do set-up work for each test here.
      _unigram_model = std::make_shared<UnigramLanguageModel>();
      _unigram_model->fit("../../dic.txt");

      _hmm_segmenter = std::make_shared<HMMSegmenter>();
      _hmm_segmenter->fit("../../corpus.txt");

      _dag_segmenter = std::make_shared<DAGSegmenter<UnigramLanguageModel>>(*_unigram_model);
      _mixed_segmenter = std::make_shared<MixedSegmenter<DAGSegmenter<UnigramLanguageModel>, HMMSegmenter>>(*_dag_segmenter, *_hmm_segmenter);
  }
 
  virtual ~TestSegmenter() {
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

  void test_mixed_segmenter(std::wstring const& sentence, std::wstring const& expect)
  {
      std::vector<std::wstring> words;
      _mixed_segmenter->segment(sentence, words);
      auto output = boost::join(words, L" ");
      EXPECT_STREQ(output.c_str(), expect.c_str());
  }

  // Objects declared here can be used by all tests in the test case for Foo.
    std::shared_ptr<UnigramLanguageModel> _unigram_model;
    std::shared_ptr<DAGSegmenter<UnigramLanguageModel>> _dag_segmenter;
    std::shared_ptr<HMMSegmenter> _hmm_segmenter;
    std::shared_ptr<MixedSegmenter<DAGSegmenter<UnigramLanguageModel>, HMMSegmenter>> _mixed_segmenter;
};

TEST_F(TestSegmenter, TestMixedSegmenter)
{
    test_mixed_segmenter(L"买水果然后去世博园", L"买 水果 然后 去 世博园");
    test_mixed_segmenter(L"胡歌是仙剑一的主演", L"胡歌 是 仙剑 一 的 主演");
    test_mixed_segmenter(L"这个把手该换了", L"这个 把手 该 换了");
    test_mixed_segmenter(L"别把手放在我的肩膀上", L"别把 手 放在 我的 肩膀 上");
    test_mixed_segmenter(L"质量和服务一样重要", L"质量 和 服务 一样 重要");
    test_mixed_segmenter(L"他正在量和服尺寸", L"他 正在 量 和服 尺寸");
}

#endif
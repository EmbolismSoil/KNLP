#ifndef KNLP_CRFSEGMENTER_H
#define KNLP_CRFSEGMENTER_H

#include <dequeue>

class CRFSegmenter{
public:
    CRFSegmenter() = default;
    CRFSegmenter(CRFSegmenter const&) = delete;
    CRFSegmenter const& operator=(CRFSegmenter const&) = delete;

    void fit(std::string const& path)
    {
    }

private:
};

#endif

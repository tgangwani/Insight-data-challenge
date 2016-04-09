#include<list>

struct tweet {
  std::list<std::string> hashtags;
  uint64_t creation_time;

  // constructor
  tweet(const std::list<std::string>& h, uint64_t t): hashtags(h), creation_time(t) {
  }

  // print tweet
  void print() const {
    std::cout<<"["<<creation_time<<"] ";
    for(auto& s: hashtags) {
      std::cout<<s<<";";
    }
    std::cout<<"\n";
  }

};

struct orderTweets {
  // true is first tweet is more recent in time
  bool operator() (const tweet& tw1, const tweet& tw2) {
    return tw1.creation_time > tw2.creation_time;
  }
};

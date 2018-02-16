#ifndef MINI_REGEXP_RESULT_H_
#define MINI_REGEXP_RESULT_H_

namespace mini_regexp_result
{
    class RE_Result
    {
    public:
        unsigned int count;
        std::vector<std::string> matched;
    
    public:
        RE_Result():count(0),matched() {}
    };
}
#endif
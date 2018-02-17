#ifndef MINI_REGEXP_RESULT_H_
#define MINI_REGEXP_RESULT_H_

namespace mini_regexp_result
{
    class RE_Result
    {
    public:
        unsigned int count;
        std::vector<std::string> sub_matched;
        std::vector<std::string> matched;
    
    public:
        RE_Result():count(0),sub_matched(),matched() {}

        unsigned int get_match_count()
        {
            return count;
        }

        std::vector<std::string>& get_submatched()
        {
            return sub_matched;
        }

        std::vector<std::string>& get_matched()
        {
            return matched;
        }
    };
}
#endif
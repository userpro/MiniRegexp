#ifndef __IUTF8_STRING__
#define __IUTF8_STRING__

class iutf8string
{
    public:
        iutf8string(const std::string& );

        iutf8string(const char* );

        ~iutf8string();

    public:
    
        int length();

        std::string substring(int start_index, int length);

        std::string get(int index);


        iutf8string operator + (iutf8string& );
        
        std::string operator [](int index);

        std::string stlstring();

        const char* c_str();

        iutf8string utf8substr(int u8start_index, int u8_length);
        
        std::string substr(int u8start_index, int u8_length);

    private:

        std::string data;
        int* offerset;
        int _length;

        void refresh();
};

#endif
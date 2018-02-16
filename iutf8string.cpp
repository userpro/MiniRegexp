#include <iostream>
#include <string>
#include "iutf8string.h"

using namespace std::string;

iutf8string::iutf8string(const string& str)
{
    data = str;
    refresh();
}

iutf8string::iutf8string(const char* str)
{
    data = string(str);
    refresh();
}

iutf8string::~iutf8string()
{
    delete[] offerset;
}

string iutf8string::stlstring()
{
    return data;
}

const char* iutf8string::c_str()
{
    return data.c_str();
}

iutf8string iutf8string::operator +(iutf8string& ustr)
{
    string temp = data + ustr.stlstring();

    return iutf8string(temp);
}

int iutf8string::length()
{

    return _length;
}

string iutf8string::get(int index)
{
    if(index >= _length) return "";
    string temp = data.substr(offerset[index], offerset[index+1] - offerset[index]);

    return temp;
}

string iutf8string::operator [](int index)
{
    if(index >= _length) return "";
    string temp = data.substr(offerset[index], offerset[index+1] - offerset[index]);

    return temp;
}

string iutf8string::substr(int u8_start_index, int u8_length)
{
    if(u8_start_index + u8_length >= _length) return "";

    return data.substr(offerset[u8_start_index], offerset[u8_start_index+u8_length] - offerset[u8_start_index]);
}

iutf8string iutf8string::utf8substr(int u8_start_index, int u8_length)
{
    if(u8_start_index + u8_length >= _length) return iutf8string("");
    string ret = data.substr(offerset[u8_start_index], offerset[u8_start_index+u8_length] - offerset[u8_start_index]);
    
    return iutf8string(ret);
}

void iutf8string::refresh()
{
    int *tmp = new int[data.length()];
    int i, tmpidx = 0;
    for(i = 0; i < data.length(); i++)
    {
        if(((int)data[i] > 0)||(!(((int)data[i] & 0x00000040) == 0)))
        {
            tmp[tmpidx] = i;
            tmpidx++;
        }
    }

    tmp[tmpidx] = data.length();

    int *tmp2 = new int[tmpidx];
    for(i = 0; i < tmpidx; i++)
    {
        tmp2[i] = tmp[i];
    }


    delete[] tmp;
    offerset = tmp2;
    _length = tmpidx;
}


//----------------test code ----------------------------
int main()
{
    iutf8string str1("_我Love你！中国  ,!");
    cout << "字符串长度:" <<str1.length() <<endl;
    int i; cout << "[" ;
    for(i = 0; i < str1.length(); i++)
    {
        cout << str1[i] << " ";
    }
    cout << "]" << endl;
    string one = str1.substr(2,11);
    cout << one << endl;

    string s1("我们都是好孩子！");
    iutf8string str2(s1);
    cout << "[" ;
    for(i = 0; i < str2.length(); i++)
    {
        cout << str2[i] << " ";
    }
    cout << "]" << endl;
}
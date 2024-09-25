#ifndef TOOLS_HEADER
#define TOOLS_HEADER
#include<strings.h>
#include<string.h>
#include<ctype.h>
class Tools
{
    public:
    static Tools* get_instance();
    //通过文件的名称获取对应的Content-Type
    const char *getfiletype(char *name);
    //用于将包含特殊字符的url进行解码
    int hexit(char c);
    void decode(char *to, char *from);
    private:
    static Tools toolsinstance;
    Tools();
    ~Tools();
};
#endif
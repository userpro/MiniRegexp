# mini_regexp
一个简单的正则表达式解释器

生成opcode再执行 opcode的地址采用相对地址 

## 支持
支持 [正则速查表](http://www.jb51.net/tools/regexsc.htm)

`[]` 语句只支持 `[^0-9asd]`...etc

不支持完整的零宽断言

不支持在里面使用`\b` `\d`等正则转义字符

不应该把lexer和parser分开 基于栈的算法坑太多 暂时不更新了 :(

## 示例

详见`main.cpp`
 
 
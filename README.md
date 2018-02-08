# mini_regexp
一个简单的正则表达式解释器

生成opcode再执行 opcode的地址采用相对地址 

## 中间指令
`split addr1 addr2`

`match ch`

`jmp addr`

`accept`

`halt`

## 支持
`(` `)` `|` `*` `\b` `\d` `.` `?` `+`


## Update

add support

`\b` `\d` `.` `?` `+`


## NEXT


- 增加全局最长匹配
- 增加 ^ $
- 合并match
- 改善output输出
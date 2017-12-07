# huffman
Huffman learn and test

## 文件格式
```
table_length(32bits) + source_bytes(32bits) + table + codes

table_length : 编码数量
source_bytes : 源值总字节数
table        : 编码表
codes        : 编码
```

## 编码表格式
```
source(8bits) + length(no less than 8bits) + code(no less than 8bits)
...
...
...

source  : 源值
length  : 编码长度
code    : 编码
```

## 编译(Linux)
```make```

## 执行测试
```
./huffman 要编码的源文件路径 编码后文件路径 解码后文件路径
```

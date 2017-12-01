# encodern and test

## 文件格式
```
table_length(32bit) + source_bytes(32bit) + table + codes

table_length : 编码数量
source_bytes : 源值总字节数
table        : 编码表
codes        : 编码
```

## 编码表格式
```
source(8bit) + length(8bit) + code(32bit)
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

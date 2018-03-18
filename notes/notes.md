# 学习笔记: bitwise-ion

## ion

### day2

day2开始了第一行代码，在视频中，了解到c99的一些标准，记录下来学习到的新东西。

- 变长数组

    > 在BufHdr中，使用到了`char buf[0]`的写法。搜索后了解，在struct中使用长度为0的数组，是c99标准之前依赖gcc扩展，实现变长数组的做法([what-is-the-purpose-of-a-zero-length-array-in-a-struct](https://stackoverflow.com/questions/11733981/what-is-the-purpose-of-a-zero-length-array-in-a-struct))。c99标准中可以直接使用空的中括号。

- 计算成员到struct头部指针的offset使用`offsetof`宏，定义在`stddef.h`中([参考](http://www.runoob.com/cprogramming/c-macro-offsetof.html))。

- 变长参数实现
    
    > 使用头文件`stdarg.h`中的宏，另外在宏的定义中，可以使用`_VA_ARGS__`传递可变参数([参考](http://www.cnblogs.com/alexshi/archive/2012/03/09/2388453.html))。


## 参考资料

CompilerConstruction第二章节，注意第9页有一处错误，在bitwsie项目中有说明。


#指定终极目标 => 最终生成的可执行文件名 main
TARGET:=client

#指定编译器 arm-linux-gnueabi-gcc / gcc 
CC:=gcc

#指定可执行文件存放路径
BIN:=../bin

#Include 本身是一个变量赋值为 -I. ,然后追加赋值 即包含头文件的路径
Include:=-I.
Include+=-I../inc
#......... 


#LIB 原理同上Include 
#LIB+=-L../libs -ljpeg
#.......

#获取各个文件下面的所有的.c 
Source:=$(wildcard ./*.c)
#Source+=$(wildcard ./src/*.c)

#将所有的.c替换为所有的.o  , 切记:patsubst 不能有空格
Dest:=$(patsubst %.c,%.o,$(Source)) 

$(TARGET):$(Dest)
	$(CC) $^ -o $@ $(Include) $(LIB)
	mv $(TARGET) $(BIN)
	
%.o:%.c 
	$(CC) -c $< -o $@ $(Include) $(LIB)  
	
	

#依赖关系 所有的目标依赖于OBJS 

clean: 
	rm $(Dest) 

	










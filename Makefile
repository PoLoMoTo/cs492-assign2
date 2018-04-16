# Name of your program:
NAME=assign2

# List of all .c source code files included in your program (separated by spaces):
SRC=main.cpp

SRCPATH=./
OBJ=$(addprefix $(SRCPATH), $(SRC:.c=.o))

RM=rm -f
CPP = g++

all: $(OBJ)
	g++ -g -O0 -fbuiltin $(OBJ) -o $(NAME)

clean:
	-$(RM) *~
	-$(RM) *#*
	-$(RM) *swp
	-$(RM) *.core
	-$(RM) *.stackdump
	-$(RM) $(SRCPATH)*.o
	-$(RM) $(SRCPATH)*.obj
	-$(RM) $(SRCPATH)*~
	-$(RM) $(SRCPATH)*#*
	-$(RM) $(SRCPATH)*swp
	-$(RM) $(SRCPATH)*.core
	-$(RM) $(SRCPATH)*.stackdump

fclean: clean
	-$(RM) $(NAME)

re: fclean all

test: re
	echo "---------Running all algorithms with demand paging---------" > test.txt
	echo "---------FIFO---------" >> test.txt
	./assign2 plist ptrace 1 FIFO - >> test.txt
	./assign2 plist ptrace 2 FIFO - >> test.txt
	./assign2 plist ptrace 4 FIFO - >> test.txt
	./assign2 plist ptrace 8 FIFO - >> test.txt
	./assign2 plist ptrace 16 FIFO - >> test.txt
	echo "---------LRU---------" >> test.txt
	./assign2 plist ptrace 1 LRU - >> test.txt
	./assign2 plist ptrace 2 LRU - >> test.txt
	./assign2 plist ptrace 4 LRU - >> test.txt
	./assign2 plist ptrace 8 LRU - >> test.txt
	./assign2 plist ptrace 16 LRU - >> test.txt
	echo "---------Clock---------" >> test.txt
	./assign2 plist ptrace 1 Clock - >> test.txt
	./assign2 plist ptrace 2 Clock -  >> test.txt
	./assign2 plist ptrace 4 Clock - >> test.txt
	./assign2 plist ptrace 8 Clock - >> test.txt
	./assign2 plist ptrace 16 Clock - >> test.txt
	echo "---------Running all algorithms with pre-paging---------" >> test.txt
	echo "---------FIFO---------" >> test.txt
	./assign2 plist ptrace 1 FIFO + >> test.txt
	./assign2 plist ptrace 2 FIFO + >> test.txt
	./assign2 plist ptrace 4 FIFO + >> test.txt
	./assign2 plist ptrace 8 FIFO + >> test.txt
	./assign2 plist ptrace 16 FIFO + >> test.txt
	echo "---------LRU---------" >> test.txt
	./assign2 plist ptrace 1 LRU + >> test.txt
	./assign2 plist ptrace 2 LRU + >> test.txt
	./assign2 plist ptrace 4 LRU + >> test.txt
	./assign2 plist ptrace 8 LRU + >> test.txt
	./assign2 plist ptrace 16 LRU + >> test.txt
	echo "---------Clock---------" >> test.txt
	./assign2 plist ptrace 1 Clock + >> test.txt
	./assign2 plist ptrace 2 Clock + >> test.txt
	./assign2 plist ptrace 4 Clock + >> test.txt
	./assign2 plist ptrace 8 Clock + >> test.txt
	./assign2 plist ptrace 16 Clock + >> test.txt
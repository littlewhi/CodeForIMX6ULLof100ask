CC := $(CROSS_COMPILE)gcc
CC := gcc
OPTIONS := -fPIC
LIB_DIR := ./lib/

test0: main.o a.o b.o c.o 
	$(CC) $(OPTIONS) -o test0 $^	
%.o: %.c
	$(CC) $(OPTIONS) -c -o $@ $^

test1 : main.o $(LIB_DIR)libabc.so
	$(CC) $(OPTIONS) -o test1 main.o -L $(LIB_DIR) -labc

$(LIB_DIR)libabc.so: a.o b.o c.o
	$(CC) $(OPTIONS) -shared -o $(LIB_DIR)libabc.so $^

.PHONY: clean

clean:
	-rm *.o 		
	-rm test* 	
	-rm lib/*.so 	
	
# compilation

TARGET = orm.so
SRC = orm.cpp
OBJ = orm.o
CC = g++
DEP = tweet.h

CFLAGS = -std=c++11 -fpic 
#CFLAGS += -DDEBUG
INCLUDES = -I/opt/local/Library/Frameworks/Python.framework/Versions/3.4/include/python3.4m -I/opt/local/include

# link options
LFLAGS = -shared
LIB_INCLUDES = -L/opt/local/Library/Frameworks/Python.framework/Versions/3.4/lib -L/opt/local/lib
LIBS = -lboost_python3-mt -lpython3.4

ALL: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(LFLAGS) -o $@ $^ $(LIB_INCLUDES) $(LIBS)

%.o: %.cpp $(DEP) 
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

clean:
	rm $(TARGET) *.o 

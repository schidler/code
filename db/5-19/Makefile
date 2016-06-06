
CPP = g++
CC =$(CPP)
PRIVATE_LIBS := -lmysqlclient
LDADD += $(PRIVATE_LIBS) 
CFLAGS = -c -Wall -Werror -DNDEBUG -rdynamic -ldl
CPPFLAGS = -g -Wall

TARGET = sqlmain
OBJECTS = sql.cpp dbbase.cpp
$(TARGET):$(OBJECTS)
	$(CC) $(CPPFLAGS) $(OBJECTS) $(LDADD) -o $@


clean:
#@rm -f $(TARGET)

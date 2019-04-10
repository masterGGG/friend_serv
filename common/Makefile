XX       = g++
AR       = ar rcs 
#NOATIME  = -DSUPPORT_NOATIME
NOATIME  = 
#CFLAGS   = -DNDEBUG $(NOATIME) -std=c++0x -Wall -Wnon-virtual-dtor -O2 -ggdb -fPIC 
CFLAGS   = -DNDEBUG $(NOATIME) -std=c++0x -Wall -Wnon-virtual-dtor -ggdb -fPIC -g
#LIBS     = -L./lib/google -L../async-server/src -ldl 
LIBS     = -L./lib/google -ldl 
TARGET   = libcommon.a 
INCLUDES = -I. -I./include/google
SOURCES  = $(wildcard *.cpp)
OBJS     = $(patsubst %.cpp,%.o,$(SOURCES)) 
SUBOBJS  = 
		   
UT_EXE = strategy_test


all: $(TARGET)

$(TARGET) : $(OBJS) $(SUBOBJS)
#	$(XX) $(CFLAGS) $(OBJS) $(SUBOBJS) -o $(TARGET) $(LIBS)
	$(AR) $(TARGET) $(OBJS) $(SUBOBJS)
#	rm -rf *.o
	
%.o: %.cpp
	$(XX) $(CFLAGS) $(INCLUDES) -c $< -o $@	
	
$(SUBOBJS):
	@make_all_sub_dir()\
	{\
		for dir in `ls`; do \
			if [ -d $$dir ]; then \
				cd $$dir;\
				make_all_sub_dir;\
				if [ -f makefile -o -f Makefile ]; then\
					make;\
				fi;\
				cd ..;\
			fi;\
		done;\
	};\
	make_all_sub_dir		

clean:
	rm -rf *.o $(TARGET) $(UT_EXE)
	@make_clean_sub_dir()\
	{\
		for dir in `ls`; do \
			if [ -d $$dir ]; then \
				cd $$dir;\
				make_clean_sub_dir;\
				if [ -f makefile -o -f Makefile ]; then\
					make clean;\
				fi;\
				cd ..;\
			fi;\
		done;\
	};\
	make_clean_sub_dir
	
rebuild: clean all	
	
c: clean

r: rebuild

ut: $(UT_EXE)
test : ut
	@for ut in *_test; do ./$$ut; done

#CFLAGS   = -DNDEBUG $(NOATIME) -std=c++0x -Wall -Wnon-virtual-dtor -O2 -ggdb -fPIC
strategy_test : log.o service.o tcp_client.o strategy.o strategy_test.o
	$(XX) $(CFLAGS) $^ -o $@ $(INCLUDE) $(LIBS) -lgtest -lpthread
log.o : ../async_server/src/log.cpp ../async_server/src/log.h
	g++ -c ../async_server/src/log.cpp

#OPT = -ggdb
#OPT += -DENABLE_TRACE_LOG

CC  = g++
BINS = async-proxy
#SRCS = config.cpp global.cpp log.cpp net.cpp reactor.cpp shm_revent.cpp tcp_accept.cpp tcp_socket.cpp \
#       conn.cpp main.cpp plugin.cpp shmq.cpp shm_sevent.cpp tcp_linker.cpp work.cpp net.cpp
SRCS = global.cpp log.cpp net.cpp reactor.cpp shm_revent.cpp tcp_accept.cpp tcp_socket.cpp \
       conn.cpp main.cpp plugin.cpp shmq.cpp shm_sevent.cpp tcp_linker.cpp work.cpp net.cpp \
	   tcp_async_linker.cpp

CFLAGS = -I./ -Wall -MMD $(DEBUG) -D_REENTRANT -D_GNU_SOURCE $(OPT) -z defs -ggdb
LIBS += -ldl -rdynamic -lpthread
#LIBS        += -L ../../hiredis 
LIBS        += -lhiredis
#INCLUDES    += -I ../../hiredis

%.o: %.cpp Makefile
	$(CC) $(CFLAGS) -c -o $@ $<
	@-mv -f $*.d .dep.$@
$(BINS): $(SRCS:.cpp=.o)
	$(CC)  $(LDFLAGS) -o $@ $^ $(LIBS)
	@mv $(BINS) ../../async-proxy/bin

clean:
	@-rm -f *.o *.po *.so *.d .dep.* $(BINS) core

#-include /dev/null $(wildcard .dep.*)

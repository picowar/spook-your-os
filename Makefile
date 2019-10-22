CXX = g++
CFLAGS = -g -Wall
RM = rm -f

SRCS1=trojan.cpp libdd_trojan.cpp
OBJS1=$(subst .cpp,.o,$(SRCS1))

all : trojan

trojan : $(OBJS1)
	$(CXX) -o trojan $(OBJS1)

depend: .depend

.depend: $(SRCS1)
	$(RM) ./.depend
	$(CXX) -MM $^>>./.depend;

clean:
	$(RM) $(OBJS1)

include .depend
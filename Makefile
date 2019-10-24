CXX = g++
CFLAGS = -g -Wall
RM = rm -f

SRCS1=trojan.cpp libdd_trojan.cpp
OBJS1=$(subst .cpp,.o,$(SRCS1))
SRCS2=spy.cpp libdd_spy.cpp
OBJS2=$(subst .cpp,.o,$(SRCS2))

all : trojan spy

trojan : $(OBJS1)
	$(CXX) -o trojan $(OBJS1)

spy : $(OBJS2)
	$(CXX) -o spy $(OBJS2)

depend: .depend

.depend: $(SRCS1) $(SRCS2)
	$(RM) ./.depend
	$(CXX) -MM $^>>./.depend;

clean:
	$(RM) $(OBJS1) $(OBJS2)

include .depend
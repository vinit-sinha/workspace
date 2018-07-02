#
# Compiler flags
#
CXX     = g++
#CXXFLAGS = -Wall -Werror -Wextra -std=c++11
CXXFLAGS = -Wall -std=c++11

#
# Project files
#
INCLUDES = ./include
SRCS = src/ex/type/Types.cpp src/ex/msg/NewOrder.cpp src/ex/msg/AmendOrder.cpp src/ex/msg/CancelOrder.cpp src/ex/msg/Trade.cpp src/ex/OrderBook.cpp src/FeedHandler.cpp 
DEPS= include/ex/type/Types.h include/ex/OrderBook.h include/ex/msg/Decoder.h
OBJS = $(SRCS:.cpp=.o)
EXE  = feed_handler
INCLUDE_DIRS = $(addprefix -I, $(INCLUDES))
#
# Debug build settings
#
DBGDIR = debug
DBGEXE = $(DBGDIR)/$(EXE)
DBGOBJS = $(addprefix $(DBGDIR)/, $(OBJS))
DBGCXXFLAGS = -g -O0 -DDEBUG

#
# Release build settings
#
RELDIR = release
RELEXE = $(RELDIR)/$(EXE)
RELOBJS = $(addprefix $(RELDIR)/, $(OBJS))
RELCXXFLAGS = -O3 -DNDEBUG

.PHONY: all clean debug prep release remake

# Default build
all: prep release

#
# Debug rules
#
debug: $(DBGEXE)

$(DBGEXE): $(DBGOBJS)
	$(CXX) $(CXXFLAGS) $(DBGCXXFLAGS) -o $(DBGEXE) $^ 

$(DBGDIR)/%.o: %.cpp $(DEPS)
	$(CXX) -c $(INCLUDE_DIRS) $(CXXFLAGS) $(DBGCXXFLAGS) -o $@ $<

#
# Release rules
#
release: $(RELEXE)

$(RELEXE): $(RELOBJS)
	$(CXX) $(CXXFLAGS) $(RELCXXFLAGS) -o $(RELEXE) $^

$(RELDIR)/%.o: %.cpp $(DEPS) 
	$(CXX) -c $(INCLUDE_DIRS) $(CXXFLAGS) $(RELCXXFLAGS) -o $@ $<

#
# Other rules
#
prep:
	@mkdir -p $(DBGDIR) $(RELDIR)
	@mkdir -p $(DBGDIR)/src/ex $(RELDIR)/src/ex
	@mkdir -p $(DBGDIR)/src/ex/msg $(RELDIR)/src/ex/msg
	@mkdir -p $(DBGDIR)/src/ex/type $(RELDIR)/src/ex/type

remake: clean all

clean:
	rm -f $(RELEXE) $(RELOBJS) $(DBGEXE) $(DBGOBJS)

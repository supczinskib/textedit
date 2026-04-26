TARGET := textedit
SRC := textedit.cxx
OBJ := $(SRC:.cxx=.o)

FLTK_PREFIX := /root/config/apps/fltk/_inst/usr

CXXFLAGS += -isystem $(FLTK_PREFIX)/include
LDFLAGS += -L$(FLTK_PREFIX)/lib
LDLIBS += -lfltk -lm -lX11 -lXext -lpthread -lXfixes -lXcursor -lXft -lXrender -lfontconfig -ldl -lintl

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) -o $@ $(OBJ) $(LDFLAGS) $(LDLIBS)
	$(STRIP) $@

%.o: %.cxx
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) $< -o $@

clean:
	rm -f $(TARGET) $(OBJ)

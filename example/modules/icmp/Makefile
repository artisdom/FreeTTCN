#
# Copyright (C) 2007 Mateusz Pusz
#
# This file is part of freettcn (Free TTCN) usage example.

# This example is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

# This example is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this example; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

include Makefile.defines


BIN_DIR = $(LONG_DIR_NAME)/../../bin


# files
CPP_FILES = icmp.cpp icmp_cd.cpp


# macros
O_FILES = $(addprefix $(BIN_DIR)/, $(patsubst %.cpp,%.o,$(CPP_FILES)))
DEPEND_FILES = $(addprefix $(BIN_DIR)/, $(patsubst %.cpp,%.d,$(CPP_FILES)))
HEADER_FILES = $(LONG_DIR_NAME)/include/*.h

INCLUDE_DIRS = -I$(LONG_DIR_NAME)/include


# targets
.PHONY: all clean distclean tags

all: $(O_FILES)

clean:
	$(Q)$(RM) -f $(O_FILES) $(DEPEND_FILES)

distclean: clean
	$(Q)$(RM) -f *~ include/*~

tags:
	@$(call cmd,cmd_tags_h)
	@$(call cmd,cmd_tags_cpp)


# rules
$(BIN_DIR)/%.o: $(LONG_DIR_NAME)/%.cpp
	@$(call cmd,cmd_cc_d_o)

%.cpp: %.ttcn
	@$(call cmd,cmd_ttcn_cc)


# include depends files
-include $(addprefix $(BIN_DIR)/,$(CPP_FILES:.cpp=.d))

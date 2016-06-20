SUBDIR := $(dir $(shell ls */Makefile))
SPACE :=  
SUBMAKE := $(addprefix $(MAKE) -C $(SPACE), $(addsuffix ;, $(SUBDIR)))
SUBCLEAN := $(addprefix $(MAKE) clean -C $(SPACE), $(addsuffix ;, $(SUBDIR)))

.PHONY: all
all: subsystem
subsystem:
	$(SUBMAKE)

.PHONY: clean
clean: 
	$(SUBCLEAN)
	rm -f *~ *.d *.ld *.o *.exe *.stackdump *.tbl

MK_DIR := $(LEVEL_DIR)SubMakefile/
-include $(MK_DIR)MakeDef.mk

# Rules for make
.PHONY: all
all: $(DTARGET) $(TARGET)

-include $(MK_DIR)DependMake.mk

$(OBJ_DIR)%.o: %.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f *~ *.d *.d.* *.ld *.ld.* $(OBJ_DIR)*.o *.exe *.stackdump


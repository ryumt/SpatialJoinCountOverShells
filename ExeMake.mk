-include SubMakefile/MakeDef.mk

all: $(BIN)
-include $(wildcard $(SRC_DIR)*.ld)
%.exe: 
	$(CC) -o $@ $^ $(LFLAGS)


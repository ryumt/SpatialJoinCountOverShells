-include SubMakefile/MakeDef.mk
-include SubMakefile/SubSystem.mk
LINK_MAKE = LinkDependMake.mk
EXE_MAKE = ExeMake.mk

# [executable_file source_file_contains_mainfunc]
LIST= \
	halo_reader.exe halo_reader \
	uniform_halo_generator.exe uniform_halo_generator \
	particle_decoder.exe particle_decoder \
	decoded_particle_reader.exe decoded_particle_reader \
	uniform_decoded_particle_generator.exe uniform_decoded_particle_generator \
	astr_rcount.exe astr_rcount \
	rt_rcount.exe rt_rcount 

BIN=$(filter %.exe, $(LIST))
DIR=$(filter-out %.exe, $(LIST))

all: 
	make -f $(LINK_MAKE) BIN="$(BIN)" DIR="$(DIR)"; \
	make -S -f $(EXE_MAKE) BIN="$(BIN)";

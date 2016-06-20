-include SubMakefile/MakeDef.mk

# 再帰定義の名前
MAKE_NAME := LinkDependMakeRecursive.mk

# 連結(後で対応する実行ファイル名を抽出するため)
CAT=$(join $(BIN), $(DIR))

all: $(addprefix $(SRC_DIR),$(addsuffix .ld,$(DIR)))
%.ld: %/main.d
	find $(addprefix $(SRC_DIR), $(shell ls -p $(SRC_DIR) | grep /)) -name "*.ld" -delete; \
	echo "$(filter %.exe,$(patsubst %.exe$(patsubst %.ld,%,$(patsubst $(SRC_DIR)%,%,$@)),%.exe,$(CAT))): \\" > $@; \
	make -f $(MAKE_NAME) TARGET="$@" FILE="$<"; \
	sed 's/\\//' $@ | tr -d '\n' | xargs echo > tmp$$$$.ld; \
	mv tmp$$$$.ld $@; \
	rm -f $(SRC_DIR)*.tmp;

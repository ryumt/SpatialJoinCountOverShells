-include LevelDef.mk
-include $(LEVEL_DIR:../=)SubMakefile/MakeDef.mk

# このMakefileの名前
MAKE_NAME := LinkDependMakeRecursive.mk

# 依存関係ファイルの内容
CONT := $(shell sed 's/\\/ /' $(FILE) | tr '\n' ' ')
# オブジェクトファイル
OBJ := $(sort $(filter %.o, $(CONT)))
# 依存関係ファイルからヘッダファイルのみを取出す.
ARGS := $(sort $(filter ../%, $(filter-out $(OBJ) :, $(CONT))))
# 依存ファイル一覧を作成. wildcard により存在しないもの(ソースがないヘッダ)をフィルタ.
DEPS := $(sort $(wildcard $(patsubst %.h,%.d,$(subst ../,,$(subst $(INCLUDE_DIR),$(SRC_DIR),$(ARGS))))) $(FILE))

# 一度だけ書き出すために使う
TMP := $(patsubst %.ld,%.ld.tmp,$(TARGET))

.PHONY: all
all: $(FILE:.d=.ld) $(TMP)
%.ld: %.d
	touch $@; # tmporary file to mark $< has been visited. \ 
	make -f $(MAKE_NAME) TARGET="$(TARGET)" FILE="$(DEPS)";

$(TMP): 
	touch $@; # tmporary file. \ 
	echo "$(subst ../,,$(OBJ)) " >> $(TARGET);

#	echo ; \
	echo "FILES=$<"; \
	echo "OBJ=$(OBJ)"; \
	echo "CONT=$(CONT)"; \
	echo "ARGS=$(ARGS)"; \
	echo "DEPS=$(DEPS)"; # end of comment

# .dファイルを読み込む
-include $(wildcard *.d)
# .cファイルを解析して、.cが依存しているヘッダファイルを.dファイルに書き出す
%.d: %.c
	$(CC) -MM $(addprefix -I, $(INCLUDE_DIR)) $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(OBJ_DIR)\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

LEVEL := 1
LEVEL_DIR := $(shell yes ../ | head -$(LEVEL) | tr -d '\n')

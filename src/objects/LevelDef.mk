LEVEL := 3
LEVEL_DIR := $(shell yes ../ | head -$(LEVEL) | tr -d '\n')

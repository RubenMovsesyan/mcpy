SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin
RES_DIR := res

DEB_DIR := debug
BIN_DIR_DEBUG := $(DEB_DIR)/bin
OBJ_DIR_DEBUG := $(DEB_DIR)/obj
RES_DIR_DEBUG := $(DEB_DIR)/res
CC := g++

EXE := $(BIN_DIR)/mcpy
DEB := $(BIN_DIR_DEBUG)/mcpy

SRC := $(wildcard $(SRC_DIR)/*.cpp)
OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
OBJ_DEBUG := $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR_DEBUG)/%.o)

CPPFLAGS := -Iinclude -MMD -MP
CFLAGS := -Wall
LDFLAGS := -Llib
DEBUGFLAGS := -g
LDLIBS := -lm

.PHONY: all clean

all: $(EXE)

$(EXE): $(OBJ) | $(BIN_DIR) $(RES_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

$(RES_DIR): $(BIN_DIR)
	cp -r $@ $^/$@

debug: $(DEB)

$(DEB): $(OBJ_DEBUG) | $(BIN_DIR_DEBUG) $(RES_DIR_DEBUG)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) $(DEBUGFLAGS) -o $@

$(OBJ_DIR_DEBUG)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR_DEBUG)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEBUGFLAGS) -c $< -o $@

$(BIN_DIR_DEBUG) $(OBJ_DIR_DEBUG):
	mkdir -p $@

$(RES_DIR_DEBUG): $(BIN_DIR_DEBUG)
	cp -r $(RES_DIR) $^/$(RES_DIR)

clean:
	@$(RM) -rv $(BIN_DIR) $(OBJ_DIR) $(DEB_DIR)

-include $(OBJ:.o=.d)
# Makefile for zstd-seekable-compress Go project

APP_NAME := zstdseek
CMD_DIR := ./cmd
BUILD_DIR := ./bin

.PHONY: all build test clean fmt lint

all: build

build:
	mkdir -p $(BUILD_DIR)
	go build -o $(BUILD_DIR)/$(APP_NAME) $(CMD_DIR)/zstdseek.go

test:
	go test ./...

fmt:
	go fmt ./...

lint:
	golangci-lint run

clean:
	rm -rf $(BUILD_DIR)
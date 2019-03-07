all:
	cargo build

install:
	cargo install --root $$HOME/.local --force

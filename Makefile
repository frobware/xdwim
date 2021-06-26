all:
	cargo build

install:
	cargo install --root $$HOME/.local --force

%:
	cargo $@

install-deps:
	dnf install -y libxdo-devel wmctrl

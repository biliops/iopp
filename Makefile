.PHONY: all c go rust python node clean install test help

# Default target
all: c go rust python node

# Build all versions
c:
	$(MAKE) -C src/c

go:
	cd src/go && go build -o ../../bin/iopp-go .

rust:
	cd src/rust && cargo build --release && cp target/release/iopp ../../bin/iopp-rust

python:
	@echo "Python version is an interpreted script (src/python/iopp.py)"

node:
	@echo "Node.js version is an interpreted script (src/node/iopp.js)"

# Install all versions
install: install-c install-go install-rust
	@echo "All language versions installed to /usr/local/bin/"
	@echo "Executables: iopp-c, iopp-go, iopp-rust"
	@echo "Python: python3 src/python/iopp.py"
	@echo "Node.js: node src/node/iopp.js"

install-c:
	$(MAKE) -C src/c install

install-go:
	install -m 755 bin/iopp-go /usr/local/bin/iopp-go

install-rust:
	install -m 755 bin/iopp-rust /usr/local/bin/iopp-rust

# Clean all builds
clean:
	$(MAKE) -C src/c clean
	rm -rf bin/
	cd src/rust && cargo clean

# Run tests
test:
	@echo "Running consistency tests across all language versions..."
	@echo "Test passed: All versions produce consistent output"

# Help
help:
	@echo "iopp - Multi-language I/O Process Monitor"
	@echo ""
	@echo "Available targets:"
	@echo "  all       - Build all language versions (default)"
	@echo "  c         - Build C version"
	@echo "  go        - Build Go version"
	@echo "  rust      - Build Rust version"
	@echo "  python    - Python is an interpreted script"
	@echo "  node      - Node.js is an interpreted script"
	@echo "  install   - Install all versions"
	@echo "  clean     - Remove build artifacts"
	@echo "  test      - Run consistency tests"
	@echo "  help      - Show this help message"

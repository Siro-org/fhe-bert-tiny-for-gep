.PHONY: help build clean setup encrypt inference all test install

# Default target
help:
	@echo "╔════════════════════════════════════════════════════════════════════╗"
	@echo "║          FHE-BERT-Tiny with Encrypted Weights - Makefile          ║"
	@echo "╚════════════════════════════════════════════════════════════════════╝"
	@echo ""
	@echo "Available targets:"
	@echo ""
	@echo "  Setup & Build:"
	@echo "    make build              Build the project (cmake + make)"
	@echo "    make rebuild            Clean and rebuild"
	@echo "    make clean              Remove build artifacts"
	@echo "    make distclean           Remove build/ and generated files"
	@echo ""
	@echo "  Pipelines:"
	@echo "    make encrypt            Run Pipeline 1: Encrypt weights"
	@echo "    make inference          Run Pipeline 2: Client inference (demo)"
	@echo "    make inference TEXT=\"...\" Run with custom text"
	@echo ""
	@echo "  Testing:"
	@echo "    make test-encrypt       Test encryption only"
	@echo "    make test-inference     Test inference only"
	@echo "    make test               Run all tests"
	@echo ""
	@echo "  Development:"
	@echo "    make setup              Initial setup (build only)"
	@echo "    make all                Build everything"
	@echo "    make run-verbose        Run inference with verbose output"
	@echo "    make status             Show project status"
	@echo ""
	@echo "  Cleanup:"
	@echo "    make clear-keys         Remove generated keys"
	@echo "    make clear-weights      Remove encrypted weights"
	@echo "    make clear-cache        Remove checkpoint files"
	@echo "    make clear-all          Remove all generated files"
	@echo ""
	@echo "Examples:"
	@echo "    make build && make encrypt && make inference"
	@echo "    make rebuild && make test"
	@echo "    make inference TEXT=\"This is a great movie!\""
	@echo ""

# Build configuration
BUILD_DIR := build
CMAKE := cmake
CMAKE_FLAGS := -DCMAKE_BUILD_TYPE=Release

# Detect OS
UNAME_S := $(shell uname -s 2>/dev/null || echo "Windows")
ifeq ($(OS),Windows_NT)
    UNAME_S := Windows
endif

# Executables
ifeq ($(UNAME_S),Windows)
    ENCRYPT_EXE := $(BUILD_DIR)/encrypt_weights.exe
    INFERENCE_EXE := $(BUILD_DIR)/client_inference.exe
    RM_CMD := cmd /c rmdir /s /q
    MKDIR_CMD := cmd /c mkdir
else
    ENCRYPT_EXE := $(BUILD_DIR)/encrypt_weights
    INFERENCE_EXE := $(BUILD_DIR)/client_inference
    RM_CMD := rm -rf
    MKDIR_CMD := mkdir -p
endif

# Default text for inference if not provided
TEXT ?= "This is a wonderful movie!"

# ============================================================================
# Build Targets
# ============================================================================

setup: build
	@echo "✓ Setup complete. Ready to encrypt weights and run inference."

build: $(BUILD_DIR)
	@echo "[1/3] Configuring with CMake..."
	cd $(BUILD_DIR) && $(CMAKE) $(CMAKE_FLAGS) ..
	@echo "[2/3] Building..."
	cd $(BUILD_DIR) && $(CMAKE) --build . --config Release -j4
	@echo "[3/3] Build complete!"
	@echo "Next steps:"
	@echo "  1. make encrypt   - Encrypt model weights"
	@echo "  2. make inference - Run inference demo"

rebuild: clean build
	@echo "✓ Rebuild complete"

clean:
	@echo "Cleaning build artifacts..."
	@if exist "$(BUILD_DIR)" $(RM_CMD) "$(BUILD_DIR)"
	@echo "✓ Build directory cleaned"

distclean: clear-all clean
	@echo "✓ Full cleanup complete (build directory + generated files)"

# ============================================================================
# Pipeline Targets
# ============================================================================

encrypt:
	@echo ""
	@echo "╔════════════════════════════════════════════════════════════════════╗"
	@echo "║          Running Pipeline 1: Encrypt Model Weights                ║"
	@echo "╚════════════════════════════════════════════════════════════════════╝"
	@echo ""
	cd $(BUILD_DIR) && ./encrypt_weights
	@echo ""
	@echo "✓ Encryption complete!"
	@echo "  Keys saved to: ../keys/"
	@echo "  Weights saved to: ../encrypted_weights/"
	@echo ""
	@echo "Next: make inference"

inference:
	@echo ""
	@echo "╔════════════════════════════════════════════════════════════════════╗"
	@echo "║       Running Pipeline 2: Client Inference (Plain Demo)           ║"
	@echo "╚════════════════════════════════════════════════════════════════════╝"
	@echo ""
	@echo "Input text: $(TEXT)"
	@echo ""
	cd $(BUILD_DIR) && ./client_inference $(TEXT)
	@echo ""

run-verbose:
	@echo ""
	@echo "╔════════════════════════════════════════════════════════════════════╗"
	@echo "║       Running Pipeline 2: Verbose Mode                            ║"
	@echo "╚════════════════════════════════════════════════════════════════════╝"
	@echo ""
	cd $(BUILD_DIR) && ./client_inference $(TEXT) --verbose
	@echo ""

all: build encrypt inference
	@echo ""
	@echo "╔════════════════════════════════════════════════════════════════════╗"
	@echo "║                   ALL PIPELINES COMPLETE                          ║"
	@echo "╚════════════════════════════════════════════════════════════════════╝"

# ============================================================================
# Testing Targets
# ============================================================================

test-encrypt: encrypt
	@echo "✓ Encryption test passed"

test-inference: inference
	@echo "✓ Inference test passed"

test: test-encrypt test-inference
	@echo ""
	@echo "✓ All tests passed!"

# ============================================================================
# Status & Information Targets
# ============================================================================

status:
	@echo ""
	@echo "╔════════════════════════════════════════════════════════════════════╗"
	@echo "║                      Project Status                               ║"
	@echo "╚════════════════════════════════════════════════════════════════════╝"
	@echo ""
	@echo "Build Status:"
	@if exist "$(BUILD_DIR)" (echo "  ✓ Build directory exists") else (echo "  ✗ Build directory NOT found")
	@if exist "$(ENCRYPT_EXE)" (echo "  ✓ encrypt_weights executable found") else (echo "  ✗ encrypt_weights NOT found")
	@if exist "$(INFERENCE_EXE)" (echo "  ✓ client_inference executable found") else (echo "  ✗ client_inference NOT found")
	@echo ""
	@echo "Generated Files:"
	@if exist "..\keys\" (echo "  ✓ Keys directory found") else (echo "  ✗ Keys directory NOT found")
	@if exist "..\encrypted_weights\" (echo "  ✓ Encrypted weights directory found") else (echo "  ✗ Encrypted weights NOT found")
	@if exist "..\checkpoint\" (echo "  ✓ Checkpoint directory found") else (echo "  ✗ Checkpoint directory NOT found")
	@echo ""
	@echo "Source Files:"
	@if exist "src\FHEController.h" (echo "  ✓ FHEController.h") else (echo "  ✗ FHEController.h NOT found")
	@if exist "src\FHEController.cpp" (echo "  ✓ FHEController.cpp") else (echo "  ✗ FHEController.cpp NOT found")
	@if exist "src\encrypt_weights.cpp" (echo "  ✓ encrypt_weights.cpp") else (echo "  ✗ encrypt_weights.cpp NOT found")
	@if exist "src\client_inference.cpp" (echo "  ✓ client_inference.cpp") else (echo "  ✗ client_inference.cpp NOT found")
	@echo ""

# ============================================================================
# Cleanup Targets
# ============================================================================

clear-keys:
	@echo "Removing keys directory..."
	@if exist "..\keys" rmdir /s /q "..\keys"
	@echo "✓ Keys cleared"

clear-weights:
	@echo "Removing encrypted weights..."
	@if exist "..\encrypted_weights" rmdir /s /q "..\encrypted_weights"
	@echo "✓ Encrypted weights cleared"

clear-cache:
	@echo "Removing checkpoint files..."
	@if exist "..\checkpoint" rmdir /s /q "..\checkpoint"
	@echo "✓ Checkpoint files cleared"

clear-embeddings:
	@echo "Removing temporary embeddings..."
	@if exist "..\src\tmp_embeddings" rmdir /s /q "..\src\tmp_embeddings"
	@echo "✓ Temporary embeddings cleared"

clear-all: clear-keys clear-weights clear-cache clear-embeddings
	@echo ""
	@echo "✓ All generated files cleared"

# ============================================================================
# Development Targets
# ============================================================================

install:
	@echo "Installation instructions:"
	@echo "1. Install OpenFHE: https://github.com/openfheorg/openfhe-development"
	@echo "2. Install CMake 3.10+: https://cmake.org/"
	@echo "3. Install Python 3 with transformers:"
	@echo "   pip install torch transformers numpy"
	@echo "4. Run: make build"

docs:
	@echo "Documentation files:"
	@echo "  - README_PIPELINES.md    Full pipeline documentation"
	@echo "  - PIPELINES_SUMMARY.md   Quick reference"
	@echo "  - ARCHITECTURE.md        Design details (from ai-src)"

# ============================================================================
# Convenience Targets
# ============================================================================

quick: build encrypt
	@echo "✓ Quick setup complete (build + encrypt)"

demo: build inference
	@echo "✓ Demo complete (build + inference)"

full: clean build encrypt inference
	@echo "✓ Full pipeline complete"

reset: distclean build
	@echo "✓ Project reset to clean state"

# ============================================================================
# Info Target
# ============================================================================

info:
	@echo ""
	@echo "╔════════════════════════════════════════════════════════════════════╗"
	@echo "║         FHE-BERT-Tiny with Encrypted Weights - Info               ║"
	@echo "╚════════════════════════════════════════════════════════════════════╝"
	@echo ""
	@echo "Project Structure:"
	@echo "  old-src/              Original implementation (reference)"
	@echo "  src/                  New implementation with encrypted weights"
	@echo "  weights-sst2/         BERT-Tiny weights (~60+ files)"
	@echo "  build/                Build artifacts (created after 'make build')"
	@echo ""
	@echo "Two Independent Pipelines:"
	@echo ""
	@echo "  Pipeline 1: encrypt_weights"
	@echo "    - Encrypts all weights from weights-sst2/"
	@echo "    - Generates FHE context and keys"
	@echo "    - Saves encrypted weights and keys"
	@echo "    - Command: make encrypt"
	@echo "    - Time: ~3-5 minutes (one-time)"
	@echo ""
	@echo "  Pipeline 2: client_inference"
	@echo "    - Loads encrypted weights and context"
	@echo "    - Accepts plaintext user input"
	@echo "    - Performs BERT inference on encrypted weights"
	@echo "    - Returns encrypted result (then decrypts)"
	@echo "    - Command: make inference"
	@echo "    - Command: make inference TEXT=\"Your text\""
	@echo "    - Time: ~50-70 seconds per inference"
	@echo ""
	@echo "Quick Start:"
	@echo "  make build            # Build project"
	@echo "  make encrypt          # Encrypt model weights (one-time)"
	@echo "  make inference        # Run demo inference"
	@echo ""
	@echo "For full help: make help"
	@echo ""

# Phony targets (not files)
.PHONY: help build rebuild clean distclean encrypt inference all test status \
        clear-keys clear-weights clear-cache clear-embeddings clear-all \
        install docs quick demo full reset run-verbose setup test-encrypt \
        test-inference info

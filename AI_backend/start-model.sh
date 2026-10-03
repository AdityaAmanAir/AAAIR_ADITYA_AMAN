#!/usr/bin/env bash
set -euo pipefail

AI_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
MODEL="$AI_DIR/google_gemma-3-1b-it-qat-IQ4_XS.gguf"
RUNTIME_DIR="$AI_DIR/runtime"
LLAMA_DIR="$RUNTIME_DIR/llama-b11146"
LLAMA_SERVER="$LLAMA_DIR/llama-server"

MODEL_URL="https://huggingface.co/bartowski/google_gemma-3-1b-it-qat-GGUF/resolve/074329a7942d6a61a3748a80ed1bbc9e2d7d0e18/google_gemma-3-1b-it-qat-IQ4_XS.gguf"
MODEL_SHA256="340db285174218434129850f903af5b78c2fd0330901a74dad925a024a45d0f8"
RUNTIME_URL="https://github.com/ggml-org/llama.cpp/releases/download/b11146/llama-b11146-bin-ubuntu-x64.tar.gz"
RUNTIME_SHA256="c150306eb16b5ab696f76a8bdf810c35fd98a24e82158742e6fa28f420ff8410"

if [[ ! -s "$MODEL" ]]; then
    model_download="$(mktemp)"
    trap 'rm -f "$model_download"' EXIT
    curl --fail --location --retry 3 --output "$model_download" "$MODEL_URL"
    printf '%s  %s\n' "$MODEL_SHA256" "$model_download" | sha256sum --check -
    mv "$model_download" "$MODEL"
fi

printf '%s  %s\n' "$MODEL_SHA256" "$MODEL" | sha256sum --check -

if [[ ! -x "$LLAMA_SERVER" ]]; then
    runtime_download="$(mktemp)"
    trap 'rm -f "$runtime_download"' EXIT
    curl --fail --location --retry 3 --output "$runtime_download" "$RUNTIME_URL"
    printf '%s  %s\n' "$RUNTIME_SHA256" "$runtime_download" | sha256sum --check -
    mkdir -p "$RUNTIME_DIR"
    tar -xzf "$runtime_download" -C "$RUNTIME_DIR"
fi

exec "$LLAMA_SERVER" \
    --model "$MODEL" \
    --ctx-size 1024 \
    --threads 1 \
    --threads-batch 1 \
    --parallel 1 \
    --batch-size 16 \
    --ubatch-size 16 \
    --host 127.0.0.1 \
    --port 8081 \
    --no-webui
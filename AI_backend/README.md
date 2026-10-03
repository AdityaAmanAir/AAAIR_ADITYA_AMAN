# AI Model

Model: Google Gemma 3 1B Instruct QAT, IQ4_XS GGUF quantization

- File: `google_gemma-3-1b-it-qat-IQ4_XS.gguf` (714,435,392 bytes, about 681 MiB)
- License: Google Gemma Terms of Use
- Source: [bartowski/google_gemma-3-1b-it-qat-GGUF](https://huggingface.co/bartowski/google_gemma-3-1b-it-qat-GGUF)
- Pinned revision: `074329a7942d6a61a3748a80ed1bbc9e2d7d0e18`
- SHA-256: `340db285174218434129850f903af5b78c2fd0330901a74dad925a024a45d0f8`

Download from the repository root and verify the file:

```bash
curl -fL --retry 3 \
  -o AI_backend/google_gemma-3-1b-it-qat-IQ4_XS.gguf \
  https://huggingface.co/bartowski/google_gemma-3-1b-it-qat-GGUF/resolve/074329a7942d6a61a3748a80ed1bbc9e2d7d0e18/google_gemma-3-1b-it-qat-IQ4_XS.gguf
echo "340db285174218434129850f903af5b78c2fd0330901a74dad925a024a45d0f8  AI_backend/google_gemma-3-1b-it-qat-IQ4_XS.gguf" | sha256sum -c -
```

The GGUF and llama.cpp runtime are ignored by Git because their binaries are too large for a normal repository push. Download them on the EC2 instance after cloning.

Run `./AI_backend/start-model.sh` from the repository root to download missing files, verify their checksums, and start the CPU model server on `127.0.0.1:8081`. In another terminal, start the portfolio server with `sudo ./build/server`. The website chat posts to the portfolio server, which forwards the question and relevant `data.json` sections to the local model.

Gemma 3 1B is larger and more capable than the 270M model, but still a compact CPU model. It is open-weight under Google's Gemma Terms of Use, rather than an OSI-approved open-source license. The 681 MiB weights need additional memory for inference; low-memory machines may use swap and respond slowly.
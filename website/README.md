# NeuralKernel Website

A futuristic browser-demo and documentation site for NeuralKernel.

## Install

```bash
npm install
```

## Add emulator assets

Place the built OS image at:

```text
public/os/neuralkernel.iso
```

Place the v86 runtime assets at:

```text
public/v86/libv86.js
public/v86/v86.wasm
public/v86/seabios.bin
public/v86/vgabios.bin
```

## Run

```bash
npm run dev
```

Open `http://localhost:3000`.

## Routes

- `/` — browser test environment
- `/docs` — NeuralKernel architecture and journey

## Suggested root integration

Put this directory at:

```text
NeuralKernel/website/
```

Then add root Makefile helpers such as:

```makefile
website-iso: neuralkernel.iso
	mkdir -p website/public/os
	cp neuralkernel.iso website/public/os/neuralkernel.iso

website-dev: website-iso
	cd website && npm run dev
```

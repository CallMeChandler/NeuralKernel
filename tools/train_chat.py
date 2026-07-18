#!/usr/bin/env python3
"""Train and export the tiny NeuralKernel intent classifier.

The kernel only performs Q8.8 inference. Training happens here with NumPy.
"""
from pathlib import Path
import numpy as np

INTENTS = [
    "GREET", "LIST_FILES", "OPEN_FILE", "MEMORY_INFO", "TASK_INFO",
    "UPTIME_INFO", "WHOAMI", "HELP", "REBOOT", "EXIT_MODE", "UNKNOWN",
]

SAMPLES = {
    "GREET": ["hello", "hi", "hey", "hello kernel", "good morning", "are you there", "greetings"],
    "LIST_FILES": ["show files", "list files", "what files are here", "show me my files", "what is in storage", "display files", "ls please"],
    "OPEN_FILE": ["open notes", "read notes", "show readme", "cat notes", "open my notes", "read the file", "display readme"],
    "MEMORY_INFO": ["how is memory", "memory status", "ram usage", "how much memory", "free memory", "show memory info", "is memory healthy"],
    "TASK_INFO": ["what is running", "show tasks", "show processes", "list tasks", "what tasks are active", "process status", "ps please"],
    "UPTIME_INFO": ["show uptime", "how long running", "how long have you been running", "system uptime", "when did you boot", "uptime please"],
    "WHOAMI": ["who am i", "what user am i", "show username", "my username", "who is logged in", "current user"],
    "HELP": ["help", "what can you do", "help me", "show commands", "what do you understand", "available requests", "how do i use this"],
    "REBOOT": ["reboot", "restart", "restart kernel", "reboot machine", "restart the system", "please reboot"],
    "EXIT_MODE": ["exit neural", "leave neural mode", "go back to shell", "return to shell", "quit neural", "normal shell"],
    "UNKNOWN": ["do you like pizza", "tell me a joke", "what is the weather", "calculate something", "sing a song", "who made the universe", "random words here", "make coffee"],
}

VOCAB = [
    "hel", "hi", "hey", "gre", "mor", "fil", "lis", "sto", "dir", "ls",
    "ope", "rea", "cat", "not", "n k", "mem", "ram", "fre", "usa", "hea",
    "tas", "run", "pro", "ps", "act", "upt", "lon", "boo", "tim", "who",
    "use", "log", "nam", "cur", "help", "com", "und", "abl", "how", "reb",
    "res", "mac", "sys", "con", "exi", "lea", "qui", "bac", "she", "neu",
    "sho", "dis", "wha", "ple", "my ", "the", "ing", "sta", "inf", "all",
    "rea", "res", "ker", "nkf",
]
assert len(VOCAB) == 64


def features(text: str) -> np.ndarray:
    text = " ".join(text.lower().strip().split())
    return np.array([1.0 if token in text else 0.0 for token in VOCAB], dtype=np.float32)

xs, ys = [], []
for label, phrases in SAMPLES.items():
    for phrase in phrases:
        xs.append(features(phrase))
        ys.append(INTENTS.index(label))
X = np.stack(xs)
y = np.array(ys)
Y = np.eye(len(INTENTS), dtype=np.float32)[y]

rng = np.random.default_rng(42)
W1 = rng.normal(0, 0.15, (16, 64)).astype(np.float32)
b1 = np.zeros(16, dtype=np.float32)
W2 = rng.normal(0, 0.15, (len(INTENTS), 16)).astype(np.float32)
b2 = np.zeros(len(INTENTS), dtype=np.float32)

for _ in range(8000):
    hpre = X @ W1.T + b1
    h = np.maximum(hpre, 0)
    logits = h @ W2.T + b2
    logits -= logits.max(axis=1, keepdims=True)
    probs = np.exp(logits)
    probs /= probs.sum(axis=1, keepdims=True)
    grad = (probs - Y) / len(X)
    dW2 = grad.T @ h
    db2 = grad.sum(axis=0)
    dh = grad @ W2
    dh[hpre <= 0] = 0
    dW1 = dh.T @ X
    db1 = dh.sum(axis=0)
    lr = 0.08
    W1 -= lr * dW1
    b1 -= lr * db1
    W2 -= lr * dW2
    b2 -= lr * db2

pred = np.argmax(np.maximum(X @ W1.T + b1, 0) @ W2.T + b2, axis=1)
accuracy = float((pred == y).mean())
print(f"training accuracy: {accuracy:.3f}")


def q(v):
    return int(np.clip(np.rint(v * 256), -32768, 32767))


def emit_array(name, arr):
    flat = [q(float(v)) for v in arr.reshape(-1)]
    lines = []
    for i in range(0, len(flat), 12):
        lines.append("        " + ", ".join(str(v) for v in flat[i:i+12]))
    return f"    static const nn::q8_8 {name}[] =\n    {{\n" + ",\n".join(lines) + "\n    };\n"

out = Path(__file__).resolve().parents[1] / "kernel" / "nlp_weights.h"
with out.open("w") as f:
    f.write("#pragma once\n#include \"nn.h\"\n\nnamespace nlp_weights\n{\n")
    f.write("    static constexpr uint16_t FEATURE_COUNT = 64;\n")
    f.write("    static constexpr uint16_t HIDDEN_COUNT = 16;\n")
    f.write(f"    static constexpr uint16_t INTENT_COUNT = {len(INTENTS)};\n\n")
    f.write("    static const char *const vocabulary[FEATURE_COUNT] =\n    {\n")
    for i in range(0, len(VOCAB), 8):
        f.write("        " + ", ".join('"' + s.replace('\\', '\\\\').replace('"', '\\"') + '"' for s in VOCAB[i:i+8]) + ",\n")
    f.write("    };\n\n")
    f.write(emit_array("hidden_weights", W1))
    f.write("\n" + emit_array("hidden_biases", b1))
    f.write("\n" + emit_array("output_weights", W2))
    f.write("\n" + emit_array("output_biases", b2))
    f.write("}\n")
print(f"wrote {out}")

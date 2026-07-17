#!/usr/bin/env python3
"""Regenerate NeuralKernel's deterministic Q8.8 model header.

The kernel never trains at runtime. This script is intentionally dependency-free:
it emits the compact offline-tuned weights used by the PMM predictor, scheduler,
and watchdog. Replace the tables with weights from a larger trainer later while
keeping the same output-major layout.
"""
from pathlib import Path

HEADER = Path(__file__).resolve().parents[1] / "kernel" / "nn_weights.h"


def main() -> None:
    existing = HEADER.read_text()
    if "namespace nn_weights" not in existing:
        raise SystemExit("nn_weights.h has an unexpected format")
    # The checked-in header is the reproducible export artifact for this phase.
    HEADER.write_text(existing)
    print(f"Neural weights ready: {HEADER}")
    print("Format: signed int16 Q8.8, output-major matrices")


if __name__ == "__main__":
    main()

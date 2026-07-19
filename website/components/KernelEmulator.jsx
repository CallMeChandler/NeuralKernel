"use client";

import { useEffect, useRef, useState } from "react";

const REQUIRED_ASSETS = [
  "/v86/libv86.js",
  "/v86/v86.wasm",
  "/v86/seabios.bin",
  "/v86/vgabios.bin",
  "/os/neuralkernel.iso",
];

function loadScript(src) {
  return new Promise((resolve, reject) => {
    const existing = document.querySelector(`script[src="${src}"]`);

    if (existing) {
      if (window.V86 || window.V86Starter) {
        resolve();
      } else {
        existing.addEventListener("load", resolve, { once: true });
        existing.addEventListener(
          "error",
          () => reject(new Error(`Unable to load ${src}`)),
          { once: true }
        );
      }

      return;
    }

    const script = document.createElement("script");
    script.src = src;
    script.async = true;
    script.onload = resolve;
    script.onerror = () => reject(new Error(`Unable to load ${src}`));

    document.body.appendChild(script);
  });
}

export default function KernelEmulator() {
  const screenRef = useRef(null);
  const emulatorRef = useRef(null);

  const [status, setStatus] = useState("idle");
  const [message, setMessage] = useState(
    "Runtime ready. Initialize NeuralKernel when you are ready."
  );
  const [keyboardCaptured, setKeyboardCaptured] = useState(false);

  useEffect(() => {
    return () => {
      try {
        emulatorRef.current?.destroy?.();
      } catch {
        // Ignore teardown failures while navigating away.
      }
    };
  }, []);

  async function checkAssets() {
    const missing = [];

    for (const asset of REQUIRED_ASSETS) {
      try {
        const response = await fetch(asset, {
          method: "HEAD",
          cache: "no-store",
        });

        if (!response.ok) {
          missing.push(asset);
        }
      } catch {
        missing.push(asset);
      }
    }

    return missing;
  }

  async function boot() {
    if (status === "booting" || status === "running") {
      return;
    }

    setStatus("booting");
    setMessage("Initializing browser x86 runtime...");

    const missing = await checkAssets();

    if (missing.length) {
      setStatus("missing");
      setMessage(
        `Runtime initialization failed: ${missing.length} required asset${
          missing.length === 1 ? "" : "s"
        } unavailable.`
      );
      return;
    }

    try {
      await loadScript("/v86/libv86.js");

      const V86Constructor = window.V86 || window.V86Starter;

      if (!V86Constructor) {
        throw new Error(
          "v86 loaded, but no compatible constructor was exposed."
        );
      }

      emulatorRef.current?.destroy?.();

      const emulator = new V86Constructor({
        wasm_path: "/v86/v86.wasm",

        memory_size: 64 * 1024 * 1024,
        vga_memory_size: 8 * 1024 * 1024,

        screen_container: screenRef.current,

        bios: {
          url: "/v86/seabios.bin",
        },

        vga_bios: {
          url: "/v86/vgabios.bin",
        },

        cdrom: {
          url: "/os/neuralkernel.iso",
        },

        autostart: true,
        disable_keyboard: false,
        disable_mouse: true,
      });

      emulatorRef.current = emulator;

      emulator.add_listener?.("emulator-started", () => {
        setStatus("running");
        setMessage("NeuralKernel is running inside your browser.");
      });

      emulator.add_listener?.("emulator-stopped", () => {
        setStatus((current) =>
          current === "paused" ? current : "stopped"
        );
      });

      setStatus("running");
      setMessage("NeuralKernel is booting inside your browser.");
    } catch (error) {
      setStatus("error");
      setMessage(
        error instanceof Error
          ? error.message
          : "The emulator could not start."
      );
    }
  }

  function pauseOrResume() {
    const emulator = emulatorRef.current;

    if (!emulator) {
      return;
    }

    if (status === "paused") {
      emulator.run?.();
      setStatus("running");
      setMessage("Emulation resumed.");
      return;
    }

    emulator.stop?.();
    setStatus("paused");
    setMessage("Emulation paused.");
  }

  function reset() {
    if (!emulatorRef.current) {
      return;
    }

    emulatorRef.current.restart?.();

    setStatus("running");
    setMessage("Virtual machine reset requested.");
  }

  function captureKeyboard() {
    if (status !== "running" && status !== "paused") {
      return;
    }

    screenRef.current?.focus();
    setKeyboardCaptured(true);
  }

  function releaseKeyboard() {
    setKeyboardCaptured(false);
  }

  const isActive = status === "running" || status === "paused";

  return (
    <section className="emulator-panel">
      <div className="emulator-toolbar">
        <div className="emulator-status">
          <span className={`status-dot ${status}`} />

          <div>
            <strong>
              {status === "idle" ? "OFFLINE" : status.toUpperCase()}
            </strong>
            <small>{message}</small>
          </div>
        </div>

        <div className="toolbar-actions">
          <button
            type="button"
            onClick={boot}
            disabled={status === "booting" || status === "running"}
          >
            {status === "booting"
              ? "Initializing..."
              : "Initialize & Boot"}
          </button>

          <button
            type="button"
            onClick={pauseOrResume}
            disabled={!emulatorRef.current}
          >
            {status === "paused" ? "Resume" : "Pause"}
          </button>

          <button
            type="button"
            onClick={reset}
            disabled={!emulatorRef.current}
          >
            Reset
          </button>
        </div>
      </div>

      <div
        className={`screen-shell ${
          keyboardCaptured ? "captured" : ""
        }`}
        onClick={captureKeyboard}
        role="button"
        tabIndex={0}
        onKeyDown={(event) => {
          if (event.key === "Escape") {
            releaseKeyboard();
            event.currentTarget.blur();
          }
        }}
      >
        <div ref={screenRef} className="v86-screen">
          <div className="v86-text-screen" />
          <canvas className="v86-canvas" />
        </div>

        {!isActive ? (
          <div className="screen-placeholder">
            <span className="boot-glyph" aria-hidden="true">
              _
            </span>

            <h2>NeuralKernel Browser Console</h2>

            <p>
              Launch the browser runtime and boot the complete NeuralKernel
              operating system.
            </p>

            <button
              type="button"
              disabled={status === "booting"}
              onClick={(event) => {
                event.stopPropagation();
                boot();
              }}
            >
              {status === "booting"
                ? "Initializing runtime..."
                : "Initialize & Boot"}
            </button>
          </div>
        ) : null}

        {status === "running" && keyboardCaptured ? (
          <span className="emulator-focus-caret" aria-hidden="true">
            _
          </span>
        ) : null}

        {status === "running" ? (
          <div className="capture-hint">
            {keyboardCaptured
              ? "Keyboard captured · Press Esc to release"
              : "Click the display to capture keyboard"}
          </div>
        ) : null}
      </div>

      <style jsx>{`
        .boot-glyph,
        .emulator-focus-caret {
          animation: kernel-caret-blink 0.85s steps(1, end) infinite;
        }

        .emulator-focus-caret {
          position: absolute;
          left: 18px;
          bottom: 18px;
          z-index: 4;
          pointer-events: none;
          font-family: var(--font-mono, monospace);
          font-size: 17px;
          line-height: 1;
          color: rgba(117, 255, 194, 0.7);
          text-shadow: 0 0 10px rgba(117, 255, 194, 0.45);
        }

        @keyframes kernel-caret-blink {
          0%,
          48% {
            opacity: 1;
          }

          49%,
          100% {
            opacity: 0;
          }
        }
      `}</style>
    </section>
  );
}
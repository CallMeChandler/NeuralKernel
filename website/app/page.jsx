import KernelEmulator from "@/components/KernelEmulator";

const quickCommands = [
  ["help", "List every available shell command"],
  ["ls", "Inspect files packed inside NKFS"],
  ["cat readme.nkfs", "Read a file from the NKFS archive"],
  ["write demo.nkfs hello", "Create a runtime file in memory"],
  ["edit notes.nkfs", "Open the full-screen text editor"],
  ["whoami", "Show the currently authenticated user"],
  ["uptime", "View time elapsed since kernel boot"],

  ["meminfo", "Inspect physical memory and heap usage"],
  ["ps", "View tasks, states and neural classifications"],
  ["neuralstats", "Inspect neural runtime decisions"],
  ["neural-off", "Return scheduling to standard round robin"],
  ["neural-on", "Enable neural scheduling and watchdog"],
  ["neural", "Enter natural-language command mode"],
  ["exit", "Log out and return to the login prompt"],
];

export default function HomePage() {
  const commandColumns = [
    quickCommands.slice(0, 7),
    quickCommands.slice(7),
  ];

  return (
    <div className="page-stack">
      <section className="hero-grid">
        <div className="hero-copy">
          <div className="eyebrow">LIVE · 32-BIT X86 · RING 3</div>

          <h1>
            Boot a neural operating system
            <span>inside your browser.</span>
          </h1>

          <p>
            NeuralKernel is a from-scratch hobby operating system with its
            own boot path, protected userspace, shell, in-memory filesystem,
            task scheduler, neural runtime, anomaly watchdog and
            natural-language command layer.
          </p>

          <div className="hero-badges">
            <span>Multiboot2</span>
            <span>i386</span>
            <span>ELF</span>
            <span>Ring 3</span>
            <span>NKFS</span>
            <span>Q8.8 Neural Runtime</span>
          </div>
        </div>

        <aside className="system-card">
          <div className="system-card-head">
            <span>NEURALKERNEL.RUNTIME</span>
            <small>STATUS / READY</small>
          </div>

          <dl>
            <div>
              <dt>Architecture</dt>
              <dd>i386 / 32-bit</dd>
            </div>

            <div>
              <dt>Kernel mode</dt>
              <dd>Ring 0</dd>
            </div>

            <div>
              <dt>Userspace</dt>
              <dd>Ring 3 ELF</dd>
            </div>

            <div>
              <dt>Filesystem</dt>
              <dd>NKFS + RAM overlay</dd>
            </div>

            <div>
              <dt>Neural math</dt>
              <dd>Q8.8 fixed-point</dd>
            </div>
          </dl>

          <div className="credential-block">
            <small>DEMO CREDENTIALS</small>
            <code>username: chandler</code>
            <code>password: chandler</code>
          </div>
        </aside>
      </section>

      <KernelEmulator />

      <section className="demo-command-section">
        <article className="command-card command-card-wide">
          <div className="section-kicker">SUGGESTED DEMO FLOW</div>
          <h2>Commands worth trying</h2>

          <p className="muted">
            Start with the standard shell, explore the filesystem and editor,
            then inspect the neural runtime or enter conversational mode.
          </p>

          <div className="command-columns">
            {commandColumns.map((commands, columnIndex) => (
              <div
                className="command-list"
                key={`command-column-${columnIndex}`}
              >
                {commands.map(([command, description]) => (
                  <div className="command-item" key={command}>
                    <code>{command}</code>
                    <span>{description}</span>
                  </div>
                ))}
              </div>
            ))}
          </div>
        </article>
      </section>
    </div>
  );
}
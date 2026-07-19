export default function TerminalSnippet({ title = "terminal", children }) {
  return (
    <div className="terminal-snippet">
      <div className="terminal-titlebar">
        <span />
        <span />
        <span />
        <small>{title}</small>
      </div>
      <pre>{children}</pre>
    </div>
  );
}

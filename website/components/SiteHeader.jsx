"use client";

import Image from "next/image";
import Link from "next/link";
import { usePathname } from "next/navigation";

const tabs = [
  { href: "/", label: "Test NeuralKernel", code: "RUN" },
  { href: "/docs", label: "Documentation", code: "DOC" },
];

export default function SiteHeader() {
  const pathname = usePathname();

  return (
    <header className="topbar">
      <div className="topbar-inner">
        <Link href="/" className="brand" aria-label="NeuralKernel home">
          <span className="brand-icon-wrap">
            <Image
              src="/NK_favicon.png"
              alt=""
              width={48}
              height={48}
              priority
              className="brand-icon"
            />
          </span>

          <span className="brand-copy">
            <strong>
              Neural<span>Kernel</span>
            </strong>
            <small>v0.1 · i386 runtime</small>
          </span>
        </Link>

        <nav className="tab-nav" aria-label="Primary navigation">
          {tabs.map((tab) => {
            const active =
              tab.href === "/"
                ? pathname === "/"
                : pathname.startsWith(tab.href);

            return (
              <Link
                key={tab.href}
                href={tab.href}
                className={`tab-link ${active ? "active" : ""}`}
              >
                <span>{tab.label}</span>
                <small>{tab.code}</small>
              </Link>
            );
          })}
        </nav>

        <a
          className="github-link"
          href="https://github.com/CallMeChandler"
          target="_blank"
          rel="noreferrer"
        >
          GitHub ↗
        </a>
      </div>
    </header>
  );
}
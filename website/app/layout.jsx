import "./globals.css";
import SiteHeader from "@/components/SiteHeader";

export const metadata = {
  title: "NeuralKernel",
  description:
    "A 32-bit x86 operating system with Ring 3 userspace, NKFS, neural scheduling, anomaly detection and natural-language interaction.",

  icons: {
    icon: "/NK_favicon.png",
    shortcut: "/NK_favicon.png",
    apple: "/NK_favicon.png",
  },
};

export default function RootLayout({ children }) {
  return (
    <html lang="en">
      <body>
        <div className="ambient ambient-one" />
        <div className="ambient ambient-two" />
        <div className="scanlines" />
        <SiteHeader />
        <main className="site-shell">{children}</main>
      </body>
    </html>
  );
}

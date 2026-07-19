import TerminalSnippet from "@/components/TerminalSnippet";
import FeatureGrid from "@/components/FeatureGrid";

export function useMDXComponents(components) {
  return {
    TerminalSnippet,
    FeatureGrid,
    ...components,
  };
}

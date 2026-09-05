"use client";

import { useEffect, useMemo, useState } from "react";

const routes: Record<string, string> = {
  "music-player": "/",
  "intrinsic-sizing": "/intrinsic-sizing",
  "auto-percent-width": "/auto-percent-width",
  "phase-a-shrink": "/phase-a-shrink",
  sizing: "/sizing",
  "justify-items": "/justify-items",
  complex: "/complex",
};

export default function ComparePage() {
  const [scene, setScene] = useState("music-player");

  useEffect(() => {
    let active = true;

    const followGui = async () => {
      try {
        const response = await fetch("/api/layout-scene", { cache: "no-store" });
        const selection = (await response.json()) as { scene?: string };
        if (active && selection.scene && routes[selection.scene]) {
          setScene(selection.scene);
        }
      } catch {
        // Keep showing the last available scene while the dev server or GUI reloads.
      }
    };

    void followGui();
    const timer = window.setInterval(followGui, 100);
    return () => {
      active = false;
      window.clearInterval(timer);
    };
  }, []);

  const route = useMemo(() => routes[scene] ?? "/", [scene]);

  return (
    <iframe
      key={route}
      src={route}
      title={`${scene} browser reference`}
      className="fixed inset-0 block h-screen w-screen border-0 bg-black"
    />
  );
}

import { readFile } from "node:fs/promises";

export const dynamic = "force-dynamic";

export async function GET() {
  let scene = "music-player";

  try {
    scene = (await readFile("/tmp/gui-layout-current-scene", "utf8")).trim() || scene;
  } catch {
    // The GUI has not published a selection yet.
  }

  return Response.json(
    { scene },
    { headers: { "Cache-Control": "no-store" } },
  );
}

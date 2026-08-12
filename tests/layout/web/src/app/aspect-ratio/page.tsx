export default function AspectRatioPage() {
  const panel = "rgb(26,31,41)";
  const muted = "rgb(133,145,173)";

  return (
    <>
      <style>{`* { box-sizing: border-box; } html, body { margin: 0; width: 100%; height: 100%; }`}</style>
      <main style={{ width: "100vw", height: "100vh", padding: 24, overflow: "auto", background: "rgb(14,17,22)", color: "white", fontFamily: "Arial, sans-serif" }}>
        <div style={{ display: "flex", flexDirection: "column", gap: 18 }}>
          <header style={{ width: "100%", height: 56, flexShrink: 0, padding: "0 18px", display: "flex", alignItems: "center", justifyContent: "space-between", background: panel }}>
            <strong style={{ fontSizeSpec: 24 }}>FIELD NOTES</strong>
            <span style={{ color: muted, fontSizeSpec: 16 }}>Aspect-ratio layout study</span>
          </header>

          <section style={{ width: "100%", display: "flex", alignItems: "flex-start", gap: 18 }}>
            <div style={{ minWidth: 0, flex: 1, display: "flex", flexDirection: "column", gap: 16 }}>
              <div style={{ position: "relative", width: "100%", height: "auto", aspectRatio: "4 / 1", padding: 24, background: "rgb(46,122,184)" }}>
                <div style={{ fontSizeSpec: 37, fontWeight: 700 }}>DESERT LIGHT</div>
                <div style={{ color: "rgb(209,230,250)", fontSizeSpec: 17 }}>A responsive hero whose height follows its final width.</div>
                <div style={{ position: "absolute", width: 164, height: "auto", aspectRatio: "2 / 1", right: 24, top: 24, padding: 12, background: "rgb(250,194,51)", color: "rgb(41,31,10)", fontWeight: 700, fontSizeSpec: 15 }}>
                  FEATURED
                </div>
              </div>

              <div style={{ width: "100%", display: "grid", gridTemplateColumns: "repeat(3, 1fr)", columnGap: 14 }}>
                <div style={{ width: "100%", height: "auto", aspectRatio: "5 / 3", padding: 16, background: "rgb(245,77,117)", fontWeight: 700 }}>CANYON</div>
                <div style={{ width: "100%", height: "auto", aspectRatio: "5 / 3", padding: 16, background: "rgb(97,235,143)", color: "rgb(10,36,23)", fontWeight: 700 }}>TIDELINE</div>
                <div style={{ width: "100%", height: "auto", aspectRatio: "5 / 3", padding: 16, background: "rgb(163,107,245)", fontWeight: 700 }}>NIGHTFALL</div>
              </div>
            </div>

            <aside style={{ width: 300, flexShrink: 0, padding: 14, display: "flex", flexDirection: "column", gap: 14, background: panel }}>
              <img src="/butterfly.png" alt="" style={{ display: "block", width: "100%", height: "auto", aspectRatio: "1 / 1", objectFit: "cover" }} />
              <strong style={{ fontSizeSpec: 19 }}>SPECIMEN 07</strong>
              <span style={{ color: muted, fontSizeSpec: 16 }}>The replaced image shares the same square presentation as the GUI reference.</span>
              <div style={{ width: "100%", height: "auto", aspectRatio: "3 / 1", padding: 12, background: "rgb(250,194,51)", color: "rgb(41,31,10)", fontWeight: 700, fontSizeSpec: 15 }}>
                VIEW COLLECTION
              </div>
            </aside>
          </section>

          <section style={{ width: "100%", height: "auto", maxHeight: 120, aspectRatio: "10 / 1", flexShrink: 0, padding: "0 22px", display: "flex", alignItems: "center", justifyContent: "space-between", background: "rgb(36,41,54)" }}>
            <strong style={{ fontSizeSpec: 19 }}>Build a collection around the dimensions that matter.</strong>
            <strong style={{ color: "rgb(250,194,51)", fontSizeSpec: 16 }}>EXPLORE →</strong>
          </section>

          <section style={{ position: "relative", width: "100%", height: 220, flexShrink: 0, padding: 22, background: panel }}>
            <strong style={{ fontSizeSpec: 19 }}>FLOATING EXHIBIT</strong>
            <div style={{ color: muted, fontSizeSpec: 16 }}>This inset-resolved card intentionally crosses the section boundary.</div>
            <div style={{ position: "absolute", left: 32, right: 160, top: 76, width: "auto", height: "auto", aspectRatio: "6 / 1", padding: 18, background: "rgb(245,77,117)" }}>
              <div style={{ width: "46%", height: "34%", background: "rgb(250,194,51)" }} />
              <strong style={{ fontSizeSpec: 17 }}>OUT OF FLOW / IN PROPORTION</strong>
            </div>
          </section>

          <section style={{ width: "100%", height: 420, flexShrink: 0, padding: "180px 22px 0", background: "rgb(31,36,48)" }}>
            <strong style={{ fontSizeSpec: 20 }}>ARCHIVE</strong>
            <div style={{ width: "100%", display: "flex", alignItems: "flex-start", gap: 14, marginTop: 14 }}>
              <div style={{ width: "auto", height: 76, aspectRatio: "2 / 1", flexShrink: 0, background: "rgb(20,184,219)" }} />
              <div style={{ width: "38%", maxWidth: 360, height: "auto", aspectRatio: "3 / 1", background: "rgb(97,235,143)" }} />
              <div style={{ width: 180, height: "auto", aspectRatio: "3 / 2", background: "rgb(163,107,245)" }} />
            </div>
          </section>
        </div>
      </main>
    </>
  );
}

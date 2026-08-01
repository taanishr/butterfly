const panel = {
  background: "rgb(31, 31, 38)",
  padding: 12,
} as const;

const box = (background: string) => ({
  background,
  borderRadius: 5,
});

export default function ComplexScene() {
  return (
    <main
      style={{
        width: "100vw",
        height: "100vh",
        boxSizing: "border-box",
        padding: 22,
        display: "grid",
        gridTemplateColumns: "260px 35% 1fr",
        gridTemplateRows: "178px 1fr 148px",
        columnGap: 14,
        rowGap: 14,
        background: "rgb(10, 10, 13)",
      }}
    >
      <div
        style={{
          ...panel,
          gridColumn: "1 / 4",
          gridRow: "1 / 2",
          minHeight: 150,
          display: "flex",
          flexDirection: "row",
          gap: 12,
        }}
      >
        <div style={{ ...box("rgb(0, 199, 148)"), width: 170, height: "100%", minWidth: 120, maxWidth: 220 }} />
        <div style={{ ...box("rgb(245, 56, 158)"), width: "28%", height: "100%", minWidth: 180, maxWidth: 420 }} />
        <div style={{ ...box("rgb(250, 235, 82)"), height: "100%", flexGrow: 1, minWidth: 180, maxWidth: 520 }} />
        <div style={{ ...box("rgb(41, 214, 230)"), width: 140, height: "100%", minWidth: 110, maxWidth: 180 }} />
      </div>

      <div
        style={{
          ...panel,
          gridColumn: "1 / 2",
          gridRow: "2 / 3",
          minWidth: 220,
          minHeight: 220,
          display: "flex",
          flexDirection: "column",
          gap: 10,
        }}
      >
        <div style={{ ...box("rgb(242, 46, 56)"), width: "100%", height: "22%", minHeight: 46, maxHeight: 90 }} />
        <div style={{ ...box("rgb(26, 184, 242)"), width: 155, height: 62, minWidth: 120, maxWidth: 210 }} />
        <div style={{ ...box("rgb(255, 122, 31)"), width: "72%", flexGrow: 1, minHeight: 70, maxHeight: 180 }} />
        <div style={{ ...box("rgb(138, 71, 250)"), width: "45%", height: 48, minWidth: 90, maxWidth: 150 }} />
      </div>

      <div
        style={{
          ...panel,
          gridColumn: "2 / 3",
          gridRow: "2 / 3",
          minWidth: 300,
          minHeight: 220,
          display: "grid",
          gridTemplateColumns: "120px 45% 1fr",
          gridTemplateRows: "64px 50% 1fr",
          columnGap: 10,
          rowGap: 10,
        }}
      >
        <div style={{ ...box("rgb(82, 242, 107)"), minWidth: 90, minHeight: 46 }} />
        <div style={{ ...box("rgb(173, 71, 245)"), minWidth: 130, maxWidth: 240, minHeight: 50 }} />
        <div style={{ ...box("rgb(250, 235, 82)"), minWidth: 80, maxWidth: 170, minHeight: 50 }} />
        <div style={{ ...box("rgb(41, 92, 250)"), gridColumn: "1 / 3", minHeight: 74, maxHeight: 130 }} />
        <div style={{ ...box("rgb(240, 46, 46)"), minWidth: 90, minHeight: 60 }} />
        <div style={{ ...box("rgb(41, 214, 230)"), gridColumn: "1 / 4", minHeight: 46, maxHeight: 80 }} />
      </div>

      <div
        style={{
          ...panel,
          gridColumn: "3 / 4",
          gridRow: "2 / 3",
          minWidth: 260,
          minHeight: 220,
          display: "flex",
          flexDirection: "row",
          flexWrap: "wrap",
          gap: 10,
        }}
      >
        <div style={{ ...box("rgb(242, 214, 41)"), width: "36%", height: 58, minWidth: 110, maxWidth: 210 }} />
        <div style={{ ...box("rgb(0, 199, 148)"), height: 58, flexGrow: 1, minWidth: 120, maxWidth: 240 }} />
        <div style={{ ...box("rgb(245, 56, 158)"), width: 180, height: 58, minWidth: 140, maxWidth: 220 }} />
        <div style={{ ...box("rgb(41, 92, 250)"), width: "62%", height: 58, minWidth: 180, maxWidth: 340 }} />
        <div style={{ ...box("rgb(255, 122, 31)"), height: 58, flexGrow: 1, minWidth: 90, maxWidth: 160 }} />
      </div>

      <div
        style={{
          ...panel,
          gridColumn: "1 / 4",
          gridRow: "3 / 4",
          minHeight: 120,
          display: "grid",
          gridTemplateColumns: "25% 220px 1fr 160px",
          gridTemplateRows: "1fr",
          columnGap: 12,
        }}
      >
        <div style={{ ...box("rgb(173, 71, 245)"), minWidth: 120 }} />
        <div style={{ ...box("rgb(26, 184, 242)"), minWidth: 160, maxWidth: 220 }} />
        <div style={{ ...box("rgb(82, 242, 107)"), minWidth: 220 }} />
        <div style={{ ...box("rgb(242, 46, 56)"), minWidth: 120 }} />
      </div>
    </main>
  );
}

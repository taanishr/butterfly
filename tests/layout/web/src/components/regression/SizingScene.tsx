const panel = {
  background: "rgb(31, 31, 38)",
  padding: 12,
} as const;

const box = (background: string) => ({
  background,
  borderRadius: 5,
});

export default function SizingScene() {
  return (
    <main
      style={{
        width: "100vw",
        height: "100vh",
        boxSizing: "border-box",
        padding: 24,
        display: "grid",
        gridTemplateColumns: "1fr 1fr",
        gridTemplateRows: "1fr 1fr",
        columnGap: 14,
        rowGap: 14,
        background: "rgb(10, 10, 13)",
      }}
    >
      <div
        style={{
          ...panel,
          minWidth: 260,
          maxWidth: 700,
          minHeight: 170,
          display: "flex",
          flexDirection: "row",
          gap: 10,
        }}
      >
        <div
          style={{
            ...box("rgb(242, 46, 56)"),
            width: "35%",
            height: "100%",
            minWidth: 90,
            maxWidth: 180,
          }}
        />
        <div
          style={{
            ...box("rgb(26, 184, 242)"),
            width: 140,
            height: "100%",
            minWidth: 100,
            maxWidth: 220,
          }}
        />
        <div
          style={{
            ...box("rgb(242, 214, 41)"),
            height: "100%",
            flexGrow: 1,
            minWidth: 80,
            maxWidth: 260,
          }}
        />
      </div>

      <div
        style={{
          ...panel,
          minWidth: 240,
          minHeight: 180,
          maxHeight: 360,
          display: "flex",
          flexDirection: "column",
          gap: 10,
        }}
      >
        <div
          style={{
            ...box("rgb(82, 242, 107)"),
            width: "100%",
            height: "25%",
            minHeight: 42,
            maxHeight: 90,
          }}
        />
        <div
          style={{
            ...box("rgb(173, 71, 245)"),
            width: 180,
            height: 64,
            minWidth: 120,
            maxWidth: 260,
          }}
        />
        <div
          style={{
            ...box("rgb(255, 122, 31)"),
            width: "75%",
            flexGrow: 1,
            minHeight: 50,
            maxHeight: 130,
          }}
        />
      </div>

      <div
        style={{
          ...panel,
          minWidth: 300,
          minHeight: 190,
          display: "grid",
          gridTemplateColumns: "140px 50% 1fr",
          gridTemplateRows: "70px 45% 1fr",
          columnGap: 10,
          rowGap: 10,
        }}
      >
        <div style={{ ...box("rgb(0, 199, 148)"), minWidth: 90, minHeight: 46 }} />
        <div style={{ ...box("rgb(245, 56, 158)"), minWidth: 130, maxWidth: 220, minHeight: 50 }} />
        <div style={{ ...box("rgb(250, 235, 82)"), minWidth: 80, maxWidth: 180, minHeight: 50 }} />
        <div
          style={{
            ...box("rgb(41, 92, 250)"),
            gridColumn: "1 / 3",
            minHeight: 70,
            maxHeight: 120,
          }}
        />
        <div style={{ ...box("rgb(240, 46, 46)"), minWidth: 90, minHeight: 60 }} />
      </div>

      <div
        style={{
          ...panel,
          minWidth: 260,
          minHeight: 180,
          maxHeight: 320,
          display: "flex",
          flexDirection: "row",
          flexWrap: "wrap",
          gap: 10,
        }}
      >
        <div style={{ ...box("rgb(41, 214, 230)"), width: "40%", height: 54, minWidth: 110, maxWidth: 220 }} />
        <div style={{ ...box("rgb(138, 71, 250)"), height: 54, flexGrow: 1, minWidth: 90, maxWidth: 180 }} />
        <div style={{ ...box("rgb(242, 214, 41)"), width: 150, height: 54, minWidth: 120, maxWidth: 210 }} />
        <div style={{ ...box("rgb(82, 242, 107)"), width: "65%", height: 54, minWidth: 160, maxWidth: 300 }} />
      </div>
    </main>
  );
}

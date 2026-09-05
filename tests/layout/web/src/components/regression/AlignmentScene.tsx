const panel = {
  background: "rgb(31, 31, 38)",
  padding: 12,
  minHeight: 180,
} as const;

const box = (background: string) => ({
  background,
  borderRadius: 5,
});

export default function AlignmentScene() {
  return (
    <main
      style={{
        width: "100vw",
        height: "100vh",
        boxSizing: "border-box",
        padding: 24,
        display: "grid",
        gridTemplateColumns: "50% 50%",
        gridTemplateRows: "50% 50%",
        columnGap: 14,
        rowGap: 14,
        background: "rgb(10, 10, 13)",
      }}
    >
      <div
        style={{
          ...panel,
          minWidth: 260,
          maxWidth: 680,
          minHeight: 170,
          display: "flex",
          flexDirection: "row",
          justifyContent: "space-between",
          alignItems: "center",
        }}
      >
        <div style={{ ...box("rgb(242, 46, 56)"), width: 70, height: 50, alignSelf: "flex-start" }} />
        <div style={{ ...box("rgb(26, 184, 242)"), width: 90, height: 80, alignSelf: "center" }} />
        <div style={{ ...box("rgb(242, 214, 41)"), width: 80, height: 55, alignSelf: "flex-end" }} />
      </div>

      <div
        style={{
          ...panel,
          minWidth: 240,
          maxHeight: 360,
          display: "flex",
          flexDirection: "column",
          justifyContent: "space-between",
          alignItems: "flex-end",
        }}
      >
        <div style={{ ...box("rgb(82, 242, 107)"), width: 70, height: 44, alignSelf: "flex-start" }} />
        <div style={{ ...box("rgb(173, 71, 245)"), width: 150, height: 60, alignSelf: "center" }} />
        <div style={{ ...box("rgb(255, 122, 31)"), width: 100, height: 52 }} />
      </div>

      <div
        style={{
          ...panel,
          minWidth: 260,
          display: "grid",
          gridTemplateColumns: "50% 50%",
          gridTemplateRows: "50% 50%",
          columnGap: 10,
          rowGap: 10,
          alignItems: "center",
        }}
      >
        <div style={{ ...box("rgb(0, 199, 148)"), width: 80, height: 46, alignSelf: "flex-start" }} />
        <div style={{ ...box("rgb(245, 56, 158)"), width: 110, height: 70, alignSelf: "center" }} />
        <div style={{ ...box("rgb(250, 235, 82)"), width: 90, height: 52, alignSelf: "flex-end" }} />
        <div
          style={{
            ...box("rgb(41, 92, 250)"),
            alignSelf: "stretch",
            minWidth: 90,
            maxWidth: 180,
            minHeight: 44,
            maxHeight: 90,
          }}
        />
      </div>

      <div
        style={{
          ...panel,
          minWidth: 260,
          display: "flex",
          flexDirection: "row",
          justifyContent: "center",
          alignItems: "stretch",
          gap: 12,
        }}
      >
        <div
          style={{
            ...box("rgb(240, 46, 46)"),
            width: 74,
            minHeight: 60,
            maxHeight: 150,
          }}
        />
        <div style={{ ...box("rgb(41, 214, 230)"), width: 120, height: 70, alignSelf: "center" }} />
        <div style={{ ...box("rgb(138, 71, 250)"), width: 90, height: 56, alignSelf: "flex-end" }} />
      </div>
    </main>
  );
}

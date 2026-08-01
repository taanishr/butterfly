export default function AbsoluteAutoSizePage() {
  return (
    <main
      style={{
        position: "relative",
        width: "100vw",
        height: "100vh",
        background: "#0f1217",
      }}
    >
      <div
        style={{
          position: "absolute",
          left: 120,
          top: 80,
          padding: 20,
          background: "#17b8d4",
        }}
      >
        <div
          style={{
            width: 180,
            height: 80,
            background: "#e83e8c",
          }}
        />
      </div>
    </main>
  );
}

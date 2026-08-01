export default function Page() {
  return (
    <main
      style={{
        width: "100vw",
        height: "100vh",
        background: "rgb(15, 18, 23)",
        overflow: "hidden",
      }}
    >
      <div
        style={{
          boxSizing: "border-box",
          width: "80%",
          maxWidth: 760,
          height: 260,
          marginTop: 80,
          marginLeft: "auto",
          marginRight: "auto",
          padding: 24,
          background: "rgb(38, 41, 51)",
        }}
      >
        <div
          style={{
            width: "min-content",
            marginTop: 46,
            marginLeft: "auto",
            marginRight: "auto",
            background: "rgb(20, 184, 219)",
          }}
        >
          <div style={{ width: 320, height: 120 }} />
        </div>
      </div>
    </main>
  );
}

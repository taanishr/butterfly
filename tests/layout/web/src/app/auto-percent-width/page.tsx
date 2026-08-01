export default function AutoPercentWidthPage() {
  return (
    <main
      style={{
        width: "100vw",
        height: "100vh",
        background: "rgb(15, 18, 23)",
      }}
    >
      <div
        style={{
          position: "absolute",
          left: 80,
          top: 80,
          width: "auto",
          height: "auto",
          background: "rgb(20, 184, 219)",
        }}
      >
        <div
          style={{
            width: "50%",
            height: 120,
            background: "rgb(245, 77, 117)",
          }}
        >
          <div
            style={{
              width: 600,
              height: 80,
              background: "rgb(250, 194, 51)",
            }}
          />
        </div>
      </div>
    </main>
  );
}

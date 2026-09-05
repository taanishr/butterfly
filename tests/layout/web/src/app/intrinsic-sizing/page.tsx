const intrinsicText = "Intrinsic sizing chooses every soft break opportunity";

const sample = {
  fontFamily: "Arial, sans-serif",
  fontSizeSpec: 18,
  lineHeight: "normal",
} as const;

export default function Page() {
  return (
    <main
      style={{
        boxSizing: "border-box",
        position: "fixed",
        inset: 0,
        padding: 32,
        overflow: "auto",
        background: "rgb(15, 18, 23)",
      }}
    >
      <div
        style={{
          ...sample,
          width: "min-content",
          color: "rgb(245, 247, 255)",
          background: "rgb(20, 184, 219)",
        }}
      >
        {intrinsicText}
      </div>

      <div
        style={{
          ...sample,
          width: "max-content",
          marginTop: 20,
          color: "rgb(245, 247, 255)",
          background: "rgb(245, 77, 117)",
        }}
      >
        {intrinsicText}
      </div>

      <div
        style={{
          width: 360,
          marginTop: 20,
          background: "rgb(38, 41, 51)",
        }}
      >
        <div
          style={{
            ...sample,
            width: "fit-content",
            color: "rgb(20, 23, 28)",
            background: "rgb(250, 194, 51)",
          }}
        >
          {intrinsicText}
        </div>
      </div>

      <div
        style={{
          ...sample,
          width: 120,
          minWidth: "max-content",
          marginTop: 20,
          color: "rgb(20, 23, 28)",
          background: "rgb(97, 235, 143)",
        }}
      >
        {intrinsicText}
      </div>

      <div
        style={{
          ...sample,
          width: 640,
          maxWidth: "min-content",
          marginTop: 20,
          color: "rgb(245, 247, 255)",
          background: "rgb(163, 107, 245)",
        }}
      >
        {intrinsicText}
      </div>
    </main>
  );
}

export default function NestedPercentIntrinsicPage() {
  return (
    <main
      style={{
        width: "100vw",
        height: "100vh",
        padding: 32,
        gap: 24,
        display: "flex",
        flexDirection: "column",
        alignItems: "flex-start",
        background: "rgb(6% 7% 9%)",
      }}
    >
      <span
        style={{
          fontSize: 22,
          color: "rgb(96% 97% 100%)",
        }}
      >
        NESTED PERCENTAGE / MAX-CONTENT
      </span>
      <span
        style={{
          fontSize: 14,
          color: "rgb(70% 73% 80%)",
        }}
      >
        The cyan percentage target should match the green auto control.
      </span>

      <div
        style={{
          width: 640,
          padding: 16,
          gap: 10,
          display: "flex",
          flexDirection: "column",
          alignItems: "flex-start",
          background: "rgb(15% 16% 20%)",
        }}
      >
        <span
          style={{
            fontSize: 12,
            color: "rgb(38% 92% 56%)",
          }}
        >
          AUTO CONTROL
        </span>
        <div
          style={{
            width: "50%",
            minWidth: "max-content",
            padding: 10,
            background: "rgb(96% 30% 46%)",
          }}
        >
          <div
            style={{
              width: "auto",
              padding: 8,
              background: "rgb(38% 92% 56%)",
            }}
          >
            <span
              style={{
                fontSize: 16,
                color: "rgb(4% 5% 6%)",
              }}
            >
              intrinsic_percentage_descendant_should_use_content
            </span>
          </div>
        </div>
      </div>

      <div
        style={{
          width: 640,
          padding: 16,
          gap: 10,
          display: "flex",
          flexDirection: "column",
          alignItems: "flex-start",
          background: "rgb(15% 16% 20%)",
        }}
      >
        <span
          style={{
            fontSize: 12,
            color: "rgb(10% 72% 95%)",
          }}
        >
          50% TARGET
        </span>
        <div
          style={{
            width: "50%",
            minWidth: "max-content",
            padding: 10,
            background: "rgb(96% 30% 46%)",
          }}
        >
          <div
            style={{
              width: "50%",
              padding: 8,
              background: "rgb(10% 72% 95%)",
            }}
          >
            <span
              style={{
                fontSize: 16,
                color: "rgb(4% 5% 6%)",
              }}
            >
              intrinsic_percentage_descendant_should_use_content
            </span>
          </div>
        </div>
      </div>
    </main>
  );
}

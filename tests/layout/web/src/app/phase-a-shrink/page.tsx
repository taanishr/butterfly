export default function Page() {
  return (
    <div className="w-screen h-screen bg-[rgb(6%_7%_9%)] flex flex-col items-start p-[48px] gap-[28px]">
      <span className="font-[Arial] text-[24px] text-[rgb(96%_97%_100%)]">
        PHASE A: INDEFINITE PERCENT VS AUTO
      </span>
      <span className="font-[Arial] text-[14px] text-[rgb(70%_73%_80%)]">
        Compare each cyan percentage target and auto control with the browser reference
      </span>

      <span className="font-[Arial] text-[13px] text-[rgb(98%_76%_24%)]">
        INDEFINITE WIDTH — 60% TARGET VS AUTO CONTROL
      </span>
      <div className="flex items-start gap-[48px]">
        <div className="flex flex-col items-start gap-[8px]">
          <span className="font-[Arial] text-[12px] text-[rgb(38%_92%_56%)]">
            AUTO CONTROL
          </span>
          <div className="w-auto h-[112px] bg-[rgb(15%_16%_20%)] flex items-center p-[14px]">
            <div className="w-auto h-[76px] bg-[rgb(8%_72%_86%)] flex items-center p-[10px] gap-[8px]">
              <div className="w-[180px] h-[56px] bg-[rgb(98%_76%_20%)]" />
              <div className="w-[120px] h-[56px] bg-[rgb(96%_30%_46%)]" />
            </div>
          </div>
        </div>

        <div className="flex flex-col items-start gap-[8px]">
          <span className="font-[Arial] text-[12px] text-[rgb(100%_54%_22%)]">
            60% TARGET
          </span>
          <div className="w-auto h-[112px] bg-[rgb(15%_16%_20%)] flex items-center p-[14px]">
            <div className="w-[60%] h-[76px] bg-[rgb(8%_72%_86%)] flex items-center p-[10px] gap-[8px]">
              <div className="w-[180px] h-[56px] bg-[rgb(98%_76%_20%)]" />
              <div className="w-[120px] h-[56px] bg-[rgb(96%_30%_46%)]" />
            </div>
          </div>
        </div>
      </div>

      <span className="font-[Arial] text-[13px] text-[rgb(98%_76%_24%)]">
        INDEFINITE HEIGHT — 50% TARGET VS AUTO CONTROL
      </span>
      <div className="flex items-start gap-[48px]">
        <div className="flex flex-col gap-[8px]">
          <span className="font-[Arial] text-[12px] text-[rgb(38%_92%_56%)]">
            AUTO CONTROL
          </span>
          <div className="w-[360px] h-auto bg-[rgb(15%_16%_20%)] flex flex-col items-stretch p-[14px]">
            <div className="w-full h-auto bg-[rgb(8%_72%_86%)] flex flex-col p-[10px] gap-[8px]">
              <div className="w-full h-[72px] bg-[rgb(98%_76%_20%)]" />
              <div className="w-full h-[48px] bg-[rgb(96%_30%_46%)]" />
            </div>
          </div>
        </div>

        <div className="flex flex-col gap-[8px]">
          <span className="font-[Arial] text-[12px] text-[rgb(100%_54%_22%)]">
            50% TARGET
          </span>
          <div className="w-[360px] h-auto bg-[rgb(15%_16%_20%)] flex flex-col items-stretch p-[14px]">
            <div className="w-full h-[50%] bg-[rgb(8%_72%_86%)] flex flex-col p-[10px] gap-[8px]">
              <div className="w-full h-[72px] bg-[rgb(98%_76%_20%)]" />
              <div className="w-full h-[48px] bg-[rgb(96%_30%_46%)]" />
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}

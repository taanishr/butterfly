export default function PercentReresolutionPage() {
  return (
    <main className="box-border grid h-screen w-screen grid-cols-[240px_1fr_2fr] grid-rows-[180px_1fr] gap-x-[20px] gap-y-[20px] bg-[rgb(6%_7%_9%)] p-[24px]">
      <div className="col-start-1 col-end-2 row-start-1 row-end-2 h-1/2 w-1/2 bg-[rgb(96%_30%_46%)]" />

      <div className="col-start-2 col-end-3 row-start-1 row-end-2 h-[60%] w-3/4 bg-[rgb(38%_92%_56%)]">
        <div className="h-1/2 w-1/2 bg-[rgb(98%_76%_20%)]" />
      </div>

      <div className="col-start-3 col-end-4 row-start-2 row-end-3 h-[70%] w-[40%] bg-[rgb(10%_72%_95%)]">
        <div className="h-1/2 w-1/2 bg-[rgb(68%_28%_96%)]">
          <div className="h-1/2 w-1/2 bg-[rgb(95%_84%_16%)]" />
        </div>
      </div>
    </main>
  );
}

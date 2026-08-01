export default function Page() {
  return (
    <div className="box-border grid h-screen w-screen grid-cols-2 grid-rows-2 gap-5 bg-[rgb(4%_4%_5%)] p-10">
      <div className="grid grid-cols-1 grid-rows-1 items-center justify-items-start bg-[rgb(12%_12%_15%)] p-5">
        <div className="h-[80px] w-[120px] rounded-[5px] bg-[rgb(95%_18%_22%)]" />
      </div>

      <div className="grid grid-cols-1 grid-rows-1 items-center justify-items-center bg-[rgb(12%_12%_15%)] p-5">
        <div className="h-[80px] w-[120px] rounded-[5px] bg-[rgb(32%_95%_42%)]" />
      </div>

      <div className="grid grid-cols-1 grid-rows-1 items-center justify-items-end bg-[rgb(12%_12%_15%)] p-5">
        <div className="h-[80px] w-[120px] justify-self-start rounded-[5px] bg-[rgb(10%_72%_95%)]" />
      </div>

      <div className="grid grid-cols-1 grid-rows-1 items-center justify-items-start bg-[rgb(12%_12%_15%)] p-5">
        <div className="h-[80px] justify-self-stretch rounded-[5px] bg-[rgb(54%_28%_98%)]" />
      </div>
    </div>
  );
}

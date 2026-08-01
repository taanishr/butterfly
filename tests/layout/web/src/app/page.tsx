"use client"

import Image from "next/image";
import { useState } from "react";

export default function Home() {
  // return (
  //   <div>
  //     <div className="relative top-[20px] left-[50px] bg-[rgba(0,128,128,1)] h-[50px]"/>
  //     <div className="bg-[rgba(0,0,128,0.5)] rounded-[50px] h-[100px]"/>
  //     <div className="bg-white w-[60px] h-[60px] rounded-full border-red-500 border-[2px] fixed left-[200px] top-[500px]"/>
  //     <div className="bg-blue-500 w-[60px] h-[60px] rounded-full border-red-500 border-[2px] absolute left-[400px] top-[500px]"/>
  //   </div>
  // );

  //  return (
  //   <div
  //     style={{
  //       height: 512,
  //       width: 512,
  //       backgroundColor: "rgba(240,240,240,1)",
  //     }}
  //   >
  //     <div
  //       style={{
  //         height: 400,
  //         width: 400,
  //         left: 20,
  //         top: 50,
  //         position: "relative",
  //         backgroundColor: "rgba(144,238,144,1)",
  //       }}
  //     >
  //       <span style={{ fontSize: 16, color: "rgba(0,0,0,1)" }}>
  //         Inline element 1{" "}
  //         <span style={{ fontSize: 16, color: "rgba(0,0,0,1)" }}>
  //           Nested inline element 1{" "}
  //         </span>
  //         <span style={{ fontSize: 16, color: "rgba(0,0,0,1)" }}>
  //           Nested inline element 2
  //         </span>
  //         <div
  //           style={{
  //             height: 10,
  //             width: 60,
  //             backgroundColor: "rgba(28,51,16,1)",
  //             display: "block", // since divs are block by default
  //           }}
  //         />
  //       </span>

  //       <span style={{ fontSize: 16, color: "rgba(0,0,0,1)" }}>
  //         Inline element 2
  //       </span>

  //       <div
  //         style={{
  //           height: 60,
  //           width: 60,
  //           borderRadius: 30,
  //           backgroundColor: "rgba(0,0,255,1)",
  //           position: "absolute",
  //           left: 50,
  //           top: 100,
  //         }}
  //       />
  //     </div>

  //     <div
  //       style={{
  //         height: 60,
  //         width: 60,
  //         borderRadius: 30,
  //         backgroundColor: "rgba(255,0,0,1)",
  //         position: "fixed",
  //         left: 432,
  //         top: 432,
  //       }}
  //     />
  //   </div>
  // );


    /* Existing event-handler test.
    function func1() {
        console.log("event handler 1")
    }
    */
    
//     return (
//         <div className="w-full h-full bg-gray-100 flex items-center justify-center">
//     <div className="w-[280px] bg-white rounded-2xl flex flex-col items-center p-6 gap-3">
//         <img src="https://i.pravatar.cc/80" className="w-[80px] h-[80px] rounded-full" />
//         <span className="text-[18pt] text-gray-900 font-medium">Sarah Johnson</span>
//         <span className="text-[13pt] text-gray-400">Product Designer @ Figma</span>
//         <div className="w-full flex justify-around">
//             <div className="flex flex-col items-center">
//                 <span className="text-[16pt] text-gray-900">284</span>
//                 <span className="text-[12pt] text-gray-400">Posts</span>
//             </div>
//             <div className="flex flex-col items-center">
//                 <span className="text-[16pt] text-gray-900">12.4k</span>
//                 <span className="text-[12pt] text-gray-400">Followers</span>
//             </div>
//             <div className="flex flex-col items-center">
//                 <span className="text-[16pt] text-gray-900">891</span>
//                 <span className="text-[12pt] text-gray-400">Following</span>
//             </div>
//         </div>
//         <div className="w-full h-[40px] bg-violet-500 rounded-full flex items-center justify-center cursor-pointer">
//             <span className="text-[14pt] text-white">Follow</span>
//         </div>
//     </div>
// </div>
//     )


    // return (
    //     <div className="w-[300px] h-[400px] bg-gray-200 flex flex-wrap content-end">
    //         <div className="w-[150px] h-[80px] bg-red-400"></div>
    //         <div className="w-[150px] h-[80px] bg-green-400"></div>
    //         <div className="w-[150px] h-[80px] bg-blue-400"></div>
    //         <div className="w-[150px] h-[80px] bg-fuchsia-400"></div>
    //     </div>
    // )
    //     <div>
    //         <div className="mt-[16px] bg-red-200 h-[20px]">
    //             <span className="mt-[24px]">Hello world</span>
    //         </div>
    //     </div>
        // <div className="h-screen w-full">
        //     <div className="absolute bg-blue-200 top-0 bottom-0 h-[200px] w-[200px] mt-0 mb-0">
        //         <span className="">
        //             fdsfsdfsdfsdf dsfsdfsdf
        //         </span>
        //         <span>
        //             fdsfsdfsdfsdfdsfsdfsdfdasdasdasdasd
        //         </span>
        //     </div>
        //     {/* <div className="bg-green-200 w-[200px] mr-[20px] mb-[10px] h-[200px]">
        //         <span className="text-red-200 mr-[10px]">
        //             sdfsdfsdfsdf 
        //         </span>
        //         <span className="text-white">
        //              sdfs
        //         </span>
        //     </div> */}
        //     {/* <div className="bg-green-200 w-[400px] h-[400px] mb-[60px] pl-[20px]">
        //         <div className="w-[200px] h-[200px] mt-[20px] bg-blue-200 absolute" onClick={func1}>
        //             <div className="absolute top-[10px] left-[20px] h-[400px] w-[200px] bg-red-200">
        //             </div>
        //             <div className="bg-yellow-200 h-[10px]">
                    
        //             </div>
        //         </div>
        //     </div> */}
        // </div>
    //     <div className="w-screen h-screen bg-white border border-[rgba(196,181,255,1)] left-[10px]" dir="ltr">
    //         <div className="w-1/5 h-full bg-[rgba(255,128,255,0.8)] pt-1 mr-[10px]">
    //             <div
    //                 className="
    //                     w-[60px] h-[30px]
    //                     bg-[rgba(127,0,255,1)]
    //                     mt-[30px] mx-auto
    //                     rounded-[7.5px]
    //                      pt-[4.5px]
    //                     border border-[rgb(108, 96, 157)]
    //                 "
    //                 >
    //                 <span
    //                     className="
    //                     text-[48px]
    //                     font-[Arial]
    //                     text-black
    //                     mx-auto
    //                     "
    //                 >
    //                     Startfsd
    //                 </span>
    //                 {/* <div className="h-[40px] w-[100px] bg-black">
    //                 </div> */}
    //                 <span
    //                     className="
    //                     text-[48px]
    //                     font-[Arial]
    //                     text-black
    //                     ml-[20px]
    //                     "
    //                 >
    //                     fsdfsdfsda 
    //                 </span>
    //                 </div>

    //                 <div
    //                 className="
    //                     w-[60px] h-[30px]
    //                     bg-[rgba(128,0,0,1)]
    //                     mt-[10px]
    //                     ml-[10px]
    //                 "
    //                 />
    //             </div>
    //         </div>
    // )
    // <>
    //   <div className="bg-green-100 mb-0">
    //     {/* PARENT */}

    //     <div className="bg-green-200 mb-8">
    //       {/* CHILD */}

    //       <div className="bg-green-300 mb-16">
    //         {/* GRANDCHILD */}
    //         GC
    //       </div>

    //     </div>
    //   </div>

    //   <div className="bg-red-300 h-4" />
    // </>
    // )

    // return (

        // <div className="absolute left-[10px] w-full h-full bg-white border border-[#C5B5FF]">
        //     {/* Container corresponding to: div(percent(0.2), percent(1.0), rgba(1,0.5,1,0.8)) */}
        //     <div className="w-[20%] h-full bg-[rgba(255,128,255,0.8)]">


        //     {/* Purple button-like div: 60x30, marginTop 30, centered horizontally, cornerRadius 7.5% */}
        //     <div
        //     className="w-[60px] h-[30px] mt-[30px] mx-auto rounded-[7.5%] pl-[9px] pt-[4.5px] border border-[#C5B5FF] bg-[#7f00ff] cursor-pointer"
        //     >
        //     <span className="inline text-[64px] font-sans ml-[10px] mr-[10px] text-black">Startfsd</span>


        //     <span className="inline text-[64px] font-sans text-black">fsdfsdfsda dads sdsfsdsds</span>
        //     </div>


        //     {/* Another small div 60x30 with marginTop 10 */}
        //     <div className="w-[60px] h-[30px] mt-[10px] bg-[#800000]" />
        //     </div>
        // </div>
    
    // <div dir="ltr">
    //   <div className="relative w-screen h-screen bg-white left-10 right-10">
    //   <div className="w-full h-full bg-white absolute left-[10px]">
    //     {/* Sidebar */}
    //     <div className="w-[20%] h-full bg-[rgba(255,128,255,0.8)] pt-0">
          
    //       {/* Button */}
    //       <div
    //         className="
    //           block
    //           w-[70px]
    //           bg-[rgba(127,0,255,1)]
    //           mt-[30px]
    //           mx-auto
    //           rounded-[7.5%]
    //           pl-[9px] pt-[4.5px]
    //           border
    //           border-[rgba(196,181,255,1)]
    //           cursor-pointer
    //         "
    //         onMouseDown={() => console.log("clicked")}
    //       >
    //         {/* Uncomment if needed */}
            
    //         <span className="text-black text-[16pt] font-[Arial]">Startfsd</span>
    //         <span className="text-black text-[16pt] font-[Arial]">fsdfsdfsda</span>
    //         <span>dads sdsfsdsds</span>
           
    //       </div>
    //       <div
    //         className="
    //           block
    //           w-[60px] h-[30px]
    //           bg-[rgba(128,0,0,1)]
    //           mt-[10px]
    //         "
    //       />
    //     </div>
    //   </div>
    //   </div>
    // </div>
    
  //   <div className="w-full h-full bg-white relative">
  //     <div className="w-full h-[100px] bg-gray-200 flex">
  //         <div className="w-[100px] h-[80px] bg-red-400"></div>
  //         <div className="w-[100px] h-[80px] bg-green-400 flex-grow"></div>
  //         <div className="w-[100px] h-[80px] bg-blue-400"></div>
  //     </div>
  // </div>

//   <div className="w-full h-full bg-white relative">
//     <div className="bg-gray-200 flex items-start">
//         <div className="w-[100px] h-[80px] bg-red-400"></div>
//         <div className="w-[80px] bg-green-400">
//             <span>hello world fsdfsdfsdfsdfsdfsdf</span>
//         </div>
//         <div className="w-[100px] h-[80px] bg-blue-400"></div>
//     </div>
// </div>

  /* Existing inspector test.
  const items = Array.from({ length: 5 }, (_, i) => i + 1);



    return (
        // <div className="h-screen w-screen bg-[#0a0a0d] p-[22px] grid gap-[14px]" style={{ gridTemplateColumns: "260px 35% 1fr", gridTemplateRows: "178px 1fr 148px" }}>
        //     <div className="bg-[#1f1f26] p-3 min-h-[150px] flex flex-row gap-3" style={{ gridColumn: "1 / 4", gridRow: "1 / 2" }}>
        //         <div className="w-[170px] h-full min-w-[120px] max-w-[220px] bg-[#00c792] rounded-[5px]" />
        //         <div className="w-[28%] h-full min-w-[180px] max-w-[420px] bg-[#f5369e] rounded-[5px]" />
        //         <div className="h-full flex-grow min-w-[180px] max-w-[520px] bg-[#faeb52] rounded-[5px]" />
        //         <div className="w-[140px] h-full min-w-[110px] max-w-[180px] bg-[#29d6e6] rounded-[5px]" />
        //     </div>

        //     <div className="bg-[#1f1f26] p-3 min-w-[220px] min-h-[220px] flex flex-col gap-[10px]" style={{ gridColumn: "1 / 2", gridRow: "2 / 3" }}>
        //         <div className="w-full h-[22%] min-h-[46px] max-h-[90px] bg-[#f22e38] rounded-[5px]" />
        //         <div className="w-[155px] h-[62px] min-w-[120px] max-w-[210px] bg-[#1ab8f2] rounded-[5px]" />
        //         <div className="w-[72%] flex-grow min-h-[70px] max-h-[180px] bg-[#ff7a1f] rounded-[5px]" />
        //         <div className="w-[45%] h-[48px] min-w-[90px] max-w-[150px] bg-[#8a47fa] rounded-[5px]" />
        //     </div>

        //     <div className="bg-[#1f1f26] p-3 min-w-[300px] min-h-[220px] grid gap-[10px]" style={{ gridColumn: "2 / 3", gridRow: "2 / 3", gridTemplateColumns: "120px 45% 1fr", gridTemplateRows: "64px 50% 1fr" }}>
        //         <div className="min-w-[90px] min-h-[46px] bg-[#52f26b] rounded-[5px]" />
        //         <div className="min-w-[130px] max-w-[240px] min-h-[50px] bg-[#ad47f5] rounded-[5px]" />
        //         <div className="min-w-[80px] max-w-[170px] min-h-[50px] bg-[#faeb52] rounded-[5px]" />
        //         <div className="min-h-[74px] max-h-[130px] bg-[#295cfb] rounded-[5px]" style={{ gridColumn: "1 / 3" }} />
        //         <div className="min-w-[90px] min-h-[60px] bg-[#f02e2e] rounded-[5px]" />
        //         <div className="min-h-[46px] max-h-[80px] bg-[#29d6e6] rounded-[5px]" style={{ gridColumn: "1 / 4" }} />
        //     </div>

        //     <div className="bg-[#1f1f26] p-3 min-w-[260px] min-h-[220px] flex flex-row flex-wrap gap-[10px]" style={{ gridColumn: "3 / 4", gridRow: "2 / 3" }}>
        //         <div className="w-[36%] h-[58px] min-w-[110px] max-w-[210px] bg-[#f2d629] rounded-[5px]" />
        //         <div className="h-[58px] flex-grow min-w-[120px] max-w-[240px] bg-[#00c792] rounded-[5px]" />
        //         <div className="w-[180px] h-[58px] min-w-[140px] max-w-[220px] bg-[#f5369e] rounded-[5px]" />
        //         <div className="w-[62%] h-[58px] min-w-[180px] max-w-[340px] bg-[#295cfb] rounded-[5px]" />
        //         <div className="h-[58px] flex-grow min-w-[90px] max-w-[160px] bg-[#ff7a1f] rounded-[5px]" />
        //     </div>

        //     <div className="bg-[#1f1f26] p-3 min-h-[120px] grid gap-x-3" style={{ gridColumn: "1 / 4", gridRow: "3 / 4", gridTemplateColumns: "25% 220px 1fr 160px", gridTemplateRows: "1fr" }}>
        //         <div className="min-w-[120px] bg-[#ad47f5] rounded-[5px]" />
        //         <div className="min-w-[160px] max-w-[220px] bg-[#1ab8f2] rounded-[5px]" />
        //         <div className="min-w-[220px] bg-[#52f26b] rounded-[5px]" />
        //         <div className="min-w-[120px] bg-[#f22e38] rounded-[5px]" />
        //     </div>
        // </div>
        
            <div className="fixed right-0 top-0 h-[240px] w-[240px] bg-black/50 p-3">
                <span className="text-[24pt]">Inspector</span>

                <div className="flex flex-col">
                <span className="text-[12pt]">Mouse</span>

                <div className="flex gap-3">
                    <span className="text-[12pt]">fsdfsdf</span>
                    <span className="text-[12pt]">fsdfsdf</span>
                </div>
                </div>
            </div>
    )
  */

  /*
  const sample = `Alpha   beta gamma delta epsilon zeta
Second   line with Supercalifragilisticexpialidocious tail`;
  const breakAllSample =
    "BreakAll: Supercalifragilisticexpialidocious0123456789";
  const cases = [
    { label: "Normal", whiteSpace: "normal", wordBreak: "normal", text: sample },
    { label: "NoWrap", whiteSpace: "nowrap", wordBreak: "normal", text: sample },
    { label: "Pre", whiteSpace: "pre", wordBreak: "normal", text: sample },
    { label: "PreWrap", whiteSpace: "pre-wrap", wordBreak: "normal", text: sample },
    {
      label: "Normal + BreakAll",
      whiteSpace: "normal",
      wordBreak: "break-all",
      text: breakAllSample,
    },
  ] as const;

  return (
    <main
      style={{
        width: "100vw",
        minHeight: "100vh",
        boxSizing: "border-box",
        display: "flex",
        flexDirection: "column",
        gap: 12,
        padding: 24,
        overflow: "auto",
        background: "#ffffff",
        color: "#14171c",
        fontFamily: "Arial, Helvetica, sans-serif",
      }}
    >
      {cases.map((testCase) => (
        <section
          key={testCase.label}
          style={{ display: "flex", flexDirection: "column", gap: 4 }}
        >
          <div style={{ color: "#525966", fontSize: "12pt" }}>
            {testCase.label}
          </div>
          <div
            style={{
              width: 280,
              minHeight: 90,
              background: "#f0f2f7",
              fontSize: "16pt",
              lineHeight: 1.25,
              whiteSpace: testCase.whiteSpace,
              wordBreak: testCase.wordBreak,
            }}
          >
            {testCase.text}
          </div>
        </section>
      ))}
    </main>
  );
  */

  /*
  const overflowSample = "The quick brown fox jumps over the lazy dog";
  const cases = [
    { label: "Clip", width: 280, text: overflowSample, textOverflow: "clip", overflow: "hidden" },
    { label: "Ellipsis, text fits", width: 280, text: "Short text", textOverflow: "ellipsis", overflow: "hidden" },
    { label: "Ellipsis", width: 280, text: overflowSample, textOverflow: "ellipsis", overflow: "hidden" },
    { label: "Ellipsis, narrower than marker", width: 6, text: "Wide text", textOverflow: "ellipsis", overflow: "hidden" },
    { label: "Custom ending", width: 280, text: overflowSample, textOverflow: '" [more]"', overflow: "hidden" },
    { label: "Ellipsis + Scroll", width: 140, text: overflowSample, textOverflow: "ellipsis", overflow: "scroll" },
  ] as const;

  return (
    <main
      style={{
        width: "100vw",
        minHeight: "100vh",
        boxSizing: "border-box",
        display: "flex",
        flexDirection: "column",
        gap: 12,
        padding: 24,
        overflow: "auto",
        background: "#ffffff",
        color: "#14171c",
        fontFamily: "Arial, Helvetica, sans-serif",
      }}
      dir="rtl"
    >
      {cases.map((testCase) => (
        <section
          key={testCase.label}
          style={{ display: "flex", flexDirection: "column", gap: 4 }}
        >
          <div style={{ color: "#525966", fontSize: "12pt" }}>
            {testCase.label}
          </div>
          <div
            style={{
              width: testCase.width,
              height: 32,
              overflow: testCase.overflow,
              background: "#f0f2f7",
              fontSize: "16pt",
              lineHeight: 1.25,
              whiteSpace: "nowrap",
              textOverflow: testCase.textOverflow,
            }}
          >
            {testCase.text}
          </div>
        </section>
      ))}
      <section style={{ display: "flex", flexDirection: "column", gap: 4 }}>
        <div style={{ color: "#525966", fontSize: "12pt" }}>
          Nested clipping ancestor
        </div>
        <div
          style={{
            width: 280,
            height: 32,
            overflow: "hidden",
            background: "#f0f2f7",
            fontSize: "16pt",
            lineHeight: 1.25,
            whiteSpace: "nowrap",
            textOverflow: "ellipsis",
          }}
        >
          <div>{overflowSample}</div>
        </div>
      </section>
    </main>
  );
  */

  /*
  const alignmentSample = `Short line
A considerably longer line that wraps inside the panel`;
  const alignmentCases = [
    { label: "Start", textAlign: "start" },
    { label: "Left", textAlign: "left" },
    { label: "Center", textAlign: "center" },
    { label: "Right", textAlign: "right" },
  ] as const;

  return (
    <main
      style={{
        width: "100vw",
        minHeight: "100vh",
        boxSizing: "border-box",
        display: "flex",
        flexDirection: "column",
        gap: 12,
        padding: 24,
        overflow: "auto",
        background: "#ffffff",
        color: "#14171c",
        fontFamily: "Arial, Helvetica, sans-serif",
      }}
    >
      {alignmentCases.map((testCase) => (
        <section
          key={testCase.label}
          style={{ display: "flex", flexDirection: "column", gap: 4 }}
        >
          <div style={{ color: "#525966", fontSize: "12pt" }}>
            {testCase.label}
          </div>
          <div
            style={{
              width: 280,
              minHeight: 90,
              background: "#f0f2f7",
              fontSize: "16pt",
              lineHeight: 1.25,
              whiteSpace: "pre-wrap",
              textAlign: testCase.textAlign,
            }}
          >
            {alignmentSample}
          </div>
        </section>
      ))}
    </main>
  );
  */

  const [extended, setExtended] = useState(false);
  const tracks = [
    ["01", "Endless Reverie", "4:38"],
    ["02", "Crystalline", "3:52"],
    ["03", "Pale Shore", "5:14"],
    ["04", "Inversion", "4:07"],
    ["05", "Soft Architecture", "6:21"],
    ["06", "Between Frames", "3:44"],
    ["07", "Nocturne Loop", "4:58"],
    ["08", "Refract", "3:30"],
    ["09", "Diffusion", "5:02"],
    ["10", "Afterimage", "4:15"],
    ["11", "Threshold", "7:03"],
    ["12", "Dissolve", "4:49"],
  ];

  return (
    <main className="w-screen h-screen pt-7 flex flex-col overflow-hidden bg-[#17171c] font-[Arial]">
      <div className="w-full h-16 shrink-0 px-6 border border-[#383842] bg-[#212129] flex items-center justify-between">
        <div className="flex items-center gap-3">
          <div className="w-10 h-10 rounded-lg bg-[#2eb88f] flex items-center justify-center text-[20px] text-white">♪</div>
          <div className="flex flex-col gap-[3px]">
            <span className="text-[14px] font-bold text-[#ebebf0]">Endless Reverie</span>
            <span className="text-[12px] text-[#7a7a8a]">Glass Prism · Mirrors</span>
          </div>
        </div>

        <div className="flex items-center gap-4">
          <div className="w-8 h-8 rounded-2xl bg-[#33333d] flex items-center justify-center text-[11px] text-[#a6a6b3]">|&lt;</div>
          <div className="w-11 h-11 rounded-[22px] bg-[#2eb88f] flex items-center justify-center text-[15px] font-bold text-white">||</div>
          <div className="w-8 h-8 rounded-2xl bg-[#33333d] flex items-center justify-center text-[11px] text-[#a6a6b3]">&gt;|</div>
        </div>

        <div className="flex items-center gap-2.5">
          <span className="text-[12px] text-[#7a7a8a]">2:14</span>
          <div className="w-[100px] h-1 rounded-sm bg-[#3d3d47]">
            <div className="w-12 h-1 rounded-sm bg-[#2eb88f]" />
          </div>
          <span className="text-[12px] text-[#7a7a8a]">4:38</span>
        </div>
      </div>

      <div className="w-full h-full grow flex overflow-hidden">
        <div className="w-[260px] h-full shrink-0 py-4 overflow-scroll border border-[#33333d] bg-[#1c1c24] flex flex-col gap-px">
          <div className="px-4 pb-2.5 flex items-center justify-between">
            <span className="text-[10px] font-bold text-[#616170]">PLAYLIST</span>
            <span className="text-[10px] text-[#616170]">12 tracks</span>
          </div>

          {tracks.map(([number, title, duration], index) => (
            <div
              key={number}
              className={`w-full h-[52px] shrink-0 px-4 flex items-center justify-between ${index === 0 ? "bg-[#243833]" : "bg-transparent"}`}
            >
              <div className="flex items-center gap-3">
                <span className={`text-[11px] ${index === 0 ? "text-[#2eb88f]" : "text-[#59596b]"}`}>{number}</span>
                <span className={`text-[13px] ${index === 0 ? "font-bold text-[#2eb88f]" : "text-[#bfbfcc]"}`}>{title}</span>
              </div>
              <span className={`text-[12px] ${index === 0 ? "text-[#2eb88f]" : "text-[#616170]"}`}>{duration}</span>
            </div>
          ))}
        </div>

        <div className="w-full h-full grow p-8 bg-[#1a1a1f] flex flex-col gap-5">
          <div className="w-full h-[160px] shrink-0 flex items-center gap-6">
            <div className="w-[152px] h-[152px] rounded-[14px] bg-[#2eb88f] flex items-center justify-center text-[52px] text-white">
              ♫
            </div>

            <div className="flex flex-col gap-1.5">
              <span className="text-[10px] font-bold text-[#616170]">ALBUM</span>
              <span
                className="text-[30px] font-bold text-[#ebebf0] cursor-pointer"
                onClick={() => setExtended(!extended)}
              >
                {extended ? "Mirrors — Extended Edition" : "Mirrors"}
              </span>
              <span className="text-[16px] text-[#2eb88f]">Glass Prism</span>
              <span className="text-[12px] text-[#7a7a8a]">2024 · Ambient · 12 tracks</span>

              <div className="pt-2 flex gap-2.5">
                <div className="w-24 h-8 rounded-2xl bg-[#2eb88f] flex items-center justify-center text-[13px] font-bold text-white">Play all</div>
                <div className="w-24 h-8 rounded-2xl bg-[#33333d] flex items-center justify-center text-[13px] text-[#b8b8c7]">Shuffle</div>
              </div>
            </div>
          </div>

          <div className="w-full h-px shrink-0 bg-[#33333d]" />

          <div className="shrink-0 flex items-center justify-between">
            <span className="text-[10px] font-bold text-[#616170]">LYRICS</span>
            <span className="text-[12px] text-[#7a7a8a]">Endless Reverie</span>
          </div>

          <div className="w-full h-full grow p-[22px] overflow-scroll rounded-xl bg-[#212129] flex flex-col gap-[7px]">
            <span className="text-[15px] text-[#858594]">Through the glass, a world apart,</span>
            <span className="text-[15px] text-[#858594]">endless echoes fill the dark.</span>
            <span className="text-[15px] text-[#858594]">Fractures in the silver light —</span>
            <div className="w-full h-2 shrink-0" />
            <span className="text-[15px] text-[#ebebf0]">Hold a breath and feel the weight</span>
            <span className="text-[15px] text-[#ebebf0]">of every word you couldn&apos;t say,</span>
            <span className="text-[15px] text-[#858594]">mirrored back in shades of grey.</span>
            <div className="w-full h-2 shrink-0" />
            <span className="text-[15px] text-[#2eb88f]">Reverie, reverie —</span>
            <span className="text-[15px] text-[#2eb88f]">I found you at the edge of sleep.</span>
            <span className="text-[15px] text-[#2eb88f]">Reverie, reverie —</span>
            <span className="text-[15px] text-[#2eb88f]">a promise too fragile to keep.</span>
            <div className="w-full h-2 shrink-0" />
            <span className="text-[15px] text-[#858594]">Soft light bends around your face,</span>
            <span className="text-[15px] text-[#858594]">I chase the outline, lose the trace.</span>
            <span className="text-[15px] text-[#858594]">The mirror holds what time erased —</span>
            <div className="w-full h-2 shrink-0" />
            <span className="text-[15px] text-[#858594]">Still you linger in the seams,</span>
            <span className="text-[15px] text-[#858594]">half-remembered, half in dreams.</span>
            <span className="text-[15px] text-[#858594]">I reach — the surface bends and gleams.</span>
            <div className="w-full h-2 shrink-0" />
            <span className="text-[15px] text-[#2eb88f]">Reverie, reverie —</span>
            <span className="text-[15px] text-[#2eb88f]">I found you at the edge of sleep.</span>
            <span className="text-[15px] text-[#2eb88f]">Reverie, reverie —</span>
            <span className="text-[15px] text-[#2eb88f]">a promise too fragile to keep.</span>
            <div className="w-full h-2 shrink-0" />
            <span className="text-[14px] text-[#59596b]">(Instrumental)</span>
            <div className="w-full h-2 shrink-0" />
            <span className="text-[15px] text-[#858594]">The glass grows cold, the echo fades,</span>
            <span className="text-[15px] text-[#858594]">and all that&apos;s left is what remains —</span>
            <span className="text-[15px] text-[#858594]">a shape of light, a broken name.</span>
            <div className="w-full h-2 shrink-0" />
            <span className="text-[15px] text-[#2eb88f]">Reverie, reverie —</span>
            <span className="text-[15px] text-[#2eb88f]">I found you at the edge of sleep.</span>
            <span className="text-[15px] text-[#2eb88f]">Reverie, reverie —</span>
            <span className="text-[15px] text-[#2eb88f]">a promise too fragile to keep.</span>
          </div>
        </div>
      </div>
    </main>
  );

    // return (
    //     <div className="p-6">
    //   <div className="grid grid-flow-col grid-rows-4 gap-4">
    //     {items.map((item) => (
    //       <div
    //         key={item}
    //         className="flex items-center justify-center h-16 w-16 bg-blue-500 text-white rounded-xl shadow"
    //       >
    //         {item}
    //       </div>
    //     ))}
    //   </div>
    // </div>
    // <div className="w-full h-screen bg-zinc-100 grid gap-2.5 p-2.5"
    //   style={{ gridTemplateColumns: '220px 1fr 1fr 1fr', gridTemplateRows: '56px 120px 1fr 44px' }}>

    //   {/* Topbar */}
    //   <div className="col-span-4 bg-white rounded-xl flex items-center justify-between px-5">
    //     <span className="text-base font-semibold text-zinc-900">Analytics</span>
    //     <div className="flex items-center gap-2">
    //       <button className="h-8 px-3.5 rounded-md bg-zinc-100 text-xs text-zinc-500">Last 30 days</button>
    //       <button className="h-8 px-3.5 rounded-md bg-zinc-900 text-xs text-white">Export</button>
    //     </div>
    //   </div>

    //   {/* Stat: Revenue */}
    //   <div className="bg-white rounded-xl p-4 flex flex-col justify-between" style={{ gridColumn: '2/3', gridRow: '2/3' }}>
    //     <span className="text-[11px] text-zinc-400">Revenue</span>
    //     <div className="flex items-center gap-2">
    //       <span className="text-[22px] font-bold text-zinc-900">$48,230</span>
    //       <span className="h-5 px-2 rounded-full bg-emerald-100 text-[10px] text-emerald-600 flex items-center">+12.4%</span>
    //     </div>
    //     <span className="text-[11px] text-zinc-300">vs $42,900 last month</span>
    //   </div>

    //   {/* Stat: Active Users */}
    //   <div className="bg-white rounded-xl p-4 flex flex-col justify-between" style={{ gridColumn: '3/4', gridRow: '2/3' }}>
    //     <span className="text-[11px] text-zinc-400">Active Users</span>
    //     <div className="flex items-center gap-2">
    //       <span className="text-[22px] font-bold text-zinc-900">8,412</span>
    //       <span className="h-5 px-2 rounded-full bg-violet-100 text-[10px] text-violet-600 flex items-center">+3.1%</span>
    //     </div>
    //     <span className="text-[11px] text-zinc-300">vs 8,160 last month</span>
    //   </div>

    //   {/* Stat: Churn */}
    //   <div className="bg-white rounded-xl p-4 flex flex-col justify-between" style={{ gridColumn: '4/5', gridRow: '2/3' }}>
    //     <span className="text-[11px] text-zinc-400">Churn Rate</span>
    //     <div className="flex items-center gap-2">
    //       <span className="text-[22px] font-bold text-zinc-900">2.3%</span>
    //       <span className="h-5 px-2 rounded-full bg-red-100 text-[10px] text-red-500 flex items-center">-0.4%</span>
    //     </div>
    //     <span className="text-[11px] text-zinc-300">vs 2.7% last month</span>
    //   </div>

    //   {/* Sidebar */}
    //   <div className="bg-white rounded-xl p-3 flex flex-col gap-1" style={{ gridColumn: '1/2', gridRow: '2/4' }}>
    //     <span className="text-[10px] tracking-wider text-zinc-400 mb-1">NAVIGATION</span>
    //     {[
    //       { label: 'Overview', active: true },
    //       { label: 'Revenue',  active: false },
    //       { label: 'Users',    active: false },
    //       { label: 'Reports',  active: false },
    //       { label: 'Settings', active: false },
    //     ].map(({ label, active }) => (
    //       <div key={label}
    //         className={`rounded-lg px-3 py-2 flex items-center gap-2 text-[13px] cursor-pointer
    //           ${active ? 'bg-zinc-900 text-white' : 'text-zinc-500 hover:bg-zinc-50'}`}>
    //         <span className="text-[10px]">▪</span>
    //         {label}
    //       </div>
    //     ))}
    //   </div>

    //   {/* Chart */}
    //   <div className="bg-white rounded-xl p-4 flex flex-col gap-3 min-h-0" style={{ gridColumn: '2/5', gridRow: '3/4' }}>
    //     <div className="flex items-center justify-between">
    //       <span className="text-sm font-semibold text-zinc-900">Revenue over time</span>
    //       <div className="flex gap-1.5">
    //         <button className="h-[26px] px-3 rounded-md bg-zinc-100 text-[11px] text-zinc-500">Monthly</button>
    //         <button className="h-[26px] px-3 rounded-md bg-zinc-900 text-[11px] text-white">Weekly</button>
    //       </div>
    //     </div>
    //     <div className="flex-1 min-h-0 flex items-end gap-1.5">
    //       {[55, 70, 45, 80, 65, 90, 75].map((h, i) => (
    //         <div key={i} className={`flex-1 rounded-sm ${i === 5 ? 'bg-violet-500' : 'bg-violet-100'}`}
    //           style={{ height: `${h}%` }} />
    //       ))}
    //     </div>
    //   </div>

    //   {/* Footer */}
    //   <div className="col-span-4 bg-white rounded-xl flex items-center justify-between px-5">
    //     <span className="text-[11px] text-zinc-400">Last synced: Apr 17, 2026 at 9:41 AM</span>
    //     <div className="flex gap-4">
    //       {['Privacy', 'Terms', 'Help'].map(l => (
    //         <span key={l} className="text-[11px] text-zinc-400 cursor-pointer hover:text-zinc-600">{l}</span>
    //       ))}
    //     </div>
    //   </div>

    // </div>
    //     <div
    //       className="h-screen w-screen bg-[#0a0a0d] p-6 grid gap-[14px]"
    //       style={{
    //         gridTemplateColumns: "1fr 1fr",
    //         gridTemplateRows: "1fr 1fr",
    //       }}
    //     >
    //       <div className="min-w-[260px] max-w-[700px] min-h-[170px] bg-[#1f1f26] p-3 flex
    //       flex-row gap-[10px]">
    //         <div className="w-[35%] h-full min-w-[90px] max-w-[180px] bg-[#f22e38]
    //         rounded-[5px]" />
    //         <div className="w-[140px] h-full min-w-[100px] max-w-[220px] bg-[#1ab8f2]
    //         rounded-[5px]" />
    //         <div className="h-full flex-grow min-w-[80px] max-w-[260px] bg-[#f2d629]
    //         rounded-[5px]" />
    //       </div>

    //       <div className="min-w-[240px] min-h-[180px] max-h-[360px] bg-[#1f1f26] p-3 flex
    //       flex-col gap-[10px]">
    //         <div className="w-full h-[25%] min-h-[42px] max-h-[90px] bg-[#52f26b]
    //         rounded-[5px]" />
    //         <div className="w-[180px] h-[64px] min-w-[120px] max-w-[260px] bg-[#ad47f5]
    //         rounded-[5px]" />
    //         <div className="w-[75%] flex-grow min-h-[50px] max-h-[130px] bg-[#ff7a1f]
    //         rounded-[5px]" />
    //       </div>

    //       <div className="min-w-[300px] min-h-[190px] bg-[#1f1f26] p-3 grid grid-cols-[140px_50%_1fr] grid-rows-[70px_45%_1fr] gap-[10px]">
    //         <div className="min-w-[90px] min-h-[46px] bg-[#00c792] rounded-[5px]" />
    //         <div className="min-w-[130px] max-w-[220px] min-h-[50px] bg-[#f5369e]
    //         rounded-[5px]" />
    //         <div className="min-w-[80px] max-w-[180px] min-h-[50px] bg-[#faeb52]
    //         rounded-[5px]" />
    //         <div className="col-start-1 col-end-3 min-h-[70px] max-h-[120px] bg-[#295cfb]
    // rounded-[5px]" />
    //         <div className="min-w-[90px] min-h-[60px] bg-[#f02e2e] rounded-[5px]" />
    //       </div>

    //       <div className="min-w-[260px] min-h-[180px] max-h-[320px] bg-[#1f1f26] p-3 flex
    //       flex-row flex-wrap gap-[10px]">
    //         <div className="w-[40%] h-[54px] min-w-[110px] max-w-[220px] bg-[#29d6e6]
    //         rounded-[5px]" />
    //         <div className="h-[54px] flex-grow min-w-[90px] max-w-[180px] bg-[#8a47fa]
    //         rounded-[5px]" />
    //         <div className="w-[150px] h-[54px] min-w-[120px] max-w-[210px] bg-[#f2d629]
    //         rounded-[5px]" />
    //         <div className="w-[65%] h-[54px] min-w-[160px] max-w-[300px] bg-[#52f26b]
    //         rounded-[5px]" />
    //       </div>
    //     </div>
    // )
}
/*
    final constraints:
        - bottom/left only have visible effects for out of flow elements
        - for in flow elements, it is relative to where it WOULD have been placed
            - very weird behavior for relative elements
        - when origin changes out of flow
            - we have to basically inverse the process (last atom gets placed first? then other atoms subsequently)
        - how do I go about this????
        - doesn't affect where next cursor starts

*/

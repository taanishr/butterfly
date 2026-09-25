#include "index.hpp"
#include "AppKit_Extensions.hpp"
#include "events.hpp"
#include "fonts.hpp"
#include "layout/sizing.hpp"
#include "layout_test_scenes.hpp"
#include <MacTypes.h>
#include <simd/vector_types.h>

static int count = 0;


auto index() -> void {
    using gui::div;
    using gui::image;
    using gui::svg;
    using gui::text;
    using runtime::Event;

    auto onClick = [](auto& node, Event& event){
        count += 1;

        if (count % 2 == 0) {
            node.color(simd_float4{1.0,0.0,0.0,1.0});
        }else {
            node.color(simd_float4{0.0,0.0,1.0,1.0});
        }
    };

    // div(gui::Size::percent(1.0), gui::Size::percent(1.0), simd_float4{1.0,1.0,1.0,1.0})
    //     .borderColor(simd_float4{0.77,0.71,1.0,1.0})
    //     .borderWidth(gui::Size::px(1.0))
    // (
    //      div(gui::Size::percent(0.2), gui::Size::percent(1.0), simd_float4{1.0,0.5,1.0,0.8})(
    //         div(gui::Size::px(60), gui::Size::px(30), simd_float4{0.498,0.0,1.0,1.0})
    //             .marginTop(30)
    //             .marginLeft(gui::Size::autoSize())
    //             .marginRight(gui::Size::autoSize())
    //             .cornerRadius(gui::Size::px(7.5))
    //             .paddingLeft(gui::Size::px(9.0))
    //             .paddingTop(gui::Size::px(4.5))
    //             .borderColor(simd_float4{0.77,0.71,1.0,1.0})
    //             .borderWidth(gui::Size::px(1.0))
    //             .addEventListener(runtime::EventType::MouseDown, onClick)
    //         (
    //             text("Startfsd")
    //                 .fontSize(gui::Size::pt(48.0))
    //                 .font(Arial)
    //                 .marginLeft(10)
    //                 .marginRight(10)
    //                 .color(simd_float4{0,0,0,1})
    //                 ,
    //             text("fsdfsdfsda   dads sdsfsdsds")
    //                 .fontSize(gui::Size::pt(48.0))
    //                 .font(Arial)
    //                 .color(simd_float4{0,0,0,1})
    //         ),
    //         div(
    //             gui::Size::px(60), gui::Size::px(30), simd_float4{0.5,0.0,0.0,1.0}
    //         ).marginTop(10)
    //      )
    // );

    // images cause like a 60mb increase in memory usage lol; need to investigate
    // turns out they just were not being downsampled

    // // seg faults fucking hell
    // using S = gui::Size;

    // // Grid demo: holy grail layout
    // div(S::percent(1.0), S::percent(1.0), simd_float4{0.95,0.95,0.95,1.0})
    //     .display(gui::Display::Grid)
    //     .gridTemplateColumns({S::fr(1), S::fr(2), S::fr(1)})
    //     .gridTemplateRows({S::px(60), S::fr(1), S::px(40)})
    //     .gridColumnGap(S::px(8))
    //     .gridRowGap(S::px(8))
    //     .padding(S::px(8))
    // (
    //     // Header — spans all 3 columns
    //     div().gridColumn(1, 4).gridRow(1, 2)
    //         .color(simd_float4{0.2,0.4,0.8,1.0})
    //         .cornerRadius(S::px(8))
    //         .display(gui::Display::Flex)
    //         .alignItems(gui::AlignItems::Center)
    //         .justifyContent(gui::JustifyContent::Center)
    //     (
    //         text("Header").fontSize(S::pt(20)).color(simd_float4{1,1,1,1})
    //     ),

    //     // Left sidebar
    //     div().gridColumn(1, 2).gridRow(2, 3)
    //         .color(simd_float4{0.9,0.9,0.95,1.0})
    //         .cornerRadius(S::px(8))
    //         .padding(S::px(12))
    //     (
    //         text("Sidebar").fontSize(S::pt(14)).color(simd_float4{0.3,0.3,0.3,1})
    //     ),

    //     // Main content
    //     div().gridColumn(2, 3).gridRow(2, 3)
    //         .color(simd_float4{1.0,1.0,1.0,1.0})
    //         .cornerRadius(S::px(8))
    //         .padding(S::px(12))
    //     (
    //         text("Main Content").fontSize(S::pt(14)).color(simd_float4{0.1,0.1,0.1,1})
    //     ),

    //     // Right sidebar
    //     div().gridColumn(3, 4).gridRow(2, 3)
    //         .color(simd_float4{0.9,0.9,0.95,1.0})
    //         .cornerRadius(S::px(8))
    //         .padding(S::px(12))
    //     (
    //         text("Panel").fontSize(S::pt(14)).color(simd_float4{0.3,0.3,0.3,1})
    //     ),

    //     // Footer — spans all 3 columns
    //     div().gridColumn(1, 4).gridRow(3, 4)
    //         .color(simd_float4{0.3,0.3,0.35,1.0})
    //         .cornerRadius(S::px(8))
    //         .display(gui::Display::Flex)
    //         .alignItems(gui::AlignItems::Center)
    //         .justifyContent(gui::JustifyContent::Center)
    //     (
    //         text("Footer").fontSize(S::pt(14)).color(simd_float4{1,1,1,1})
    //     )
    // );

   



    
//     div(gui::Size::percent(1.0), gui::Size::percent(1.0), simd_float4{1.0,1.0,1.0,1.0})
//     .display(gui::Display::Flex)
//     .paddingTop(gui::Size::px(32.0))
//     .flexDirection(gui::FlexDirection::Col)
// (
//     // Header
//     div(gui::Size::percent(1.0), gui::Size::px(56), simd_float4{1.0,1.0,1.0,1.0})
//         .display(gui::Display::Flex)
//         .alignItems(gui::AlignItems::Center)
//         .justifyContent(gui::JustifyContent::SpaceBetween)
//         .paddingLeft(gui::Size::px(16))
//         .paddingRight(gui::Size::px(16))
//         .borderColor(simd_float4{0.88,0.88,0.88,1.0})
//         .borderWidth(gui::Size::px(1.0))
//         .flexShrink(gui::Size::px(0.0))
//     (
//         // Left
//         div()
//             .color(simd_float4{0.0,0.0,0.0,0.0})
//             .display(gui::Display::Flex)
//             .alignItems(gui::AlignItems::Center)
//             .flexGap(gui::Size::px(8))
//         (
//             div(gui::Size::px(40), gui::Size::px(40), simd_float4{0.96,0.96,0.96,1.0})
//                 .cornerRadius(gui::Size::px(20))
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//                 .justifyContent(gui::JustifyContent::Center)
//             (
//                 text("=").fontSize(gui::Size::pt(18)).color(simd_float4{0.5,0.5,0.5,1.0})
//             ),
//             div()
//                 .color(simd_float4{0.96,0.96,0.96,1.0})
//                 .paddingLeft(gui::Size::px(12))
//                 .paddingRight(gui::Size::px(12))
//                 .height(gui::Size::px(32))
//                 .cornerRadius(gui::Size::px(6))
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//             (
//                 text("You: San Francisco").fontSize(gui::Size::pt(13)).color(simd_float4{0.5,0.5,0.5,1.0})
//             )
//         ),
//         // Right
//         div()
//             .color(simd_float4{0.0,0.0,0.0,0.0})
//             .display(gui::Display::Flex)
//             .alignItems(gui::AlignItems::Center)
//             .flexGap(gui::Size::px(8))
//         (
//             div()
//                 .color(simd_float4{0.96,0.96,0.96,1.0})
//                 .height(gui::Size::px(36))
//                 .paddingLeft(gui::Size::px(16))
//                 .paddingRight(gui::Size::px(16))
//                 .cornerRadius(gui::Size::px(6))
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//             (
//                 text("Import").fontSize(gui::Size::pt(13)).color(simd_float4{0.5,0.5,0.5,1.0})
//             ),
//             div()
//                 .color(simd_float4{0.1,0.1,0.1,1.0})
//                 .height(gui::Size::px(36))
//                 .paddingLeft(gui::Size::px(16))
//                 .paddingRight(gui::Size::px(16))
//                 .cornerRadius(gui::Size::px(6))
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//             (
//                 text("Add trip").fontSize(gui::Size::pt(13)).color(simd_float4{1.0,1.0,1.0,1.0})
//             )
//         )
//     ),
//     // Body
//     div()
//         .color(simd_float4{0.0,0.0,0.0,0.0})
//         .display(gui::Display::Flex)
//         .flexGrow(gui::Size::px(1))
//         .overflow(gui::Overflow::Scroll)
//     (
//         // Sidebar
//         div()
//             // .width(gui::Size::px(160.0))
//             .color(simd_float4{1.0,1.0,1.0,1.0})
//             .borderColor(simd_float4{0.88,0.88,0.88,1.0})
//             .borderWidth(gui::Size::px(1.0))
//             .display(gui::Display::Flex)
//             .flexDirection(gui::FlexDirection::Col)
//             .flexGap(gui::Size::px(4))
//             .padding(gui::Size::px(12))
//             .flexShrink(gui::Size::px(0.0))
//         (
//             text("TRIPS").fontSize(gui::Size::pt(10)).color(simd_float4{0.6,0.6,0.6,1.0}),
//             div()
//                 .width(gui::Size::percent(1.0))
//                 .color(simd_float4{0.96,0.96,0.96,1.0})
//                 .cornerRadius(gui::Size::px(8))
//                 .paddingLeft(gui::Size::px(12))
//                 .paddingRight(gui::Size::px(12))
//                 .paddingTop(gui::Size::px(8))
//                 .paddingBottom(gui::Size::px(8))
//                 .display(gui::Display::Flex)
//                 .flexDirection(gui::FlexDirection::Col)
//                 .flexGap(gui::Size::px(2))
//             (
//                 text("Tokyo & Kyoto").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0}),
//                 text("Mar 10 - Mar 24").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0})
//             ),
//             div()
//                 .width(gui::Size::percent(1.0))
//                 .color(simd_float4{1.0,1.0,1.0,1.0})
//                 .cornerRadius(gui::Size::px(8))
//                 .paddingLeft(gui::Size::px(12))
//                 .paddingRight(gui::Size::px(12))
//                 .paddingTop(gui::Size::px(8))
//                 .paddingBottom(gui::Size::px(8))
//                 .display(gui::Display::Flex)
//                 .flexDirection(gui::FlexDirection::Col)
//                 .flexGap(gui::Size::px(2))
//             (
//                 text("NYC Weekend").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0}),
//                 text("Apr 4 - Apr 7").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0})
//             ),
//             div()
//                 .width(gui::Size::percent(1.0))
//                 .color(simd_float4{1.0,1.0,1.0,1.0})
//                 .cornerRadius(gui::Size::px(8))
//                 .paddingLeft(gui::Size::px(12))
//                 .paddingRight(gui::Size::px(12))
//                 .paddingTop(gui::Size::px(8))
//                 .paddingBottom(gui::Size::px(8))
//                 .display(gui::Display::Flex)
//                 .flexDirection(gui::FlexDirection::Col)
//                 .flexGap(gui::Size::px(2))
//             (
//                 text("London + Paris").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0}),
//                 text("Jun 1 - Jun 12").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0})
//             )
//         ),
//         // Main content
//         div()
//             .color(simd_float4{0.97,0.97,0.97,1.0})
//             .flexGrow(gui::Size::px(1))
//             .display(gui::Display::Flex)
//             .flexDirection(gui::FlexDirection::Col)
//             .padding(gui::Size::px(24))
//             .flexGap(gui::Size::px(12))
//             .overflow(gui::Overflow::Scroll)
//         (
//             // Title row
//             div()
//                 .color(simd_float4{0.0,0.0,0.0,0.0})
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//                 .justifyContent(gui::JustifyContent::SpaceBetween)
//             (
//                 div()
//                     .color(simd_float4{0.0,0.0,0.0,0.0})
//                     .display(gui::Display::Flex)
//                     .flexDirection(gui::FlexDirection::Col)
//                     .flexGap(gui::Size::px(2))
//                 (
//                     text("Tokyo & Kyoto").fontSize(gui::Size::pt(20)).color(simd_float4{0.1,0.1,0.1,1.0}),
//                     text("Mar 10 - Mar 24 · 14 days").fontSize(gui::Size::pt(13)).color(simd_float4{0.5,0.5,0.5,1.0})
//                 ),
//                 div()
//                     .color(simd_float4{0.96,0.96,0.96,1.0})
//                     .paddingLeft(gui::Size::px(16))
//                     .paddingRight(gui::Size::px(16))
//                     .height(gui::Size::px(32))
//                     .cornerRadius(gui::Size::px(6))
//                     .display(gui::Display::Flex)
//                     .alignItems(gui::AlignItems::Center)
//                 (
//                     text("Edit").fontSize(gui::Size::pt(12)).color(simd_float4{0.5,0.5,0.5,1.0})
//                 )
//             ),
//             // Flight
//             div(gui::Size::percent(1.0), gui::Size::px(80), simd_float4{1.0,1.0,1.0,1.0})
//                 .cornerRadius(gui::Size::px(12))
//                 .paddingLeft(gui::Size::px(16))
//                 .paddingRight(gui::Size::px(16))
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//                 .justifyContent(gui::JustifyContent::SpaceBetween)
//             (
//                 div()
//                     .color(simd_float4{0.0,0.0,0.0,0.0})
//                     .display(gui::Display::Flex)
//                     .flexDirection(gui::FlexDirection::Col)
//                     .flexGap(gui::Size::px(4))
//                 (
//                     div()
//                         .color(simd_float4{0.0,0.0,0.0,0.0})
//                         .display(gui::Display::Flex)
//                         .flexGap(gui::Size::px(4))
//                         .alignItems(gui::AlignItems::Center)
//                     (
//                         svg("/Users/treja/projects/gui/assets/plane.svg")
//                             .width(gui::Size::px(22))
//                             .height(gui::Size::px(22)),
//                         text("SFO -> NRT").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
//                     )
//                     ,text("Mar 10 · United 837 · 11h 30m").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0})
//                 ),
//                 div()
//                     .color(simd_float4{0.93,0.88,1.0,1.0})
//                     .paddingLeft(gui::Size::px(12))
//                     .paddingRight(gui::Size::px(12))
//                     .height(gui::Size::px(24))
//                     .cornerRadius(gui::Size::px(12))
//                     .display(gui::Display::Flex)
//                     .alignItems(gui::AlignItems::Center)
//                 (
//                     text("[Flight]").fontSize(gui::Size::pt(11)).color(simd_float4{0.4,0.3,1.0,1.0})
//                 )
//             ),
//             // Hotel
//             div(gui::Size::percent(1.0), gui::Size::px(80), simd_float4{1.0,1.0,1.0,1.0})
//                 .cornerRadius(gui::Size::px(12))
//                 .paddingLeft(gui::Size::px(16))
//                 .paddingRight(gui::Size::px(16))
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//                 .justifyContent(gui::JustifyContent::SpaceBetween)
//             (
//                 div()
//                     .color(simd_float4{0.0,0.0,0.0,0.0})
//                     .display(gui::Display::Flex)
//                     .flexDirection(gui::FlexDirection::Col)
//                     .flexGap(gui::Size::px(4))
//                 (
//                     text("Park Hyatt Tokyo").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0}),
//                     text("Mar 11 - Mar 17 · 6 nights").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0})
//                 ),
//                 div()
//                     .color(simd_float4{0.85,0.93,1.0,1.0})
//                     .paddingLeft(gui::Size::px(12))
//                     .paddingRight(gui::Size::px(12))
//                     .height(gui::Size::px(24))
//                     .cornerRadius(gui::Size::px(12))
//                     .display(gui::Display::Flex)
//                     .alignItems(gui::AlignItems::Center)
//                 (
//                     text("[Hotel]").fontSize(gui::Size::pt(11)).color(simd_float4{0.2,0.6,0.9,1.0})
//                 )
//             ),
//             // Train
//             div(gui::Size::percent(1.0), gui::Size::px(80), simd_float4{1.0,1.0,1.0,1.0})
//                 .cornerRadius(gui::Size::px(12))
//                 .paddingLeft(gui::Size::px(16))
//                 .paddingRight(gui::Size::px(16))
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//                 .justifyContent(gui::JustifyContent::SpaceBetween)
//             (
//                 div()
//                     .color(simd_float4{0.0,0.0,0.0,0.0})
//                     .display(gui::Display::Flex)
//                     .flexDirection(gui::FlexDirection::Col)
//                     .flexGap(gui::Size::px(4))
//                 (
//                     text("Tokyo -> Kyoto").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0}),
//                     text("Mar 17 · Shinkansen · 2h 15m").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0})
//                 ),
//                 div()
//                     .color(simd_float4{0.85,1.0,0.9,1.0})
//                     .paddingLeft(gui::Size::px(12))
//                     .paddingRight(gui::Size::px(12))
//                     .height(gui::Size::px(24))
//                     .cornerRadius(gui::Size::px(12))
//                     .display(gui::Display::Flex)
//                     .alignItems(gui::AlignItems::Center)
//                 (
//                     text("[Train]").fontSize(gui::Size::pt(11)).color(simd_float4{0.1,0.7,0.4,1.0})
//                 )
//             )
//         )
//         // Detail panel
//         ,div()
//             // .width(gui::Size::px(200.0))
//             .color(simd_float4{1.0,1.0,1.0,1.0})
//             .borderColor(simd_float4{0.88,0.88,0.88,1.0})
//             .borderWidth(gui::Size::px(1.0))
//             .display(gui::Display::Flex)
//             .flexDirection(gui::FlexDirection::Col)
//             .flexGap(gui::Size::px(16))
//             .padding(gui::Size::px(16))
//             .flexShrink(gui::Size::px(0.0))
//             .overflow(gui::Overflow::Scroll)
//         (
//             div()
//                 .color(simd_float4{0.0,0.0,0.0,0.0})
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//                 .justifyContent(gui::JustifyContent::SpaceBetween)
//             (
//                 text("Flight Details").fontSize(gui::Size::pt(14)).color(simd_float4{0.1,0.1,0.1,1.0}),
//                 text("x").fontSize(gui::Size::pt(13)).color(simd_float4{0.6,0.6,0.6,1.0})
//             ),
//             div()
//                 .color(simd_float4{0.0,0.0,0.0,0.0})
//                 .display(gui::Display::Flex)
//                 .flexDirection(gui::FlexDirection::Col)
//                 .flexGap(gui::Size::px(12))
//             (
//                 div()
//                     .color(simd_float4{0.0,0.0,0.0,0.0})
//                     .display(gui::Display::Flex)
//                     .justifyContent(gui::JustifyContent::SpaceBetween)
//                 (
//                     text("Flight").fontSize(gui::Size::pt(13)).color(simd_float4{0.6,0.6,0.6,1.0}),
//                     text("UA 837").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
//                 ),
//                 div()
//                     .color(simd_float4{0.0,0.0,0.0,0.0})
//                     .display(gui::Display::Flex)
//                     .justifyContent(gui::JustifyContent::SpaceBetween)
//                 (
//                     text("Departs").fontSize(gui::Size::pt(13)).color(simd_float4{0.6,0.6,0.6,1.0}),
//                     text("10:45 AM").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
//                 ),
//                 div()
//                     .color(simd_float4{0.0,0.0,0.0,0.0})
//                     .display(gui::Display::Flex)
//                     .justifyContent(gui::JustifyContent::SpaceBetween)
//                 (
//                     text("Arrives").fontSize(gui::Size::pt(13)).color(simd_float4{0.6,0.6,0.6,1.0}),
//                     text("3:15 PM +1").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
//                 ),
//                 div()
//                     .color(simd_float4{0.0,0.0,0.0,0.0})
//                     .display(gui::Display::Flex)
//                     .justifyContent(gui::JustifyContent::SpaceBetween)
//                 (
//                     text("Duration").fontSize(gui::Size::pt(13)).color(simd_float4{0.6,0.6,0.6,1.0}),
//                     text("11h 30m").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
//                 ),
//                 div()
//                     .color(simd_float4{0.0,0.0,0.0,0.0})
//                     .display(gui::Display::Flex)
//                     .justifyContent(gui::JustifyContent::SpaceBetween)
//                 (
//                     text("Seat").fontSize(gui::Size::pt(13)).color(simd_float4{0.6,0.6,0.6,1.0}),
//                     text("42A").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
//                 ),
//                 div()
//                     .color(simd_float4{0.0,0.0,0.0,0.0})
//                     .display(gui::Display::Flex)
//                     .justifyContent(gui::JustifyContent::SpaceBetween)
//                 (
//                     text("Class").fontSize(gui::Size::pt(13)).color(simd_float4{0.6,0.6,0.6,1.0}),
//                     text("Economy").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
//                 )
//             ),
//             div()
//                 .width(gui::Size::percent(1.0))
//                 .color(simd_float4{0.97,0.97,0.97,1.0})
//                 .cornerRadius(gui::Size::px(8))
//                 .padding(gui::Size::px(12))
//                 .display(gui::Display::Flex)
//                 .flexDirection(gui::FlexDirection::Col)
//                 .flexGap(gui::Size::px(4))
//             (
//                 text("Local time at destination").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0}),
//                 text("3:15 PM JST (UTC+9)").fontSize(gui::Size::pt(13)).color(simd_float4{0.1,0.1,0.1,1.0})
//             )
//         )
//     )
// );

//    div(gui::Size::percent(1.0), gui::Size::percent(1.0), simd_float4{0.95,0.95,0.95,1.0})
//         .display(gui::Display::Flex)
//         .alignItems(gui::AlignItems::Center)
//         .justifyContent(gui::JustifyContent::Center)
//     (
//         div()
//             .color(simd_float4{1.0,1.0,1.0,1.0})
//             .cornerRadius(gui::Size::px(16))
//             .padding(gui::Size::px(12))
//             .display(gui::Display::Flex)
//             .flexDirection(gui::FlexDirection::Col)
//             .alignItems(gui::AlignItems::Center) // expands maxWidth for... some reason?
//             .flexGap(gui::Size::px(12))
//         (
//             image("/Users/treja/Downloads/sf90.jpg", gui::Size::px(80), gui::Size::px(80))
//                 .cornerRadius(gui::Size::percent(0.5))
//             ,text("Sarah Johnson")
//                 .fontSize(gui::Size::pt(18))
//                 .color(simd_float4{0.1,0.1,0.1,1.0})
//             ,text("Product Designer @ Figma")
//                 .fontSize(gui::Size::pt(13))
//                 .color(simd_float4{0.5,0.5,0.5,1.0})
//             ,//
//             div()
//                 .width(gui::Size::percent(1.0)) // this is broken now... percent sizing leads to this being full width of grandparent when html doesnt do that
//                 .height(gui::Size::px(50))
//                 .color(simd_float4{0.0,0.0,0.0,0.0})
//                 .display(gui::Display::Flex)
//                 .justifyContent(gui::JustifyContent::SpaceAround)
//             (
//                 div(gui::Size::px(70), gui::Size::px(50), simd_float4{0.0,0.0,0.0,0.0})
//                     .display(gui::Display::Flex)
//                     .flexDirection(gui::FlexDirection::Col)
//                     .alignItems(gui::AlignItems::Center)
//                 (
//                     text("284").fontSize(gui::Size::pt(16)).color(simd_float4{0.1,0.1,0.1,1.0})
//                     ,text("Posts").fontSize(gui::Size::pt(12)).color(simd_float4{0.5,0.5,0.5,1.0})
//                 )
//                 ,div(gui::Size::px(70), gui::Size::px(50), simd_float4{0.0,0.0,0.0,0.0})
//                     .display(gui::Display::Flex)
//                     .flexDirection(gui::FlexDirection::Col)
//                     .alignItems(gui::AlignItems::Center)
//                 (
//                     text("12.4k").fontSize(gui::Size::pt(16)).color(simd_float4{0.1,0.1,0.1,1.0}),
//                     text("Followers").fontSize(gui::Size::pt(12)).color(simd_float4{0.5,0.5,0.5,1.0})
//                 ),
//                 div(gui::Size::px(70), gui::Size::px(50), simd_float4{0.0,0.0,0.0,0.0})
//                     .display(gui::Display::Flex)
//                     .flexDirection(gui::FlexDirection::Col)
//                     .alignItems(gui::AlignItems::Center)
//                 (
//                     text("891").fontSize(gui::Size::pt(16)).color(simd_float4{0.1,0.1,0.1,1.0}),
//                     text("Following").fontSize(gui::Size::pt(12)).color(simd_float4{0.5,0.5,0.5,1.0})
//                 )
//             )
//             ,
//             div(gui::Size::px(120), gui::Size::px(40), simd_float4{0.4,0.3,1.0,1.0})
//                 .cornerRadius(gui::Size::px(20))
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//                 .justifyContent(gui::JustifyContent::Center)
//                 .addEventListener(runtime::EventType::MouseDown, onClick)
//             (
//                 text("Follow").fontSize(gui::Size::pt(14)).color(simd_float4{1.0,1.0,1.0,1.0})
//             )
//         )
//     );



// // Dashboard layout: analytics overview
// div(gui::Size::percent(1.0), gui::Size::percent(1.0), simd_float4{0.96,0.96,0.97,1.0})
//     .display(gui::Display::Grid)
//     .gridTemplateColumns({gui::Size::px(220), gui::Size::fr(1), gui::Size::fr(1), gui::Size::fr(1)})
//     .gridTemplateRows({gui::Size::px(56), gui::Size::px(120), gui::Size::fr(1), gui::Size::px(44)})
//     .gridColumnGap(gui::Size::px(10))
//     .gridRowGap(gui::Size::px(10))
//     .padding(gui::Size::px(10))
//     .paddingTop(gui::Size::px(32))
// (
//     // ── Topbar: columns 1-5, row 1 ──
//     div().gridColumn(1, 5).gridRow(1, 2)
//         .color(simd_float4{1.0,1.0,1.0,1.0})
//         .cornerRadius(gui::Size::px(10))
//         .display(gui::Display::Flex)
//         .alignItems(gui::AlignItems::Center)
//         .justifyContent(gui::JustifyContent::SpaceBetween)
//         .paddingLeft(gui::Size::px(20))
//         .paddingRight(gui::Size::px(20))
//     (
//         text("Analytics").fontSize(gui::Size::pt(16)).color(simd_float4{0.1,0.1,0.1,1.0}),
//         div()
//             .color(simd_float4{0.0,0.0,0.0,0.0})
//             .display(gui::Display::Flex)
//             .alignItems(gui::AlignItems::Center)
//             .flexGap(gui::Size::px(8))
//         (
//             div()
//                 .color(simd_float4{0.96,0.96,0.96,1.0})
//                 .height(gui::Size::px(32))
//                 .paddingLeft(gui::Size::px(14))
//                 .paddingRight(gui::Size::px(14))
//                 .cornerRadius(gui::Size::px(6))
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//             (
//                 text("Last 30 days").fontSize(gui::Size::pt(12)).color(simd_float4{0.5,0.5,0.5,1.0})
//             ),
//             div()
//                 .color(simd_float4{0.1,0.1,0.1,1.0})
//                 .height(gui::Size::px(32))
//                 .paddingLeft(gui::Size::px(14))
//                 .paddingRight(gui::Size::px(14))
//                 .cornerRadius(gui::Size::px(6))
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//             (
//                 text("Export").fontSize(gui::Size::pt(12)).color(simd_float4{1.0,1.0,1.0,1.0})
//             )
//         )
//     ),

//     // ── Stat card: Revenue ──
//     div().gridColumn(2, 3).gridRow(2, 3)
//         .color(simd_float4{1.0,1.0,1.0,1.0})
//         .cornerRadius(gui::Size::px(10))
//         .padding(gui::Size::px(16))
//         .display(gui::Display::Flex)
//         .flexDirection(gui::FlexDirection::Col)
//         .justifyContent(gui::JustifyContent::SpaceBetween)
//     (
//         text("Revenue").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0}),
//         div()
//             .color(simd_float4{0.0,0.0,0.0,0.0})
//             .display(gui::Display::Flex)
//             .alignItems(gui::AlignItems::Center)
//             .flexGap(gui::Size::px(8))
//         (
//             text("$48,230").fontSize(gui::Size::pt(22)).color(simd_float4{0.1,0.1,0.1,1.0}),
//             div()
//                 .color(simd_float4{0.85,1.0,0.9,1.0})
//                 .paddingLeft(gui::Size::px(8))
//                 .paddingRight(gui::Size::px(8))
//                 .height(gui::Size::px(20))
//                 .cornerRadius(gui::Size::px(10))
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//             (
//                 text("+12.4%").fontSize(gui::Size::pt(10)).color(simd_float4{0.1,0.7,0.4,1.0})
//             )
//         ),
//         text("vs $42,900 last month").fontSize(gui::Size::pt(11)).color(simd_float4{0.7,0.7,0.7,1.0})
//     ),

//     // ── Stat card: Users ──
//     div().gridColumn(3, 4).gridRow(2, 3)
//         .color(simd_float4{1.0,1.0,1.0,1.0})
//         .cornerRadius(gui::Size::px(10))
//         .padding(gui::Size::px(16))
//         .display(gui::Display::Flex)
//         .flexDirection(gui::FlexDirection::Col)
//         .justifyContent(gui::JustifyContent::SpaceBetween)
//     (
//         text("Active Users").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0}),
//         div()
//             .color(simd_float4{0.0,0.0,0.0,0.0})
//             .display(gui::Display::Flex)
//             .alignItems(gui::AlignItems::Center)
//             .flexGap(gui::Size::px(8))
//         (
//             text("8,412").fontSize(gui::Size::pt(22)).color(simd_float4{0.1,0.1,0.1,1.0}),
//             div()
//                 .color(simd_float4{0.93,0.88,1.0,1.0})
//                 .paddingLeft(gui::Size::px(8))
//                 .paddingRight(gui::Size::px(8))
//                 .height(gui::Size::px(20))
//                 .cornerRadius(gui::Size::px(10))
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//             (
//                 text("+3.1%").fontSize(gui::Size::pt(10)).color(simd_float4{0.4,0.3,1.0,1.0})
//             )
//         ),
//         text("vs 8,160 last month").fontSize(gui::Size::pt(11)).color(simd_float4{0.7,0.7,0.7,1.0})
//     ),

//     // ── Stat card: Churn ──
//     div().gridColumn(4, 5).gridRow(2, 3)
//         .color(simd_float4{1.0,1.0,1.0,1.0})
//         .cornerRadius(gui::Size::px(10))
//         .padding(gui::Size::px(16))
//         .display(gui::Display::Flex)
//         .flexDirection(gui::FlexDirection::Col)
//         .justifyContent(gui::JustifyContent::SpaceBetween)
//     (
//         text("Churn Rate").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0}),
//         div()
//             .color(simd_float4{0.0,0.0,0.0,0.0})
//             .display(gui::Display::Flex)
//             .alignItems(gui::AlignItems::Center)
//             .flexGap(gui::Size::px(8))
//         (
//             text("2.3%").fontSize(gui::Size::pt(22)).color(simd_float4{0.1,0.1,0.1,1.0}),
//             div()
//                 .color(simd_float4{1.0,0.9,0.88,1.0})
//                 .paddingLeft(gui::Size::px(8))
//                 .paddingRight(gui::Size::px(8))
//                 .height(gui::Size::px(20))
//                 .cornerRadius(gui::Size::px(10))
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//             (
//                 text("-0.4%").fontSize(gui::Size::pt(10)).color(simd_float4{0.9,0.3,0.2,1.0})
//             )
//         ),
//         text("vs 2.7% last month").fontSize(gui::Size::pt(11)).color(simd_float4{0.7,0.7,0.7,1.0})
//     ),

//     // ── Nav sidebar: column 1, rows 2-4 ──
//     div().gridColumn(1, 2).gridRow(2, 4)
//         .color(simd_float4{1.0,1.0,1.0,1.0})
//         .cornerRadius(gui::Size::px(10))
//         .padding(gui::Size::px(12))
//         .display(gui::Display::Flex)
//         .flexDirection(gui::FlexDirection::Col)
//         .flexGap(gui::Size::px(4))
//     (
//         text("NAVIGATION").fontSize(gui::Size::pt(10)).color(simd_float4{0.6,0.6,0.6,1.0}),
//         div()
//             .width(gui::Size::percent(1.0))
//             .color(simd_float4{0.1,0.1,0.1,1.0})
//             .cornerRadius(gui::Size::px(8))
//             .paddingLeft(gui::Size::px(12))
//             .paddingRight(gui::Size::px(12))
//             .paddingTop(gui::Size::px(8))
//             .paddingBottom(gui::Size::px(8))
//             .display(gui::Display::Flex)
//             .alignItems(gui::AlignItems::Center)
//             .flexGap(gui::Size::px(8))
//         (
//             text("▪").fontSize(gui::Size::pt(10)).color(simd_float4{1.0,1.0,1.0,1.0}),
//             text("Overview").fontSize(gui::Size::pt(13)).color(simd_float4{1.0,1.0,1.0,1.0})
//         ),
//         div()
//             .width(gui::Size::percent(1.0))
//             .color(simd_float4{1.0,1.0,1.0,1.0})
//             .cornerRadius(gui::Size::px(8))
//             .paddingLeft(gui::Size::px(12))
//             .paddingRight(gui::Size::px(12))
//             .paddingTop(gui::Size::px(8))
//             .paddingBottom(gui::Size::px(8))
//             .display(gui::Display::Flex)
//             .alignItems(gui::AlignItems::Center)
//             .flexGap(gui::Size::px(8))
//         (
//             text("▪").fontSize(gui::Size::pt(10)).color(simd_float4{0.6,0.6,0.6,1.0}),
//             text("Revenue").fontSize(gui::Size::pt(13)).color(simd_float4{0.3,0.3,0.3,1.0})
//         ),
//         div()
//             .width(gui::Size::percent(1.0))
//             .color(simd_float4{1.0,1.0,1.0,1.0})
//             .cornerRadius(gui::Size::px(8))
//             .paddingLeft(gui::Size::px(12))
//             .paddingRight(gui::Size::px(12))
//             .paddingTop(gui::Size::px(8))
//             .paddingBottom(gui::Size::px(8))
//             .display(gui::Display::Flex)
//             .alignItems(gui::AlignItems::Center)
//             .flexGap(gui::Size::px(8))
//         (
//             text("▪").fontSize(gui::Size::pt(10)).color(simd_float4{0.6,0.6,0.6,1.0}),
//             text("Users").fontSize(gui::Size::pt(13)).color(simd_float4{0.3,0.3,0.3,1.0})
//         ),
//         div()
//             .width(gui::Size::percent(1.0))
//             .color(simd_float4{1.0,1.0,1.0,1.0})
//             .cornerRadius(gui::Size::px(8))
//             .paddingLeft(gui::Size::px(12))
//             .paddingRight(gui::Size::px(12))
//             .paddingTop(gui::Size::px(8))
//             .paddingBottom(gui::Size::px(8))
//             .display(gui::Display::Flex)
//             .alignItems(gui::AlignItems::Center)
//             .flexGap(gui::Size::px(8))
//         (
//             text("▪").fontSize(gui::Size::pt(10)).color(simd_float4{0.6,0.6,0.6,1.0}),
//             text("Reports").fontSize(gui::Size::pt(13)).color(simd_float4{0.3,0.3,0.3,1.0})
//         ),
//         div()
//             .width(gui::Size::percent(1.0))
//             .color(simd_float4{1.0,1.0,1.0,1.0})
//             .cornerRadius(gui::Size::px(8))
//             .paddingLeft(gui::Size::px(12))
//             .paddingRight(gui::Size::px(12))
//             .paddingTop(gui::Size::px(8))
//             .paddingBottom(gui::Size::px(8))
//             .display(gui::Display::Flex)
//             .alignItems(gui::AlignItems::Center)
//             .flexGap(gui::Size::px(8))
//         (
//             text("▪").fontSize(gui::Size::pt(10)).color(simd_float4{0.6,0.6,0.6,1.0}),
//             text("Settings").fontSize(gui::Size::pt(13)).color(simd_float4{0.3,0.3,0.3,1.0})
//         )
//     ),

//     // ── Chart area: columns 2-4, row 3 ──
//     div().gridColumn(2, 5).gridRow(3, 4)
//         .color(simd_float4{1.0,1.0,1.0,1.0})
//         .cornerRadius(gui::Size::px(10))
//         .padding(gui::Size::px(16))
//         .display(gui::Display::Flex)
//         .flexDirection(gui::FlexDirection::Col)
//         .flexGap(gui::Size::px(12))
//     (
//         div()
//             .color(simd_float4{0.0,0.0,0.0,0.0})
//             .display(gui::Display::Flex)
//             .alignItems(gui::AlignItems::Center)
//             .justifyContent(gui::JustifyContent::SpaceBetween)
//         (
//             text("Revenue over time").fontSize(gui::Size::pt(14)).color(simd_float4{0.1,0.1,0.1,1.0}),
//             div()
//                 .color(simd_float4{0.0,0.0,0.0,0.0})
//                 .display(gui::Display::Flex)
//                 .flexGap(gui::Size::px(6))
//         (
//                 div()
//                     .color(simd_float4{0.96,0.96,0.96,1.0})
//                     .height(gui::Size::px(26))
//                     .paddingLeft(gui::Size::px(12))
//                     .paddingRight(gui::Size::px(12))
//                     .cornerRadius(gui::Size::px(6))
//                     .display(gui::Display::Flex)
//                     .alignItems(gui::AlignItems::Center)
//                 (
//                     text("Monthly").fontSize(gui::Size::pt(11)).color(simd_float4{0.5,0.5,0.5,1.0})
//                 ),
//                 div()
//                     .color(simd_float4{0.1,0.1,0.1,1.0})
//                     .height(gui::Size::px(26))
//                     .paddingLeft(gui::Size::px(12))
//                     .paddingRight(gui::Size::px(12))
//                     .cornerRadius(gui::Size::px(6))
//                     .display(gui::Display::Flex)
//                     .alignItems(gui::AlignItems::Center)
//                 (
//                     text("Weekly").fontSize(gui::Size::pt(11)).color(simd_float4{1.0,1.0,1.0,1.0})
//                 )
//             )
//         ),
//         div()
//             .color(simd_float4{0.0,0.0,0.0,0.0})
//             .flexGrow(gui::Size::px(1))
//             .display(gui::Display::Flex)
//             .alignItems(gui::AlignItems::FlexEnd)
//             .flexGap(gui::Size::px(6))
//         (
//             div().flexGrow(gui::Size::px(1)).height(gui::Size::percent(0.61)).color(simd_float4{0.88,0.88,0.98,1.0}).cornerRadius(gui::Size::px(4))(),
//             div().flexGrow(gui::Size::px(1)).height(gui::Size::percent(0.78)).color(simd_float4{0.88,0.88,0.98,1.0}).cornerRadius(gui::Size::px(4))(),
//             div().flexGrow(gui::Size::px(1)).height(gui::Size::percent(0.50)).color(simd_float4{0.88,0.88,0.98,1.0}).cornerRadius(gui::Size::px(4))(),
//             div().flexGrow(gui::Size::px(1)).height(gui::Size::percent(0.89)).color(simd_float4{0.88,0.88,0.98,1.0}).cornerRadius(gui::Size::px(4))(),
//             div().flexGrow(gui::Size::px(1)).height(gui::Size::percent(0.72)).color(simd_float4{0.88,0.88,0.98,1.0}).cornerRadius(gui::Size::px(4))(),
//             div().flexGrow(gui::Size::px(1)).height(gui::Size::percent(1.00)).color(simd_float4{0.4,0.3,1.0,1.0}).cornerRadius(gui::Size::px(4))(),
//             div().flexGrow(gui::Size::px(1)).height(gui::Size::percent(0.83)).color(simd_float4{0.88,0.88,0.98,1.0}).cornerRadius(gui::Size::px(4))()
//         )
//     ),

//     // ── Footer: all columns, row 4 ──
//     div().gridColumn(1, 5).gridRow(4, 5)
//         .color(simd_float4{1.0,1.0,1.0,1.0})
//         .cornerRadius(gui::Size::px(10))
//         .display(gui::Display::Flex)
//         .alignItems(gui::AlignItems::Center)
//         .justifyContent(gui::JustifyContent::SpaceBetween)
//         .paddingLeft(gui::Size::px(20))
//         .paddingRight(gui::Size::px(20))
//     (
//         text("Last synced: Apr 17, 2026 at 9:41 AM").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0}),
//         div()
//             .color(simd_float4{0.0,0.0,0.0,0.0})
//             .display(gui::Display::Flex)
//             .alignItems(gui::AlignItems::Center)
//             .flexGap(gui::Size::px(16))
//         (
//             text("Privacy").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0}),
//             text("Terms").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0}),
//             text("Help").fontSize(gui::Size::pt(11)).color(simd_float4{0.6,0.6,0.6,1.0})
//         )
//     )
// );

// using S = gui::Size;
// div(S::percent(1.0), S::percent(1.0), simd_float4{0.95,0.96,0.98,1.0})
//         .display(gui::Display::Grid)
//         .gridTemplateColumns({S::px(250), S::fr(1), S::px(300)})
//         .gridTemplateRows({S::px(68), S::fr(1)})
//         .gridColumnGap(S::px(14))
//         .gridRowGap(S::px(14))
//         .paddingLeft(S::px(16))
//         .paddingRight(S::px(16))
//         .paddingBottom(S::px(16))
//         .paddingTop(S::px(28))
//     (
//         div().gridColumn(1, 4).gridRow(1, 2)
//             .color(simd_float4{1.0,1.0,1.0,1.0})
//             .cornerRadius(S::px(10))
//             .borderColor(simd_float4{0.86,0.88,0.91,1.0})
//             .borderWidth(S::px(1))
//             .display(gui::Display::Flex)
//             .alignItems(gui::AlignItems::Center)
//             .justifyContent(gui::JustifyContent::SpaceBetween)
//             .paddingLeft(S::px(18))
//             .paddingRight(S::px(18))
//         (
//             div()
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//                 .flexGap(S::px(12))
//             (
//                 div(S::px(34), S::px(34), simd_float4{0.04,0.4,0.67,1.0})
//                     .cornerRadius(S::px(6))
//                     .display(gui::Display::Flex)
//                     .alignItems(gui::AlignItems::Center)
//                     .justifyContent(gui::JustifyContent::Center)
//                 (
//                     text("in").fontSize(S::pt(18)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
//                 ),
//                 div()
//                     .color(simd_float4{0.94,0.97,1.0,1.0})
//                     .height(S::px(38))
//                     .paddingLeft(S::px(14))
//                     .paddingRight(S::px(64))
//                     .cornerRadius(S::px(19))
//                     .display(gui::Display::Flex)
//                     .alignItems(gui::AlignItems::Center)
//                 (
//                     text("Search").fontSize(S::pt(13)).color(simd_float4{0.41,0.49,0.56,1.0})
//                 )
//             ),
//             div()
//                 .display(gui::Display::Flex)
//                 .alignItems(gui::AlignItems::Center)
//                 .flexGap(S::px(10))
//             (
//                 div().paddingLeft(S::px(12)).paddingRight(S::px(12)).height(S::px(34)).cornerRadius(S::px(17)).display(gui::Display::Flex).alignItems(gui::AlignItems::Center)
//                 (
//                     text("Home").fontSize(S::pt(13)).color(simd_float4{0.12,0.16,0.2,1.0})
//                 ),
//                 div().paddingLeft(S::px(12)).paddingRight(S::px(12)).height(S::px(34)).cornerRadius(S::px(17)).display(gui::Display::Flex).alignItems(gui::AlignItems::Center)
//                 (
//                     text("My Network").fontSize(S::pt(13)).color(simd_float4{0.42,0.48,0.54,1.0})
//                 ),
//                 div().paddingLeft(S::px(12)).paddingRight(S::px(12)).height(S::px(34)).cornerRadius(S::px(17)).display(gui::Display::Flex).alignItems(gui::AlignItems::Center)
//                 (
//                     text("Jobs").fontSize(S::pt(13)).color(simd_float4{0.42,0.48,0.54,1.0})
//                 ),
//                 div().paddingLeft(S::px(12)).paddingRight(S::px(12)).height(S::px(34)).cornerRadius(S::px(17)).display(gui::Display::Flex).alignItems(gui::AlignItems::Center)
//                 (
//                     text("Messages").fontSize(S::pt(13)).color(simd_float4{0.42,0.48,0.54,1.0})
//                 ),
//                 div().paddingLeft(S::px(14)).paddingRight(S::px(14)).height(S::px(36)).color(simd_float4{0.1,0.45,0.77,1.0}).cornerRadius(S::px(18)).display(gui::Display::Flex).alignItems(gui::AlignItems::Center)
//                 (
//                     text("Try Premium").fontSize(S::pt(12)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
//                 )
//             )
//         ),

//         div().gridColumn(1, 2).gridRow(2, 3)
//             .display(gui::Display::Flex)
//             .flexDirection(gui::FlexDirection::Col)
//             .flexGap(S::px(14))
//         (
//             div()
//                 .color(simd_float4{1.0,1.0,1.0,1.0})
//                 .cornerRadius(S::px(10))
//                 .borderColor(simd_float4{0.86,0.88,0.91,1.0})
//                 .borderWidth(S::px(1))
//                 .display(gui::Display::Flex)
//                 .flexDirection(gui::FlexDirection::Col)
//             (
//                 div()
//                     .height(S::px(74))
//                     .color(simd_float4{0.72,0.84,0.95,1.0})
//                     .cornerRadius(S::px(10))(),
//                 div()
//                     .paddingLeft(S::px(18))
//                     .paddingRight(S::px(18))
//                     .paddingBottom(S::px(18))
//                     .display(gui::Display::Flex)
//                     .flexDirection(gui::FlexDirection::Col)
//                     .alignItems(gui::AlignItems::Center)
//                     .flexGap(S::px(8))
//                 (
//                     div(S::px(84), S::px(84), simd_float4{0.1,0.45,0.77,1.0})
//                         .cornerRadius(S::px(42))
//                         .marginTop(S::px(-42))
//                         .display(gui::Display::Flex)
//                         .alignItems(gui::AlignItems::Center)
//                         .justifyContent(gui::JustifyContent::Center)
//                     (
//                         text("TR").fontSize(S::pt(24)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
//                     ),
//                     text("Taanish Reja").fontSize(S::pt(18)).font(ArialBold).color(simd_float4{0.09,0.12,0.16,1.0}),
//                     text("Building a GPU-native UI kit").fontSize(S::pt(12)).color(simd_float4{0.39,0.45,0.51,1.0}),
//                     div().height(S::px(1)).color(simd_float4{0.91,0.93,0.95,1.0})(),
//                     div()
//                         .display(gui::Display::Flex)
//                         .justifyContent(gui::JustifyContent::SpaceBetween)
//                     (
//                         text("Profile viewers").fontSize(S::pt(12)).color(simd_float4{0.39,0.45,0.51,1.0}),
//                         text("128").fontSize(S::pt(12)).font(ArialBold).color(simd_float4{0.1,0.45,0.77,1.0})
//                     ),
//                     div()
//                         .display(gui::Display::Flex)
//                         .justifyContent(gui::JustifyContent::SpaceBetween)
//                     (
//                         text("Post impressions").fontSize(S::pt(12)).color(simd_float4{0.39,0.45,0.51,1.0}),
//                         text("2,481").fontSize(S::pt(12)).font(ArialBold).color(simd_float4{0.1,0.45,0.77,1.0})
//                     )
//                 )
//             ),
//             div()
//                 .color(simd_float4{1.0,1.0,1.0,1.0})
//                 .cornerRadius(S::px(10))
//                 .borderColor(simd_float4{0.86,0.88,0.91,1.0})
//                 .borderWidth(S::px(1))
//                 .padding(S::px(16))
//                 .display(gui::Display::Flex)
//                 .flexDirection(gui::FlexDirection::Col)
//                 .flexGap(S::px(10))
//             (
//                 text("Recent").fontSize(S::pt(13)).font(ArialBold).color(simd_float4{0.12,0.16,0.2,1.0}),
//                 text("#metal").fontSize(S::pt(12)).color(simd_float4{0.39,0.45,0.51,1.0}),
//                 text("#layoutengines").fontSize(S::pt(12)).color(simd_float4{0.39,0.45,0.51,1.0}),
//                 text("#cpp").fontSize(S::pt(12)).color(simd_float4{0.39,0.45,0.51,1.0}),
//                 text("#rendering").fontSize(S::pt(12)).color(simd_float4{0.39,0.45,0.51,1.0})
//             )
//         ),

//         div().gridColumn(2, 3).gridRow(2, 3)
//             .display(gui::Display::Flex)
//             .flexDirection(gui::FlexDirection::Col)
//             .flexGap(S::px(14))
//         (
//             div()
//                 .color(simd_float4{1.0,1.0,1.0,1.0})
//                 .cornerRadius(S::px(10))
//                 .borderColor(simd_float4{0.86,0.88,0.91,1.0})
//                 .borderWidth(S::px(1))
//                 .padding(S::px(16))
//                 .display(gui::Display::Flex)
//                 .flexDirection(gui::FlexDirection::Col)
//                 .flexGap(S::px(14))
//             (
//                 div()
//                     .display(gui::Display::Flex)
//                     .alignItems(gui::AlignItems::Center)
//                     .flexGap(S::px(12))
//                 (
//                     div(S::px(52), S::px(52), simd_float4{0.13,0.52,0.81,1.0})
//                         .cornerRadius(S::px(26))
//                         .display(gui::Display::Flex)
//                         .alignItems(gui::AlignItems::Center)
//                         .justifyContent(gui::JustifyContent::Center)
//                     (
//                         text("TR").fontSize(S::pt(15)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
//                     ),
//                     div()
//                         .color(simd_float4{0.96,0.97,0.98,1.0})
//                         .borderColor(simd_float4{0.84,0.87,0.9,1.0})
//                         .borderWidth(S::px(1))
//                         .cornerRadius(S::px(22))
//                         .height(S::px(44))
//                         .paddingLeft(S::px(18))
//                         .paddingRight(S::px(18))
//                         .display(gui::Display::Flex)
//                         .alignItems(gui::AlignItems::Center)
//                         .flexGrow(S::px(1))
//                     (
//                         text("Start a post about the grid stress test").fontSize(S::pt(13)).color(simd_float4{0.42,0.48,0.54,1.0})
//                     )
//                 ),
//                 div()
//                     .display(gui::Display::Flex)
//                     .justifyContent(gui::JustifyContent::SpaceBetween)
//                 (
//                     div().paddingLeft(S::px(10)).paddingRight(S::px(10)).height(S::px(32)).display(gui::Display::Flex).alignItems(gui::AlignItems::Center)
//                     (
//                         text("Photo").fontSize(S::pt(12)).color(simd_float4{0.1,0.45,0.77,1.0})
//                     ),
//                     div().paddingLeft(S::px(10)).paddingRight(S::px(10)).height(S::px(32)).display(gui::Display::Flex).alignItems(gui::AlignItems::Center)
//                     (
//                         text("Video").fontSize(S::pt(12)).color(simd_float4{0.09,0.58,0.37,1.0})
//                     ),
//                     div().paddingLeft(S::px(10)).paddingRight(S::px(10)).height(S::px(32)).display(gui::Display::Flex).alignItems(gui::AlignItems::Center)
//                     (
//                         text("Write article").fontSize(S::pt(12)).color(simd_float4{0.79,0.47,0.08,1.0})
//                     )
//                 )
//             ),
//             div()
//                 .color(simd_float4{1.0,1.0,1.0,1.0})
//                 .cornerRadius(S::px(10))
//                 .borderColor(simd_float4{0.86,0.88,0.91,1.0})
//                 .borderWidth(S::px(1))
//                 .padding(S::px(16))
//                 .display(gui::Display::Flex)
//                 .flexDirection(gui::FlexDirection::Col)
//                 .flexGap(S::px(14))
//             (
//                 div()
//                     .display(gui::Display::Flex)
//                     .alignItems(gui::AlignItems::Center)
//                     .justifyContent(gui::JustifyContent::SpaceBetween)
//                 (
//                     div()
//                         .display(gui::Display::Flex)
//                         .alignItems(gui::AlignItems::Center)
//                         .flexGap(S::px(12))
//                     (
//                         div(S::px(52), S::px(52), simd_float4{0.18,0.18,0.2,1.0})
//                             .cornerRadius(S::px(26))
//                             .display(gui::Display::Flex)
//                             .alignItems(gui::AlignItems::Center)
//                             .justifyContent(gui::JustifyContent::Center)
//                         (
//                             text("MS").fontSize(S::pt(15)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
//                         ),
//                         div()
//                             .display(gui::Display::Flex)
//                             .flexDirection(gui::FlexDirection::Col)
//                             .flexGap(S::px(3))
//                         (
//                             text("Maya Stone").fontSize(S::pt(14)).font(ArialBold).color(simd_float4{0.12,0.16,0.2,1.0}),
//                             text("Design systems at Northstar").fontSize(S::pt(12)).color(simd_float4{0.39,0.45,0.51,1.0}),
//                             text("2h").fontSize(S::pt(11)).color(simd_float4{0.58,0.62,0.67,1.0})
//                         )
//                     ),
//                     text("...").fontSize(S::pt(16)).color(simd_float4{0.53,0.57,0.62,1.0})
//                 ),
//                 text("Spent the morning rebuilding our composer with a new grid shell and nested flex rows. The ergonomics are getting close.")
//                     .fontSize(S::pt(14))
//                     .color(simd_float4{0.14,0.17,0.21,1.0}),
//                 div()
//                     .height(S::px(210))
//                     .color(simd_float4{0.9,0.94,0.98,1.0})
//                     .cornerRadius(S::px(12))
//                     .padding(S::px(18))
//                     .display(gui::Display::Grid)
//                     .gridTemplateColumns({S::fr(1), S::fr(1), S::fr(1), S::fr(1)})
//                     .gridTemplateRows({S::px(64), S::fr(1), S::px(46)})
//                     .gridColumnGap(S::px(10))
//                     .gridRowGap(S::px(10))
//                 (
//                     div().gridColumn(1, 5).gridRow(1, 2).color(simd_float4{0.1,0.45,0.77,1.0}).cornerRadius(S::px(10))
//                         .display(gui::Display::Flex)
//                         .alignItems(gui::AlignItems::Center)
//                         .paddingLeft(S::px(16))
//                     (
//                         text("Feed card inside a feed card").fontSize(S::pt(14)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
//                     ),
//                     div().gridColumn(1, 2).gridRow(2, 3).color(simd_float4{1.0,1.0,1.0,1.0}).cornerRadius(S::px(10))(),
//                     div().gridColumn(2, 4).gridRow(2, 3).color(simd_float4{1.0,1.0,1.0,1.0}).cornerRadius(S::px(10))(),
//                     div().gridColumn(4, 5).gridRow(2, 3).color(simd_float4{0.84,0.91,0.98,1.0}).cornerRadius(S::px(10))(),
//                     div().gridColumn(1, 5).gridRow(3, 4).color(simd_float4{0.16,0.2,0.24,1.0}).cornerRadius(S::px(10))
//                         .display(gui::Display::Flex)
//                         .alignItems(gui::AlignItems::Center)
//                         .paddingLeft(S::px(16))
//                     (
//                         text("Nested grid preview").fontSize(S::pt(12)).color(simd_float4{0.92,0.94,0.97,1.0})
//                     )
//                 ),
//                 div()
//                     .display(gui::Display::Flex)
//                     .justifyContent(gui::JustifyContent::SpaceBetween)
//                     .alignItems(gui::AlignItems::Center)
//                 (
//                     text("842 reactions   54 comments").fontSize(S::pt(12)).color(simd_float4{0.47,0.52,0.57,1.0}),
//                     div()
//                         .display(gui::Display::Flex)
//                         .alignItems(gui::AlignItems::Center)
//                         .flexGap(S::px(8))
//                     (
//                         div(S::px(104), S::px(36), simd_float4{0.1,0.45,0.77,1.0})
//                             .cornerRadius(S::px(18))
//                             .display(gui::Display::Flex)
//                             .alignItems(gui::AlignItems::Center)
//                             .justifyContent(gui::JustifyContent::Center)
//                             .addEventListener(EventType::MouseDown, onClick)
//                         (
//                             text("Follow").fontSize(S::pt(13)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
//                         ),
//                         div(S::px(104), S::px(36), simd_float4{0.95,0.96,0.98,1.0})
//                             .cornerRadius(S::px(18))
//                             .borderColor(simd_float4{0.84,0.87,0.9,1.0})
//                             .borderWidth(S::px(1))
//                             .display(gui::Display::Flex)
//                             .alignItems(gui::AlignItems::Center)
//                             .justifyContent(gui::JustifyContent::Center)
//                         (
//                             text("Comment").fontSize(S::pt(13)).color(simd_float4{0.27,0.33,0.38,1.0})
//                         )
//                     )
//                 )
//             )
//         ),

//         div().gridColumn(3, 4).gridRow(2, 3)
//             .display(gui::Display::Flex)
//             .flexDirection(gui::FlexDirection::Col)
//             .flexGap(S::px(14))
//         (
//             div()
//                 .color(simd_float4{1.0,1.0,1.0,1.0})
//                 .cornerRadius(S::px(10))
//                 .borderColor(simd_float4{0.86,0.88,0.91,1.0})
//                 .borderWidth(S::px(1))
//                 .padding(S::px(16))
//                 .display(gui::Display::Flex)
//                 .flexDirection(gui::FlexDirection::Col)
//                 .flexGap(S::px(12))
//             (
//                 text("LinkedIn News").fontSize(S::pt(15)).font(ArialBold).color(simd_float4{0.12,0.16,0.2,1.0}),
//                 div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
//                 (
//                     text("Renderer benchmarks are up").fontSize(S::pt(13)).font(ArialBold).color(simd_float4{0.14,0.17,0.21,1.0}),
//                     text("Top story   1,204 readers").fontSize(S::pt(11)).color(simd_float4{0.5,0.55,0.6,1.0})
//                 ),
//                 div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
//                 (
//                     text("More teams are testing native grids").fontSize(S::pt(13)).font(ArialBold).color(simd_float4{0.14,0.17,0.21,1.0}),
//                     text("Trending   884 readers").fontSize(S::pt(11)).color(simd_float4{0.5,0.55,0.6,1.0})
//                 ),
//                 div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
//                 (
//                     text("UI infra hiring cools, specialists still win").fontSize(S::pt(13)).font(ArialBold).color(simd_float4{0.14,0.17,0.21,1.0}),
//                     text("3h ago   642 readers").fontSize(S::pt(11)).color(simd_float4{0.5,0.55,0.6,1.0})
//                 )
//             ),
//             div()
//                 .color(simd_float4{1.0,1.0,1.0,1.0})
//                 .cornerRadius(S::px(10))
//                 .borderColor(simd_float4{0.86,0.88,0.91,1.0})
//                 .borderWidth(S::px(1))
//                 .padding(S::px(16))
//                 .display(gui::Display::Flex)
//                 .flexDirection(gui::FlexDirection::Col)
//                 .flexGap(S::px(14))
//             (
//                 text("Add to your feed").fontSize(S::pt(15)).font(ArialBold).color(simd_float4{0.12,0.16,0.2,1.0}),
//                 div()
//                     .display(gui::Display::Flex)
//                     .alignItems(gui::AlignItems::Center)
//                     .justifyContent(gui::JustifyContent::SpaceBetween)
//                 (
//                     div()
//                         .display(gui::Display::Flex)
//                         .alignItems(gui::AlignItems::Center)
//                         .flexGap(S::px(10))
//                     (
//                         div(S::px(42), S::px(42), simd_float4{0.82,0.51,0.19,1.0}).cornerRadius(S::px(21)).display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::Center)
//                         (
//                             text("AK").fontSize(S::pt(12)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
//                         ),
//                         div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(3))
//                         (
//                             text("Ada Kim").fontSize(S::pt(13)).font(ArialBold).color(simd_float4{0.14,0.17,0.21,1.0}),
//                             text("Graphics engineer").fontSize(S::pt(11)).color(simd_float4{0.5,0.55,0.6,1.0})
//                         )
//                     ),
//                     div(S::px(74), S::px(32), simd_float4{1.0,1.0,1.0,1.0}).cornerRadius(S::px(16)).borderColor(simd_float4{0.47,0.52,0.57,1.0}).borderWidth(S::px(1)).display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::Center)
//                     (
//                         text("+ Follow").fontSize(S::pt(11)).font(ArialBold).color(simd_float4{0.26,0.31,0.36,1.0})
//                     )
//                 ),
//                 div()
//                     .display(gui::Display::Flex)
//                     .alignItems(gui::AlignItems::Center)
//                     .justifyContent(gui::JustifyContent::SpaceBetween)
//                 (
//                     div()
//                         .display(gui::Display::Flex)
//                         .alignItems(gui::AlignItems::Center)
//                         .flexGap(S::px(10))
//                     (
//                         div(S::px(42), S::px(42), simd_float4{0.26,0.56,0.39,1.0}).cornerRadius(S::px(21)).display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::Center)
//                         (
//                             text("LM").fontSize(S::pt(12)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
//                         ),
//                         div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(3))
//                         (
//                             text("Layout Monthly").fontSize(S::pt(13)).font(ArialBold).color(simd_float4{0.14,0.17,0.21,1.0}),
//                             text("Newsletter").fontSize(S::pt(11)).color(simd_float4{0.5,0.55,0.6,1.0})
//                         )
//                     ),
//                     div(S::px(74), S::px(32), simd_float4{1.0,1.0,1.0,1.0}).cornerRadius(S::px(16)).borderColor(simd_float4{0.47,0.52,0.57,1.0}).borderWidth(S::px(1)).display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::Center)
//                     (
//                         text("+ Follow").fontSize(S::pt(11)).font(ArialBold).color(simd_float4{0.26,0.31,0.36,1.0})
//                     )
//                 )
//             )
//         )
//     );


    // using S = gui::Size;
    
    // // Existing dark music player test.
    // // Dark music player — scrollable playlist (left) + nested scrollable lyrics (right)
    // div(S::percent(1.0), S::percent(1.0), simd_float4{0.09,0.09,0.11,1.0})
    //     .display(gui::Display::Flex)
    //     .flexDirection(gui::FlexDirection::Col)
    //     .paddingTop(S::px(28))
    // (
    //     // ── Now Playing bar ──
    //     div(S::percent(1.0), S::px(64), simd_float4{0.13,0.13,0.16,1.0})
    //         .borderColor(simd_float4{0.22,0.22,0.26,1.0})
    //         .borderWidth(S::px(1))
    //         .flexShrink(S::px(0.0))
    //         .display(gui::Display::Flex)
    //         .alignItems(gui::AlignItems::Center)
    //         .justifyContent(gui::JustifyContent::SpaceBetween)
    //         .paddingLeft(S::px(24))
    //         .paddingRight(S::px(24))
    //     (
    //         div()
    //             .display(gui::Display::Flex)
    //             .alignItems(gui::AlignItems::Center)
    //             .flexGap(S::px(12))
    //         (
    //             div(S::px(40), S::px(40), simd_float4{0.18,0.72,0.56,1.0})
    //                 .cornerRadius(S::px(8))
    //                 .display(gui::Display::Flex)
    //                 .alignItems(gui::AlignItems::Center)
    //                 .justifyContent(gui::JustifyContent::Center)
    //             (
    //                 text("♪").fontSize(S::pt(20)).color(simd_float4{1.0,1.0,1.0,1.0})
    //             ),
    //             div()
    //                 .display(gui::Display::Flex)
    //                 .flexDirection(gui::FlexDirection::Col)
    //                 .flexGap(S::px(3))
    //             (
    //                 text("Endless Reverie").fontSize(S::pt(14)).font(ArialBold).color(simd_float4{0.92,0.92,0.94,1.0}),
    //                 text("Glass Prism  ·  Mirrors").fontSize(S::pt(12)).color(simd_float4{0.48,0.48,0.54,1.0})
    //             )
    //         ),
    //         div()
    //             .display(gui::Display::Flex)
    //             .alignItems(gui::AlignItems::Center)
    //             .flexGap(S::px(16))
    //         (
    //             div(S::px(32), S::px(32), simd_float4{0.20,0.20,0.24,1.0})
    //                 .cornerRadius(S::px(16))
    //                 .display(gui::Display::Flex)
    //                 .alignItems(gui::AlignItems::Center)
    //                 .justifyContent(gui::JustifyContent::Center)
    //             (
    //                 text("|<").fontSize(S::pt(11)).color(simd_float4{0.65,0.65,0.70,1.0})
    //             ),
    //             div(S::px(44), S::px(44), simd_float4{0.18,0.72,0.56,1.0})
    //                 .cornerRadius(S::px(22))
    //                 .display(gui::Display::Flex)
    //                 .alignItems(gui::AlignItems::Center)
    //                 .justifyContent(gui::JustifyContent::Center)
    //             (
    //                 text("||").fontSize(S::pt(15)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
    //             ),
    //             div(S::px(32), S::px(32), simd_float4{0.20,0.20,0.24,1.0})
    //                 .cornerRadius(S::px(16))
    //                 .display(gui::Display::Flex)
    //                 .alignItems(gui::AlignItems::Center)
    //                 .justifyContent(gui::JustifyContent::Center)
    //             (
    //                 text(">|").fontSize(S::pt(11)).color(simd_float4{0.65,0.65,0.70,1.0})
    //             )
    //         ),
    //         div()
    //             .display(gui::Display::Flex)
    //             .alignItems(gui::AlignItems::Center)
    //             .flexGap(S::px(10))
    //         (
    //             text("2:14").fontSize(S::pt(12)).color(simd_float4{0.48,0.48,0.54,1.0}),
    //             div(S::px(100), S::px(4), simd_float4{0.24,0.24,0.28,1.0})
    //                 .cornerRadius(S::px(2))
    //             (
    //                 div(S::px(48), S::px(4), simd_float4{0.18,0.72,0.56,1.0})
    //                     .cornerRadius(S::px(2))()
    //             ),
    //             text("4:38").fontSize(S::pt(12)).color(simd_float4{0.48,0.48,0.54,1.0})
    //         )
    //     ),
    //     // ── Body ──
    //     div(S::percent(1.0), S::percent(1.0), simd_float4{0.0,0.0,0.0,0.0})
    //         .display(gui::Display::Flex)
    //         .flexGrow(S::px(1))
    //     (
    //         // sidebar start
    //         // Left: scrollable playlist
    //         div(S::px(260), S::percent(1.0), simd_float4{0.11,0.11,0.14,1.0})
    //             .borderColor(simd_float4{0.20,0.20,0.24,1.0})
    //             .borderWidth(S::px(1))
    //             .flexShrink(S::px(0.0))
    //             .paddingTop(S::px(16))
    //             .paddingBottom(S::px(16))
    //             .overflow(gui::Overflow::Scroll)
    //             .display(gui::Display::Flex)
    //             .flexDirection(gui::FlexDirection::Col)
    //             .flexGap(S::px(1))
    //         (
    //             div()
    //                 .paddingLeft(S::px(16))
    //                 .paddingRight(S::px(16))
    //                 .paddingBottom(S::px(10))
    //                 .display(gui::Display::Flex)
    //                 .alignItems(gui::AlignItems::Center)
    //                 .justifyContent(gui::JustifyContent::SpaceBetween)
    //             (
    //                 text("PLAYLIST").fontSize(S::pt(10)).font(ArialBold).color(simd_float4{0.38,0.38,0.44,1.0}),
    //                 text("12 tracks").fontSize(S::pt(10)).color(simd_float4{0.38,0.38,0.44,1.0})
    //             ),
    //             // 01 – active
    //             div(S::percent(1.0), S::px(52), simd_float4{0.14,0.22,0.20,1.0})
    //                 .paddingLeft(S::px(16)).paddingRight(S::px(16))
    //                 .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
    //             (
    //                 div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
    //                 (
    //                     text("01").fontSize(S::pt(11)).color(simd_float4{0.18,0.72,0.56,1.0}),
    //                     text("Endless Reverie").fontSize(S::pt(13)).font(ArialBold).color(simd_float4{0.18,0.72,0.56,1.0})
    //                 ),
    //                 text("4:38").fontSize(S::pt(12)).color(simd_float4{0.18,0.72,0.56,1.0})
    //             ),
    //             // 02
    //             div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
    //                 .paddingLeft(S::px(16)).paddingRight(S::px(16))
    //                 .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
    //             (
    //                 div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
    //                 (
    //                     text("02").fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0}),
    //                     text("Crystalline").fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})
    //                 ),
    //                 text("3:52").fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})
    //             ),
    //             // 03
    //             div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
    //                 .paddingLeft(S::px(16)).paddingRight(S::px(16))
    //                 .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
    //             (
    //                 div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
    //                 (
    //                     text("03").fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0}),
    //                     text("Pale Shore").fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})
    //                 ),
    //                 text("5:14").fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})
    //             ),
    //             // 04
    //             div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
    //                 .paddingLeft(S::px(16)).paddingRight(S::px(16))
    //                 .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
    //             (
    //                 div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
    //                 (
    //                     text("04").fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0}),
    //                     text("Inversion").fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})
    //                 ),
    //                 text("4:07").fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})
    //             ),
    //             // 05
    //             div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
    //                 .paddingLeft(S::px(16)).paddingRight(S::px(16))
    //                 .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
    //             (
    //                 div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
    //                 (
    //                     text("05").fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0}),
    //                     text("Soft Architecture").fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})
    //                 ),
    //                 text("6:21").fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})
    //             ),
    //             // 06
    //             div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
    //                 .paddingLeft(S::px(16)).paddingRight(S::px(16))
    //                 .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
    //             (
    //                 div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
    //                 (
    //                     text("06").fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0}),
    //                     text("Between Frames").fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})
    //                 ),
    //                 text("3:44").fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})
    //             ),
    //             // 07
    //             div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
    //                 .paddingLeft(S::px(16)).paddingRight(S::px(16))
    //                 .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
    //             (
    //                 div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
    //                 (
    //                     text("07").fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0}),
    //                     text("Nocturne Loop").fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})
    //                 ),
    //                 text("4:58").fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})
    //             ),
    //             // 08
    //             div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
    //                 .paddingLeft(S::px(16)).paddingRight(S::px(16))
    //                 .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
    //             (
    //                 div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
    //                 (
    //                     text("08").fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0}),
    //                     text("Refract").fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})
    //                 ),
    //                 text("3:30").fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})
    //             ),
    //             // 09
    //             div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
    //                 .paddingLeft(S::px(16)).paddingRight(S::px(16))
    //                 .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
    //             (
    //                 div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
    //                 (
    //                     text("09").fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0}),
    //                     text("Diffusion").fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})
    //                 ),
    //                 text("5:02").fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})
    //             ),
    //             // 10
    //             div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
    //                 .paddingLeft(S::px(16)).paddingRight(S::px(16))
    //                 .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
    //             (
    //                 div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
    //                 (
    //                     text("10").fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0}),
    //                     text("Afterimage").fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})
    //                 ),
    //                 text("4:15").fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})
    //             ),
    //             // 11
    //             div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
    //                 .paddingLeft(S::px(16)).paddingRight(S::px(16))
    //                 .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
    //             (
    //                 div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
    //                 (
    //                     text("11").fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0}),
    //                     text("Threshold").fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})
    //                 ),
    //                 text("7:03").fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})
    //             ),
    //             // 12
    //             div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
    //                 .paddingLeft(S::px(16)).paddingRight(S::px(16))
    //                 .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
    //             (
    //                 div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
    //                 (
    //                     text("12").fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0}),
    //                     text("Dissolve").fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})
    //                 ),
    //                 text("4:49").fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})
    //             )
    //         ),
    //         // sidebar end

    //         // Right: album info + scrollable lyrics
    //         div(S::percent(1.0), S::percent(1.0), simd_float4{0.10,0.10,0.12,1.0})
    //             .flexGrow(S::px(1))
    //             .display(gui::Display::Flex)
    //             .flexDirection(gui::FlexDirection::Col)
    //             .padding(S::px(32))
    //             .flexGap(S::px(20))
    //         (
    //             // Album banner
    //             div(S::percent(1.0), S::px(160), simd_float4{0.0,0.0,0.0,0.0})
    //                 .display(gui::Display::Flex)
    //                 .alignItems(gui::AlignItems::Center)
    //                 .flexGap(S::px(24))
    //                 .flexShrink(S::px(0.0))
    //             (
    //                 div(S::px(152), S::px(152), simd_float4{0.18,0.72,0.56,1.0})
    //                     .cornerRadius(S::px(14))
    //                     .display(gui::Display::Flex)
    //                     .alignItems(gui::AlignItems::Center)
    //                     .justifyContent(gui::JustifyContent::Center)
    //                 (
    //                     text("♫").fontSize(S::pt(52)).color(simd_float4{1.0,1.0,1.0,1.0})
    //                 ),
    //                 div()
    //                     .display(gui::Display::Flex)
    //                     .flexDirection(gui::FlexDirection::Col)
    //                     .flexGap(S::px(6))
    //                 (
    //                     text("ALBUM").fontSize(S::pt(10)).font(ArialBold).color(simd_float4{0.38,0.38,0.44,1.0}),
    //                     text("Mirrors")
    //                         .fontSize(S::pt(30))
    //                         .font(ArialBold)
    //                         .color(simd_float4{0.92,0.92,0.94,1.0})
    //                         .addEventListener(runtime::EventType::MouseDown, [](auto& node, Event&) {
    //                             node.text(node.text() == "Mirrors" ? "Mirrors — Extended Edition" : "Mirrors");
    //                         }),
    //                     text("Glass Prism").fontSize(S::pt(16)).color(simd_float4{0.18,0.72,0.56,1.0}),
    //                     text("2024  ·  Ambient  ·  12 tracks").fontSize(S::pt(12)).color(simd_float4{0.48,0.48,0.54,1.0}),
    //                     div()
    //                         .display(gui::Display::Flex)
    //                         .flexGap(S::px(10))
    //                         .paddingTop(S::px(8))
    //                     (
    //                         div(S::px(96), S::px(32), simd_float4{0.18,0.72,0.56,1.0})
    //                             .cornerRadius(S::px(16))
    //                             .display(gui::Display::Flex)
    //                             .alignItems(gui::AlignItems::Center)
    //                             .justifyContent(gui::JustifyContent::Center)
    //                         (
    //                             text("Play all").fontSize(S::pt(13)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})
    //                         ),
    //                         div(S::px(96), S::px(32), simd_float4{0.20,0.20,0.24,1.0})
    //                             .cornerRadius(S::px(16))
    //                             .display(gui::Display::Flex)
    //                             .alignItems(gui::AlignItems::Center)
    //                             .justifyContent(gui::JustifyContent::Center)
    //                         (
    //                             text("Shuffle").fontSize(S::pt(13)).color(simd_float4{0.72,0.72,0.78,1.0})
    //                         )
    //                     )
    //                 )
    //             ),
    //             // Divider
    //             div(S::percent(1.0), S::px(1), simd_float4{0.20,0.20,0.24,1.0})
    //                 .flexShrink(S::px(0.0))(),
    //             // Lyrics label row
    //             div()
    //                 .display(gui::Display::Flex)
    //                 .alignItems(gui::AlignItems::Center)
    //                 .justifyContent(gui::JustifyContent::SpaceBetween)
    //                 .flexShrink(S::px(0.0))
    //             (
    //                 text("LYRICS").fontSize(S::pt(10)).font(ArialBold).color(simd_float4{0.38,0.38,0.44,1.0}),
    //                 text("Endless Reverie").fontSize(S::pt(12)).color(simd_float4{0.48,0.48,0.54,1.0})
    //             ),
    //             // Scrollable lyrics
    //             div(S::percent(1.0), S::percent(1.0), simd_float4{0.13,0.13,0.16,1.0})
    //                 .cornerRadius(S::px(12))
    //                 .padding(S::px(22))
    //                 .overflow(gui::Overflow::Scroll)
    //                 .display(gui::Display::Flex)
    //                 .flexDirection(gui::FlexDirection::Col)
    //                 .flexGap(S::px(7))
    //                 .flexGrow(S::px(1))
    //             (
    //                 text("Through the glass, a world apart,").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
    //                 text("endless echoes fill the dark.").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
    //                 text("Fractures in the silver light —").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
    //                 div(S::percent(1.0), S::px(8), simd_float4{0.0,0.0,0.0,0.0})(),
    //                 text("Hold a breath and feel the weight").fontSize(S::pt(15)).color(simd_float4{0.92,0.92,0.94,1.0}),
    //                 text("of every word you couldn't say,").fontSize(S::pt(15)).color(simd_float4{0.92,0.92,0.94,1.0}),
    //                 text("mirrored back in shades of grey.").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
    //                 div(S::percent(1.0), S::px(8), simd_float4{0.0,0.0,0.0,0.0})(),
    //                 text("Reverie, reverie —").fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0}),
    //                 text("I found you at the edge of sleep.").fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0}),
    //                 text("Reverie, reverie —").fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0}),
    //                 text("a promise too fragile to keep.").fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0}),
    //                 div(S::percent(1.0), S::px(8), simd_float4{0.0,0.0,0.0,0.0})(),
    //                 text("Soft light bends around your face,").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
    //                 text("I chase the outline, lose the trace.").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
    //                 text("The mirror holds what time erased —").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
    //                 div(S::percent(1.0), S::px(8), simd_float4{0.0,0.0,0.0,0.0})(),
    //                 text("Still you linger in the seams,").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
    //                 text("half-remembered, half in dreams.").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
    //                 text("I reach — the surface bends and gleams.").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
    //                 div(S::percent(1.0), S::px(8), simd_float4{0.0,0.0,0.0,0.0})(),
    //                 text("Reverie, reverie —").fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0}),
    //                 text("I found you at the edge of sleep.").fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0}),
    //                 text("Reverie, reverie —").fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0}),
    //                 text("a promise too fragile to keep.").fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0}),
    //                 div(S::percent(1.0), S::px(8), simd_float4{0.0,0.0,0.0,0.0})(),
    //                 text("(Instrumental)").fontSize(S::pt(14)).color(simd_float4{0.35,0.35,0.42,1.0}),
    //                 div(S::percent(1.0), S::px(8), simd_float4{0.0,0.0,0.0,0.0})(),
    //                 text("The glass grows cold, the echo fades,").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
    //                 text("and all that's left is what remains —").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
    //                 text("a shape of light, a broken name.").fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0}),
    //                 div(S::percent(1.0), S::px(8), simd_float4{0.0,0.0,0.0,0.0})(),
    //                 text("Reverie, reverie —").fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0}),
    //                 text("I found you at the edge of sleep.").fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0}),
    //                 text("Reverie, reverie —").fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0}),
    //                 text("a promise too fragile to keep.").fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0})
    //             )
    //         )
    //     )
    // );

    // // Browser reference: react_tests/src/app/intrinsic-sizing/page.tsx
    // using S = gui::Size;
    // constexpr auto intrinsicText = "Intrinsic sizing chooses every soft break opportunity";

    // div(S::percent(1.0), S::percent(1.0), simd_float4{0.059,0.071,0.090,1.0})
    //     .padding(S::px(32))
    //     .overflow(gui::Overflow::Scroll)
    // (
    //     div(S::minContent(), S::autoSize(), simd_float4{0.078,0.722,0.859,1.0})
    //     (
    //         text(intrinsicText)
    //             .font(Arial)
    //             .fontSize(S::pt(18))
    //             .color(simd_float4{0.961,0.969,1.0,1.0})
    //     ),

    //     div(S::maxContent(), S::autoSize(), simd_float4{0.961,0.302,0.459,1.0})
    //         .marginTop(S::px(20))
    //     (
    //         text(intrinsicText)
    //             .font(Arial)
    //             .fontSize(S::pt(18))
    //             .color(simd_float4{0.961,0.969,1.0,1.0})
    //     ),

    //     div(S::px(360), S::autoSize(), simd_float4{0.149,0.161,0.200,1.0})
    //         .marginTop(S::px(20))
    //     (
    //         div(S::fitContent(), S::autoSize(), simd_float4{0.980,0.761,0.200,1.0})
    //         (
    //             text(intrinsicText)
    //                 .font(Arial)
    //                 .fontSize(S::pt(18))
    //                 .color(simd_float4{0.078,0.090,0.110,1.0})
    //         )
    //     ),

    //     div(S::px(120), S::autoSize(), simd_float4{0.380,0.922,0.561,1.0})
    //         .minWidth(S::maxContent())
    //         .marginTop(S::px(20))
    //     (
    //         text(intrinsicText)
    //             .font(Arial)
    //             .fontSize(S::pt(18))
    //             .color(simd_float4{0.078,0.090,0.110,1.0})
    //     ),

    //     div(S::px(640), S::autoSize(), simd_float4{0.639,0.420,0.961,1.0})
    //         .maxWidth(S::minContent())
    //         .marginTop(S::px(20))
    //     (
    //         text(intrinsicText)
    //             .font(Arial)
    //             .fontSize(S::pt(18))
    //             .color(simd_float4{0.961,0.969,1.0,1.0})
    //     )
    // );


    // // Auto-sized parent with a percentage-width child.
    // //
    // // The child's first percentage basis is the parent's unresolved auto width.
    // // The 600px grandchild supplies the intrinsic width. Once the auto-sized parent
    // // resolves to 600px, the child must resolve 50% to 300px. The yellow grandchild
    // // intentionally overflows the pink child so the 300px boundary remains visible.
    // div(S::percent(1.0), S::percent(1.0), simd_float4{0.06,0.07,0.09,1.0})
    // (
    //     div(S::autoSize(), S::autoSize(), simd_float4{0.08,0.72,0.86,1.0})
    //         .position(gui::Position::Absolute)
    //         .left(S::px(80))
    //         .top(S::px(80))
    //     (
    //         div(S::percent(0.5), S::px(120), simd_float4{0.96,0.30,0.46,1.0})
    //         (
    //             div(S::px(600), S::px(80), simd_float4{0.98,0.76,0.20,1.0})()
    //         )
    //     )
    // );


//     // Flex Phase A shrink-to-fit stress test.
//     // Compare the resolved geometry with the equivalent browser reference page.
//     div(S::percent(1.0), S::percent(1.0), simd_float4{0.06,0.07,0.09,1.0})
//         .display(gui::Display::Flex)
//         .flexDirection(gui::FlexDirection::Col)
//         .alignItems(gui::AlignItems::FlexStart)
//         .padding(S::px(48))
//         .flexGap(S::px(28))
//     (
//         text("PHASE A: INDEFINITE PERCENT VS AUTO")
//             .font(Arial)
//             .fontSize(S::pt(24))
//             .color(simd_float4{0.96,0.97,1.0,1.0}),
//         text("Compare each cyan percentage target and auto control with the browser reference")
//             .font(Arial)
//             .fontSize(S::pt(14))
//             .color(simd_float4{0.70,0.73,0.80,1.0}),

//         // Width case: row flex parent has no definite width.
//         text("INDEFINITE WIDTH — 60% TARGET VS AUTO CONTROL")
//             .font(Arial)
//             .fontSize(S::pt(13))
//             .color(simd_float4{0.98,0.76,0.24,1.0}),
//         div().display(gui::Display::Flex).alignItems(gui::AlignItems::FlexStart).flexGap(S::px(48))
//         (
//             div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).alignItems(gui::AlignItems::FlexStart).flexGap(S::px(8))
//             (
//                 text("AUTO CONTROL").font(Arial).fontSize(S::pt(12)).color(simd_float4{0.38,0.92,0.56,1.0}),
//                 div(S::autoSize(), S::px(112), simd_float4{0.15,0.16,0.20,1.0})
//                     .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).padding(S::px(14))
//                 (
//                     div(S::autoSize(), S::px(76), simd_float4{0.08,0.72,0.86,1.0})
//                         .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).padding(S::px(10)).flexGap(S::px(8))
//                     (
//                         div(S::px(180), S::px(56), simd_float4{0.98,0.76,0.20,1.0})(),
//                         div(S::px(120), S::px(56), simd_float4{0.96,0.30,0.46,1.0})()
//                     )
//                 )
//             ),
//             div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).alignItems(gui::AlignItems::FlexStart).flexGap(S::px(8))
//             (
//                 text("60% TARGET").font(Arial).fontSize(S::pt(12)).color(simd_float4{1.0,0.54,0.22,1.0}),
//                 div(S::autoSize(), S::px(112), simd_float4{0.15,0.16,0.20,1.0})
//                     .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).padding(S::px(14))
//                 (
//                     div(S::percent(0.60), S::px(76), simd_float4{0.08,0.72,0.86,1.0})
//                         .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).padding(S::px(10)).flexGap(S::px(8))
//                     (
//                         div(S::px(180), S::px(56), simd_float4{0.98,0.76,0.20,1.0})(),
//                         div(S::px(120), S::px(56), simd_float4{0.96,0.30,0.46,1.0})()
//                     )
//                 )
//             )
//         ),

//         // Height case: column flex parent has no definite height.
//         text("INDEFINITE HEIGHT — 50% TARGET VS AUTO CONTROL")
//             .font(Arial)
//             .fontSize(S::pt(13))
//             .color(simd_float4{0.98,0.76,0.24,1.0}),
//         div().display(gui::Display::Flex).alignItems(gui::AlignItems::FlexStart).flexGap(S::px(48))
//         (
//             div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(8))
//             (
//                 text("AUTO CONTROL").font(Arial).fontSize(S::pt(12)).color(simd_float4{0.38,0.92,0.56,1.0}),
//                 div(S::px(360), S::autoSize(), simd_float4{0.15,0.16,0.20,1.0})
//                     .display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).alignItems(gui::AlignItems::Stretch).padding(S::px(14))
//                 (
//                     div(S::percent(1.0), S::autoSize(), simd_float4{0.08,0.72,0.86,1.0})
//                         .display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).padding(S::px(10)).flexGap(S::px(8))
//                     (
//                         div(S::percent(1.0), S::px(72), simd_float4{0.98,0.76,0.20,1.0})(),
//                         div(S::percent(1.0), S::px(48), simd_float4{0.96,0.30,0.46,1.0})()
//                     )
//                 )
//             ),
//             div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(8))
//             (
//                 text("50% TARGET").font(Arial).fontSize(S::pt(12)).color(simd_float4{1.0,0.54,0.22,1.0}),
//                 div(S::px(360), S::autoSize(), simd_float4{0.15,0.16,0.20,1.0})
//                     .display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).alignItems(gui::AlignItems::Stretch).padding(S::px(14))
//                 (
//                     div(S::percent(1.0), S::percent(0.50), simd_float4{0.08,0.72,0.86,1.0})
//                         .display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).padding(S::px(10)).flexGap(S::px(8))
//                     (
//                         div(S::percent(1.0), S::px(72), simd_float4{0.98,0.76,0.20,1.0})(),
//                         div(S::percent(1.0), S::px(48), simd_float4{0.96,0.30,0.46,1.0})()
//                     )
//                 )
//             )
//         )
//     );
    
    // using S = gui::Size;
    // const std::string Devanagari = "/System/Library/Fonts/Supplemental/Devanagari Sangam MN.ttc";
    // const std::string GeezaPro = "/System/Library/Fonts/GeezaPro.ttc";
    // const auto background = simd_float4{0.07f, 0.08f, 0.10f, 1.0f};
    // const auto panel = simd_float4{0.12f, 0.13f, 0.16f, 1.0f};
    // const auto heading = simd_float4{0.45f, 0.80f, 1.0f, 1.0f};
    // const auto body = simd_float4{0.92f, 0.93f, 0.96f, 1.0f};
    // const auto note = simd_float4{0.58f, 0.62f, 0.70f, 1.0f};

    // div(S::percent(1.0), S::percent(1.0), background)
    //     .display(gui::Display::Flex)
    //     .flexDirection(gui::FlexDirection::Col)
    //     .flexGap(S::px(16))
    //     .padding(S::px(28))
    //     .overflow(gui::Overflow::Scroll)
    // (
    //     text("HarfBuzz shaping coverage").font(ArialBold).fontSize(S::pt(24)).color(body),
    //     text("Kerning, ligatures, combining marks, and contextual script shaping")
    //         .fontSize(S::pt(13)).color(note),

    //     div().width(S::percent(1.0)).color(panel).padding(S::px(18))
    //         .display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(8))
    //     (
    //         text("4. Kerning").font(ArialBold).fontSize(S::pt(13)).color(heading),
    //         text("AVATAR   To Wa Yo VA").font(Arial).fontSize(S::pt(34)).color(body),
    //         text("Look for tighter AV, To, Wa, and Yo pairs.").fontSize(S::pt(12)).color(note)
    //     ),

    //     div().width(S::percent(1.0)).color(panel).padding(S::px(18))
    //         .display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(8))
    //     (
    //         text("5. Ligatures").font(ArialBold).fontSize(S::pt(13)).color(heading),
    //         text("office   affinity   ffi   fi   fl").font(Helvetica).fontSize(S::pt(34)).color(body),
    //         text("Helvetica should substitute fi/fl glyphs; ‘office’ should contain an fi ligature.")
    //             .fontSize(S::pt(12)).color(note)
    //     ),

    //     div().width(S::percent(1.0)).color(panel).padding(S::px(18))
    //         .display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(8))
    //     (
    //         text("6a. Combining marks").font(ArialBold).fontSize(S::pt(13)).color(heading),
    //         text("decomposed:  cafe\xCC\x81   A\xCC\x8A   n\xCC\x83").font(Arial).fontSize(S::pt(34)).color(body),
    //         text("composed:    café   Å   ñ").font(Arial).fontSize(S::pt(34)).color(body),
    //         text("The two rows should have equivalent accent placement.").fontSize(S::pt(12)).color(note)
    //     ),

    //     div().width(S::percent(1.0)).color(panel).padding(S::px(18))
    //         .display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(8))
    //     (
    //         text("6b. Arabic contextual shaping").font(ArialBold).fontSize(S::pt(13)).color(heading),
    //         text("السلام عليكم").font(GeezaPro).fontSize(S::pt(40)).color(body),
    //         text("Letters should join contextually; lam-alef should form a ligature.").fontSize(S::pt(12)).color(note)
    //     ),

    //     div().width(S::percent(1.0)).color(panel).padding(S::px(18))
    //         .display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(8))
    //     (
    //         text("6c. Devanagari reordering and conjuncts").font(ArialBold).fontSize(S::pt(13)).color(heading),
    //         text("नमस्ते दुनिया").font(Devanagari).fontSize(S::pt(40)).color(body),
    //         text("The conjunct and pre-base vowel marks should be shaped and positioned.")
    //             .fontSize(S::pt(12)).color(note)
    //     ),

    //     div().width(S::percent(1.0)).color(panel).padding(S::px(18))
    //         .display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(8))
    //     (
    //         text("7a. Mixed LTR and RTL runs").font(ArialBold).fontSize(S::pt(13)).color(heading),
    //         text("Hello السلام").font(Arial).fontSize(S::pt(30)).color(body),
    //         text("السلام Hello").font(Arial).fontSize(S::pt(30)).color(body),
    //         text("Each Arabic run should remain joined while the two scripts retain their reading order.")
    //             .fontSize(S::pt(12)).color(note)
    //     ),

    //     div().width(S::percent(1.0)).color(panel).padding(S::px(18))
    //         .display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(8))
    //     (
    //         text("7b. Numbers and neutral punctuation").font(ArialBold).fontSize(S::pt(13)).color(heading),
    //         text("السلام 123 عليكم").font(Arial).fontSize(S::pt(30)).color(body),
    //         text("Hello (السلام) 123").font(Arial).fontSize(S::pt(30)).color(body),
    //         text("The digits should read 123 and the parentheses should enclose the Arabic word.")
    //             .fontSize(S::pt(12)).color(note)
    //     ),

    //     div().width(S::percent(1.0)).color(panel).padding(S::px(18))
    //         .display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(8))
    //     (
    //         text("7c. Bidi resolution across text siblings").font(ArialBold).fontSize(S::pt(13)).color(heading),
    //         div()
    //         (
    //             text("Hello ").font(Arial).fontSize(S::pt(30)).color(body),
    //             text("السلام").font(GeezaPro).fontSize(S::pt(30)).color(body),
    //             text(" 123").font(Arial).fontSize(S::pt(30)).color(body)
    //         ),
    //         text("This is three sibling Text nodes and should match a single mixed-direction sentence.")
    //             .fontSize(S::pt(12)).color(note)
    //     ),

    //     div().width(S::percent(1.0)).color(panel).padding(S::px(18))
    //         .display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(8))
    //     (
    //         text("7d. Mixed-direction wrapping").font(ArialBold).fontSize(S::pt(13)).color(heading),
    //         div().width(S::px(260))
    //         (
    //             text("Hello السلام 123 عليكم from a narrow mixed-direction line")
    //                 .font(Arial).fontSize(S::pt(24)).color(body)
    //         ),
    //         text("Every physical line should remain separate, joined, and non-overlapping.")
    //             .fontSize(S::pt(12)).color(note)
    //     )
    // );

//     const auto alignmentSample = R"(Short line
// A considerably longer line that wraps inside the panel)";
//     const auto panelColor = simd_float4{0.94f, 0.95f, 0.97f, 1.0f};
//     const auto textColor = simd_float4{0.08f, 0.09f, 0.11f, 1.0f};
//     const auto labelColor = simd_float4{0.32f, 0.35f, 0.40f, 1.0f};

//     div(S::percent(1.0), S::percent(1.0), simd_float4{1.0f, 1.0f, 1.0f, 1.0f})
//         .display(gui::Display::Flex)
//         .flexDirection(gui::FlexDirection::Col)
//         .flexGap(S::px(12))
//         .padding(S::px(24))
//         .overflow(gui::Overflow::Scroll)
//     (
//         div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
//         (
//             text("Start").fontSize(S::pt(12)).color(labelColor),
//             div().width(S::px(280)).minHeight(S::px(90)).color(panelColor)
//                 .textAlign(gui::TextAlign::Start)
//             (
//                 text(alignmentSample).fontSize(S::pt(16)).lineHeight(1.25f).color(textColor)
//                     .whiteSpace(gui::WhiteSpace::PreWrap)
//             )
//         ),
//         div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
//         (
//             text("Left").fontSize(S::pt(12)).color(labelColor),
//             div().width(S::px(280)).minHeight(S::px(90)).color(panelColor)
//                 .textAlign(gui::TextAlign::Left)
//             (
//                 text(alignmentSample).fontSize(S::pt(16)).lineHeight(1.25f).color(textColor)
//                     .whiteSpace(gui::WhiteSpace::PreWrap)
//             )
//         ),
//         div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
//         (
//             text("Center").fontSize(S::pt(12)).color(labelColor),
//             div().width(S::px(280)).minHeight(S::px(90)).color(panelColor)
//                 .textAlign(gui::TextAlign::Center)
//             (
//                 text(alignmentSample).fontSize(S::pt(16)).lineHeight(1.25f).color(textColor)
//                     .whiteSpace(gui::WhiteSpace::PreWrap)
//             )
//         ),
//         div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
//         (
//             text("Right").fontSize(S::pt(12)).color(labelColor),
//             div().width(S::px(280)).minHeight(S::px(90)).color(panelColor)
//                 .textAlign(gui::TextAlign::Right)
//             (
//                 text(alignmentSample).fontSize(S::pt(16)).lineHeight(1.25f).color(textColor)
//                     .whiteSpace(gui::WhiteSpace::PreWrap)
//             )
//         )
//     );



//     const auto sample = R"(Alpha   beta gamma delta epsilon zeta
// Second   line with Supercalifragilisticexpialidocious tail)";
//     const auto breakAllSample = "BreakAll: Supercalifragilisticexpialidocious0123456789";
//     const auto panelColor = simd_float4{0.94f, 0.95f, 0.97f, 1.0f};
//     const auto textColor = simd_float4{0.08f, 0.09f, 0.11f, 1.0f};
//     const auto labelColor = simd_float4{0.32f, 0.35f, 0.40f, 1.0f};

//     div(S::percent(1.0), S::percent(1.0), simd_float4{1.0f, 1.0f, 1.0f, 1.0f})
//         .display(gui::Display::Flex)
//         .flexDirection(gui::FlexDirection::Col)
//         .flexGap(S::px(12))
//         .padding(S::px(24))
//         .overflow(gui::Overflow::Scroll)
//     (
//         div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
//         (
//             text("Normal").fontSize(S::pt(12)).color(labelColor),
//             div().width(S::px(280)).minHeight(S::px(90)).color(panelColor)
//             (
//                 text(sample).fontSize(S::pt(16)).lineHeight(1.25f).color(textColor)
//                     .whiteSpace(gui::WhiteSpace::Normal)
//             )
//         ),
//         div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
//         (
//             text("NoWrap").fontSize(S::pt(12)).color(labelColor),
//             div().width(S::px(280)).minHeight(S::px(90)).color(panelColor)
//             (
//                 text(sample).fontSize(S::pt(16)).lineHeight(1.25f).color(textColor)
//                     .whiteSpace(gui::WhiteSpace::NoWrap)
//             )
//         ),
//         div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
//         (
//             text("Pre").fontSize(S::pt(12)).color(labelColor),
//             div().width(S::px(280)).minHeight(S::px(90)).color(panelColor)
//             (
//                 text(sample).fontSize(S::pt(16)).lineHeight(1.25f).color(textColor)
//                     .whiteSpace(gui::WhiteSpace::Pre)
//             )
//         ),
//         div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
//         (
//             text("PreWrap").fontSize(S::pt(12)).color(labelColor),
//             div().width(S::px(280)).minHeight(S::px(90)).color(panelColor)
//             (
//                 text(sample).fontSize(S::pt(16)).lineHeight(1.25f).color(textColor)
//                     .whiteSpace(gui::WhiteSpace::PreWrap)
//             )
//         ),
//         div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
//         (
//             text("Normal + BreakAll").fontSize(S::pt(12)).color(labelColor),
//             div().width(S::px(280)).minHeight(S::px(90)).color(panelColor)
//             (
//                 text(breakAllSample).fontSize(S::pt(16)).lineHeight(1.25f).color(textColor)
//                     .whiteSpace(gui::WhiteSpace::Normal)
//                     .wordBreak(gui::WordBreak::BreakAll)
//             )
//         )
//     );


    // const auto overflowSample = "The quick brown fox jumps over the lazy dog";
    // const auto panelColor = simd_float4{0.94f, 0.95f, 0.97f, 1.0f};
    // const auto textColor = simd_float4{0.08f, 0.09f, 0.11f, 1.0f};
    // const auto labelColor = simd_float4{0.32f, 0.35f, 0.40f, 1.0f};

    // div(S::percent(1.0), S::percent(1.0), simd_float4{1.0f, 1.0f, 1.0f, 1.0f})
    //     .display(gui::Display::Flex)
    //     .flexDirection(gui::FlexDirection::Col)
    //     .flexGap(S::px(12))
    //     .padding(S::px(24))
    //     .overflow(gui::Overflow::Scroll)
    // (
    //     div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
    //     (
    //         text("Clip").fontSize(S::pt(12)).color(labelColor),
    //         div().width(S::px(280)).height(S::px(32)).color(panelColor)
    //             .overflow(gui::Overflow::Hidden)
    //             .textOverflow(gui::TextOverflow::clip())
    //         (
    //             text(overflowSample).fontSize(S::pt(16)).lineHeight(1.25f).color(textColor)
    //                 .whiteSpace(gui::WhiteSpace::NoWrap)
    //         )
    //     ),
    //     div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
    //     (
    //         text("Ellipsis, text fits").fontSize(S::pt(12)).color(labelColor),
    //         div().width(S::px(280)).height(S::px(32)).color(panelColor)
    //             .overflow(gui::Overflow::Hidden)
    //             .textOverflow(gui::TextOverflow::ellipsis())
    //         (
    //             text("Short text").fontSize(S::pt(16)).lineHeight(1.25f).color(textColor)
    //                 .whiteSpace(gui::WhiteSpace::NoWrap)
    //         )
    //     ),
    //     div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
    //     (
    //         text("Ellipsis").fontSize(S::pt(12)).color(labelColor),
    //         div().width(S::px(280)).height(S::px(32)).color(panelColor)
    //             .overflow(gui::Overflow::Hidden)
    //             .textOverflow(gui::TextOverflow::ellipsis())
    //         (
    //             text(overflowSample).fontSize(S::pt(16)).lineHeight(1.25f).color(textColor)
    //                 .whiteSpace(gui::WhiteSpace::NoWrap)
    //         )
    //     ),
    //     div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
    //     (
    //         text("Ellipsis, narrower than marker").fontSize(S::pt(12)).color(labelColor),
    //         div().width(S::px(6)).height(S::px(32)).color(panelColor)
    //             .overflow(gui::Overflow::Hidden)
    //             .textOverflow(gui::TextOverflow::ellipsis())
    //         (
    //             text("Wide text").fontSize(S::pt(16)).lineHeight(1.25f).color(textColor)
    //                 .whiteSpace(gui::WhiteSpace::NoWrap)
    //         )
    //     ),
    //     div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
    //     (
    //         text("Custom ending").fontSize(S::pt(12)).color(labelColor),
    //         div().width(S::px(280)).height(S::px(32)).color(panelColor)
    //             .overflow(gui::Overflow::Hidden)
    //             .textOverflow(gui::TextOverflow::custom(" [more]"))
    //         (
    //             text(overflowSample).fontSize(S::pt(16)).lineHeight(1.25f).color(textColor)
    //                 .whiteSpace(gui::WhiteSpace::NoWrap)
    //         )
    //     ),
    //     div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
    //     (
    //         text("Ellipsis + Scroll").fontSize(S::pt(12)).color(labelColor),
    //         div().width(S::px(140)).height(S::px(32)).color(panelColor)
    //             .overflow(gui::Overflow::Scroll)
    //             .textOverflow(gui::TextOverflow::ellipsis())
    //         (
    //             text(overflowSample).fontSize(S::pt(16)).lineHeight(1.25f).color(textColor)
    //                 .whiteSpace(gui::WhiteSpace::NoWrap)
    //         )
    //     ),
    //     div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(4))
    //     (
    //         text("Nested clipping ancestor").fontSize(S::pt(12)).color(labelColor),
    //         div().width(S::px(280)).height(S::px(32)).color(panelColor)
    //             .overflow(gui::Overflow::Hidden)
    //             .textOverflow(gui::TextOverflow::ellipsis())
    //         (
    //             div()
    //             (
    //                 text(overflowSample).fontSize(S::pt(16)).lineHeight(1.25f).color(textColor)
    //                     .whiteSpace(gui::WhiteSpace::NoWrap)
    //             )
    //         )
    //     )
    // );


// 1st general test on min/max size
// using S = gui::Size;

// div()
//     .width(S::percent(1.0))
//     .height(S::percent(1.0))
//     .color(simd_float4{0.09,0.09,0.11,1.0})
//     .display(gui::Display::Flex)
//     .flexDirection(gui::FlexDirection::Col)
//     .flexGap(S::px(16))
//     .padding(S::px(24))
// (
//     div()
//         .width(S::percent(1.0))
//         .color(simd_float4{0.13,0.13,0.16,1.0})
//         .display(gui::Display::Flex)
//         .flexDirection(gui::FlexDirection::Row)
//         .flexWrap(gui::FlexWrap::Wrap)
//         .flexGap(S::px(8))
//         .minHeight(S::px(60))
//         .maxHeight(S::px(120))
//         .padding(S::px(12))
//     (
//         div()
//             .height(S::px(36))
//             .color(simd_float4{0.18,0.72,0.56,1.0})
//             .flexGrow(S::px(1))
//             .minWidth(S::px(80))
//             .maxWidth(S::px(200))
//             .cornerRadius(S::px(6))
//         (),
//         div()
//             .height(S::px(36))
//             .color(simd_float4{0.30,0.30,0.36,1.0})
//             .flexGrow(S::px(2))
//             .minWidth(S::px(120))
//             .maxWidth(S::px(400))
//             .cornerRadius(S::px(6))
//         (),
//         div()
//             .width(S::px(60))
//             .height(S::px(36))
//             .color(simd_float4{0.22,0.22,0.28,1.0})
//             .flexShrink(S::px(0.0))
//             .cornerRadius(S::px(6))
//         ()
//     ),

//     div()
//         .width(S::percent(1.0))
//         .color(simd_float4{0.11,0.11,0.14,1.0})
//         .display(gui::Display::Grid)
//         .gridTemplateColumns({S::percent(0.33), S::percent(0.33), S::percent(0.33)})
//         .gridTemplateRows({S::px(80), S::px(80)})
//         .gridColumnGap(S::px(8))
//         .gridRowGap(S::px(8))
//         .minHeight(S::px(80))
//         .maxHeight(S::px(300))
//         .padding(S::px(12))
//     (
//         div()
//             .color(simd_float4{0.18,0.72,0.56,1.0})
//             .minHeight(S::px(60))
//             .maxHeight(S::px(140))
//             .cornerRadius(S::px(6))
//         (),
//         div()
//             .color(simd_float4{0.30,0.30,0.36,1.0})
//             .gridColumn(2, 4)
//             .minWidth(S::px(160))
//             .maxWidth(S::px(500))
//             .minHeight(S::px(60))
//             .cornerRadius(S::px(6))
//         (),
//         div()
//             .color(simd_float4{0.22,0.22,0.28,1.0})
//             .gridColumn(1, 4)
//             .minHeight(S::px(40))
//             .maxHeight(S::px(80))
//             .cornerRadius(S::px(6))
//         ()
//     ),

//     div()
//         .width(S::percent(1.0))
//         .color(simd_float4{0.13,0.13,0.16,1.0})
//         .minWidth(S::px(200))
//         .maxWidth(S::px(600))
//         .minHeight(S::px(80))
//         .maxHeight(S::px(160))
//         .padding(S::px(16))
//         .cornerRadius(S::px(8))
//     (
//         div()
//             .width(S::percent(0.6))
//             .height(S::px(20))
//             .color(simd_float4{0.18,0.72,0.56,1.0})
//             .minWidth(S::px(100))
//             .maxWidth(S::px(320))
//             .cornerRadius(S::px(4))
//         (),
//         div()
//             .width(S::percent(0.9))
//             .height(S::px(20))
//             .color(simd_float4{0.30,0.30,0.36,1.0})
//             .minWidth(S::px(140))
//             .maxWidth(S::px(480))
//             .marginTop(S::px(8))
//             .cornerRadius(S::px(4))
//         (),
//         div()
//             .width(S::percent(0.4))
//             .height(S::px(20))
//             .color(simd_float4{0.22,0.22,0.28,1.0})
//             .minWidth(S::px(80))
//             .maxWidth(S::px(200))
//             .marginTop(S::px(8))
//             .cornerRadius(S::px(4))
//         ()
//     )
// );

// using S = gui::Size;

/*
div()
    .width(S::percent(1.0))
    .height(S::percent(1.0))
    .color(simd_float4{0.09,0.09,0.11,1.0})
    .display(gui::Display::Grid)
    .gridTemplateColumns({S::percent(0.5), S::percent(0.5)})
    .gridTemplateRows({S::px(200), S::px(200)})
    .gridColumnGap(S::px(12))
    .gridRowGap(S::px(12))
    .padding(S::px(24))
(
    div()
        .color(simd_float4{0.13,0.13,0.16,1.0})
        .display(gui::Display::Flex)
        .flexDirection(gui::FlexDirection::Row)
        .flexGap(S::px(8))
        .padding(S::px(12))
        .minWidth(S::px(120))
        .maxWidth(S::px(600))
    (
        div()
            .height(S::percent(1.0))
            .color(simd_float4{0.18,0.72,0.56,1.0})
            .flexGrow(S::px(1))
            .minWidth(S::px(60))
            .maxWidth(S::px(180))
            .cornerRadius(S::px(6))
        (),
        div()
            .height(S::percent(1.0))
            .color(simd_float4{0.22,0.22,0.28,1.0})
            .flexGrow(S::px(3))
            .minWidth(S::px(100))
            .cornerRadius(S::px(6))
            .display(gui::Display::Flex)
            .flexDirection(gui::FlexDirection::Col)
            .flexGap(S::px(6))
            .padding(S::px(8))
        (
            div()
                .width(S::percent(1.0))
                .color(simd_float4{0.18,0.72,0.56,1.0})
                .flexGrow(S::px(1))
                .minHeight(S::px(24))
                .maxHeight(S::px(60))
                .cornerRadius(S::px(4))
            (),
            div()
                .width(S::percent(1.0))
                .color(simd_float4{0.30,0.30,0.36,1.0})
                .flexGrow(S::px(2))
                .minHeight(S::px(40))
                .cornerRadius(S::px(4))
            (),
            div()
                .width(S::percent(1.0))
                .color(simd_float4{0.18,0.72,0.56,1.0})
                .flexGrow(S::px(1))
                .minHeight(S::px(24))
                .maxHeight(S::px(60))
                .cornerRadius(S::px(4))
            ()
        )
    ),

    div()
        .color(simd_float4{0.13,0.13,0.16,1.0})
        .display(gui::Display::Flex)
        .flexDirection(gui::FlexDirection::Col)
        .flexGap(S::px(8))
        .padding(S::px(12))
    (
        div()
            .width(S::percent(1.0))
            .color(simd_float4{0.22,0.22,0.28,1.0})
            .flexGrow(S::px(1))
            .minHeight(S::px(40))
            .maxHeight(S::px(80))
            .cornerRadius(S::px(6))
        (),
        div()
            .width(S::percent(1.0))
            .color(simd_float4{0.30,0.30,0.36,1.0})
            .flexShrink(S::px(0.0))
            .height(S::px(60))
            .minWidth(S::px(80))
            .cornerRadius(S::px(6))
        (),
        div()
            .width(S::percent(1.0))
            .color(simd_float4{0.22,0.22,0.28,1.0})
            .flexGrow(S::px(2))
            .minHeight(S::px(40))
            .cornerRadius(S::px(6))
        ()
    ),

    div()
        .color(simd_float4{0.11,0.11,0.14,1.0})
        .gridColumn(1, 3)
        .display(gui::Display::Flex)
        .flexDirection(gui::FlexDirection::Row)
        .flexWrap(gui::FlexWrap::Wrap)
        .flexGap(S::px(8))
        .padding(S::px(12))
        .minHeight(S::px(80))
        .maxHeight(S::px(200))
    (
        div()
            .height(S::px(48))
            .color(simd_float4{0.18,0.72,0.56,1.0})
            .flexGrow(S::px(1))
            .minWidth(S::px(80))
            .maxWidth(S::px(160))
            .cornerRadius(S::px(6))
        (),
        div()
            .height(S::px(48))
            .color(simd_float4{0.22,0.22,0.28,1.0})
            .flexGrow(S::px(1))
            .minWidth(S::px(80))
            .maxWidth(S::px(160))
            .cornerRadius(S::px(6))
        (),
        div()
            .height(S::px(48))
            .color(simd_float4{0.30,0.30,0.36,1.0})
            .flexGrow(S::px(1))
            .minWidth(S::px(80))
            .maxWidth(S::px(160))
            .cornerRadius(S::px(6))
        (),
        div()
            .height(S::px(48))
            .color(simd_float4{0.18,0.72,0.56,1.0})
            .flexGrow(S::px(1))
            .minWidth(S::px(80))
            .maxWidth(S::px(160))
            .cornerRadius(S::px(6))
        (),
        div()
            .height(S::px(48))
            .color(simd_float4{0.22,0.22,0.28,1.0})
            .flexGrow(S::px(1))
            .minWidth(S::px(80))
            .maxWidth(S::px(160))
            .cornerRadius(S::px(6))
        ()
    )
);
*/

/*
div()
    .width(S::percent(1.0))
    .height(S::percent(1.0))
    .color(simd_float4{0.04,0.04,0.05,1.0})
    .display(gui::Display::Grid)
    .gridTemplateColumns({S::percent(0.5), S::percent(0.5)})
    .gridTemplateRows({S::percent(0.5), S::percent(0.5)})
    .gridColumnGap(S::px(14))
    .gridRowGap(S::px(14))
    .padding(S::px(24))
(
    div()
        .color(simd_float4{0.12,0.12,0.15,1.0})
        .display(gui::Display::Flex)
        .flexDirection(gui::FlexDirection::Row)
        .flexGap(S::px(10))
        .padding(S::px(12))
        .minWidth(S::px(240))
        .maxWidth(S::px(720))
    (
        div()
            .height(S::percent(1.0))
            .color(simd_float4{0.95,0.18,0.22,1.0})
            .flexGrow(S::px(2))
            .flexShrink(S::px(1))
            .minWidth(S::px(90))
            .maxWidth(S::px(180))
            .cornerRadius(S::px(5))
        (),
        div()
            .height(S::percent(1.0))
            .color(simd_float4{0.10,0.72,0.95,1.0})
            .flexGrow(S::px(1))
            .flexShrink(S::px(4))
            .minWidth(S::px(160))
            .maxWidth(S::px(260))
            .cornerRadius(S::px(5))
        (),
        div()
            .height(S::percent(1.0))
            .color(simd_float4{0.95,0.84,0.16,1.0})
            .flexGrow(S::px(3))
            .flexShrink(S::px(1))
            .minWidth(S::px(70))
            .maxWidth(S::px(140))
            .cornerRadius(S::px(5))
        ()
    ),

    div()
        .color(simd_float4{0.12,0.12,0.15,1.0})
        .display(gui::Display::Flex)
        .flexDirection(gui::FlexDirection::Col)
        .flexGap(S::px(10))
        .padding(S::px(12))
        .minHeight(S::px(180))
        .maxHeight(S::px(360))
    (
        div()
            .width(S::percent(1.0))
            .color(simd_float4{0.32,0.95,0.42,1.0})
            .flexGrow(S::px(1))
            .flexShrink(S::px(1))
            .minHeight(S::px(42))
            .maxHeight(S::px(80))
            .cornerRadius(S::px(5))
        (),
        div()
            .width(S::percent(1.0))
            .color(simd_float4{0.68,0.28,0.96,1.0})
            .flexGrow(S::px(4))
            .flexShrink(S::px(1))
            .minHeight(S::px(90))
            .maxHeight(S::px(150))
            .cornerRadius(S::px(5))
        (),
        div()
            .width(S::percent(1.0))
            .color(simd_float4{1.00,0.48,0.12,1.0})
            .flexGrow(S::px(2))
            .flexShrink(S::px(3))
            .minHeight(S::px(38))
            .maxHeight(S::px(90))
            .cornerRadius(S::px(5))
        ()
    ),

    div()
        .color(simd_float4{0.12,0.12,0.15,1.0})
        .display(gui::Display::Grid)
        .gridTemplateColumns({S::percent(0.45), S::percent(0.55)})
        .gridTemplateRows({S::percent(1.0)})
        .gridColumnGap(S::px(10))
        .padding(S::px(12))
        .minWidth(S::px(260))
        .maxWidth(S::px(560))
    (
        div()
            .height(S::percent(1.0))
            .color(simd_float4{0.00,0.78,0.58,1.0})
            .minWidth(S::px(140))
            .maxWidth(S::px(220))
            .cornerRadius(S::px(5))
        (),
        div()
            .height(S::percent(1.0))
            .color(simd_float4{0.96,0.22,0.62,1.0})
            .display(gui::Display::Flex)
            .flexDirection(gui::FlexDirection::Col)
            .flexGap(S::px(8))
            .padding(S::px(8))
            .minWidth(S::px(120))
            .maxWidth(S::px(300))
            .cornerRadius(S::px(5))
        (
            div()
                .width(S::percent(1.0))
                .color(simd_float4{0.98,0.92,0.32,1.0})
                .flexGrow(S::px(2))
                .minHeight(S::px(36))
                .maxHeight(S::px(80))
                .cornerRadius(S::px(4))
            (),
            div()
                .width(S::percent(1.0))
                .color(simd_float4{0.16,0.36,0.98,1.0})
                .flexGrow(S::px(1))
                .minHeight(S::px(52))
                .maxHeight(S::px(100))
                .cornerRadius(S::px(4))
            ()
        )
    ),

    div()
        .color(simd_float4{0.12,0.12,0.15,1.0})
        .display(gui::Display::Flex)
        .flexDirection(gui::FlexDirection::Row)
        .flexWrap(gui::FlexWrap::Wrap)
        .flexGap(S::px(10))
        .padding(S::px(12))
        .minHeight(S::px(150))
        .maxHeight(S::px(260))
    (
        div()
            .height(S::px(52))
            .color(simd_float4{0.94,0.18,0.18,1.0})
            .flexGrow(S::px(1))
            .flexShrink(S::px(1))
            .minWidth(S::px(130))
            .maxWidth(S::px(210))
            .cornerRadius(S::px(5))
        (),
        div()
            .height(S::px(52))
            .color(simd_float4{0.16,0.84,0.90,1.0})
            .flexGrow(S::px(3))
            .flexShrink(S::px(1))
            .minWidth(S::px(90))
            .maxWidth(S::px(160))
            .cornerRadius(S::px(5))
        (),
        div()
            .height(S::px(52))
            .color(simd_float4{0.94,0.84,0.14,1.0})
            .flexGrow(S::px(1))
            .flexShrink(S::px(2))
            .minWidth(S::px(150))
            .maxWidth(S::px(240))
            .cornerRadius(S::px(5))
        (),
        div()
            .height(S::px(52))
            .color(simd_float4{0.54,0.28,0.98,1.0})
            .flexGrow(S::px(2))
            .flexShrink(S::px(1))
            .minWidth(S::px(100))
            .maxWidth(S::px(190))
            .cornerRadius(S::px(5))
        ()
    )
);
*/

/*
div()
    .width(S::percent(1.0))
    .height(S::percent(1.0))
    .color(simd_float4{0.04,0.04,0.05,1.0})
    .display(gui::Display::Grid)
    .gridTemplateColumns({S::percent(0.5), S::percent(0.5)})
    .gridTemplateRows({S::percent(0.5), S::percent(0.5)})
    .gridColumnGap(S::px(14))
    .gridRowGap(S::px(14))
    .padding(S::px(24))
(
    div()
        .color(simd_float4{0.12,0.12,0.15,1.0})
        .overflow(gui::Overflow::Scroll)
        .padding(S::px(12))
        .minWidth(S::px(260))
        .maxWidth(S::px(520))
        .minHeight(S::px(150))
        .maxHeight(S::px(260))
    (
        div()
            .width(S::px(640))
            .height(S::px(56))
            .color(simd_float4{0.95,0.18,0.22,1.0})
            .cornerRadius(S::px(5))
        (),
        div()
            .width(S::px(500))
            .height(S::px(56))
            .color(simd_float4{0.10,0.72,0.95,1.0})
            .marginTop(S::px(10))
            .cornerRadius(S::px(5))
        (),
        div()
            .width(S::px(760))
            .height(S::px(56))
            .color(simd_float4{0.95,0.84,0.16,1.0})
            .marginTop(S::px(10))
            .cornerRadius(S::px(5))
        ()
    ),

    div()
        .color(simd_float4{0.12,0.12,0.15,1.0})
        .overflow(gui::Overflow::Scroll)
        .display(gui::Display::Flex)
        .flexDirection(gui::FlexDirection::Col)
        .flexGap(S::px(10))
        .padding(S::px(12))
        .minWidth(S::px(220))
        .maxWidth(S::px(480))
        .minHeight(S::px(160))
        .maxHeight(S::px(260))
    (
        div()
            .width(S::percent(1.0))
            .height(S::px(90))
            .color(simd_float4{0.32,0.95,0.42,1.0})
            .minWidth(S::px(360))
            .cornerRadius(S::px(5))
        (),
        div()
            .width(S::percent(1.0))
            .height(S::px(120))
            .color(simd_float4{0.68,0.28,0.96,1.0})
            .minWidth(S::px(520))
            .cornerRadius(S::px(5))
        (),
        div()
            .width(S::percent(1.0))
            .height(S::px(90))
            .color(simd_float4{1.00,0.48,0.12,1.0})
            .minWidth(S::px(420))
            .cornerRadius(S::px(5))
        ()
    ),

    div()
        .color(simd_float4{0.12,0.12,0.15,1.0})
        .overflow(gui::Overflow::Scroll)
        .display(gui::Display::Grid)
        .gridTemplateColumns({S::px(220), S::px(260), S::px(180)})
        .gridTemplateRows({S::px(110), S::px(130)})
        .gridColumnGap(S::px(10))
        .gridRowGap(S::px(10))
        .padding(S::px(12))
        .minWidth(S::px(260))
        .maxWidth(S::px(560))
        .minHeight(S::px(180))
        .maxHeight(S::px(300))
    (
        div()
            .color(simd_float4{0.00,0.78,0.58,1.0})
            .minWidth(S::px(220))
            .minHeight(S::px(110))
            .cornerRadius(S::px(5))
        (),
        div()
            .color(simd_float4{0.96,0.22,0.62,1.0})
            .minWidth(S::px(260))
            .minHeight(S::px(110))
            .cornerRadius(S::px(5))
        (),
        div()
            .color(simd_float4{0.98,0.92,0.32,1.0})
            .minWidth(S::px(180))
            .minHeight(S::px(110))
            .cornerRadius(S::px(5))
        (),
        div()
            .color(simd_float4{0.16,0.36,0.98,1.0})
            .minWidth(S::px(220))
            .minHeight(S::px(130))
            .cornerRadius(S::px(5))
        (),
        div()
            .color(simd_float4{0.94,0.18,0.18,1.0})
            .minWidth(S::px(260))
            .minHeight(S::px(130))
            .cornerRadius(S::px(5))
        (),
        div()
            .color(simd_float4{0.16,0.84,0.90,1.0})
            .minWidth(S::px(180))
            .minHeight(S::px(130))
            .cornerRadius(S::px(5))
        ()
    ),

    div()
        .color(simd_float4{0.12,0.12,0.15,1.0})
        .overflow(gui::Overflow::Scroll)
        .display(gui::Display::Flex)
        .flexDirection(gui::FlexDirection::Row)
        .flexGap(S::px(10))
        .padding(S::px(12))
        .minWidth(S::px(260))
        .maxWidth(S::px(560))
        .minHeight(S::px(180))
        .maxHeight(S::px(300))
    (
        div()
            .height(S::percent(1.0))
            .color(simd_float4{0.95,0.84,0.16,1.0})
            .flexShrink(S::px(0))
            .width(S::px(180))
            .cornerRadius(S::px(5))
        (),
        div()
            .height(S::percent(1.0))
            .color(simd_float4{0.54,0.28,0.98,1.0})
            .flexShrink(S::px(0))
            .width(S::px(240))
            .cornerRadius(S::px(5))
        (),
        div()
            .height(S::percent(1.0))
            .color(simd_float4{0.32,0.95,0.42,1.0})
            .flexShrink(S::px(0))
            .width(S::px(300))
            .cornerRadius(S::px(5))
        ()
    )
);
*/

/*
div()
    .width(S::percent(1.0))
    .height(S::percent(1.0))
    .color(simd_float4{0.04,0.04,0.05,1.0})
    .display(gui::Display::Grid)
    .gridTemplateColumns({S::percent(0.5), S::percent(0.5)})
    .gridTemplateRows({S::percent(0.5), S::percent(0.5)})
    .gridColumnGap(S::px(14))
    .gridRowGap(S::px(14))
    .padding(S::px(24))
(
    div()
        .color(simd_float4{0.12,0.12,0.15,1.0})
        .display(gui::Display::Flex)
        .flexDirection(gui::FlexDirection::Row)
        .justifyContent(gui::JustifyContent::SpaceBetween)
        .alignItems(gui::AlignItems::Center)
        .padding(S::px(12))
        .minWidth(S::px(260))
        .maxWidth(S::px(680))
        .minHeight(S::px(170))
    (
        div()
            .width(S::px(70))
            .height(S::px(50))
            .color(simd_float4{0.95,0.18,0.22,1.0})
            .alignSelf(gui::AlignSelf::FlexStart)
            .cornerRadius(S::px(5))
        (),
        div()
            .width(S::px(90))
            .height(S::px(80))
            .color(simd_float4{0.10,0.72,0.95,1.0})
            .alignSelf(gui::AlignSelf::Center)
            .cornerRadius(S::px(5))
        (),
        div()
            .width(S::px(80))
            .height(S::px(55))
            .color(simd_float4{0.95,0.84,0.16,1.0})
            .alignSelf(gui::AlignSelf::FlexEnd)
            .cornerRadius(S::px(5))
        ()
    ),

    div()
        .color(simd_float4{0.12,0.12,0.15,1.0})
        .display(gui::Display::Flex)
        .flexDirection(gui::FlexDirection::Col)
        .justifyContent(gui::JustifyContent::SpaceBetween)
        .alignItems(gui::AlignItems::FlexEnd)
        .padding(S::px(12))
        .minWidth(S::px(240))
        .minHeight(S::px(180))
        .maxHeight(S::px(360))
    (
        div()
            .width(S::px(70))
            .height(S::px(44))
            .color(simd_float4{0.32,0.95,0.42,1.0})
            .alignSelf(gui::AlignSelf::FlexStart)
            .cornerRadius(S::px(5))
        (),
        div()
            .width(S::px(150))
            .height(S::px(60))
            .color(simd_float4{0.68,0.28,0.96,1.0})
            .alignSelf(gui::AlignSelf::Center)
            .cornerRadius(S::px(5))
        (),
        div()
            .width(S::px(100))
            .height(S::px(52))
            .color(simd_float4{1.00,0.48,0.12,1.0})
            .alignSelf(gui::AlignSelf::Auto)
            .cornerRadius(S::px(5))
        ()
    ),

    div()
        .color(simd_float4{0.12,0.12,0.15,1.0})
        .display(gui::Display::Grid)
        .gridTemplateColumns({S::percent(0.5), S::percent(0.5)})
        .gridTemplateRows({S::percent(0.5), S::percent(0.5)})
        .gridColumnGap(S::px(10))
        .gridRowGap(S::px(10))
        .alignItems(gui::AlignItems::Center)
        .padding(S::px(12))
        .minWidth(S::px(260))
        .minHeight(S::px(180))
    (
        div()
            .width(S::px(80))
            .height(S::px(46))
            .color(simd_float4{0.00,0.78,0.58,1.0})
            .alignSelf(gui::AlignSelf::FlexStart)
            .cornerRadius(S::px(5))
        (),
        div()
            .width(S::px(110))
            .height(S::px(70))
            .color(simd_float4{0.96,0.22,0.62,1.0})
            .alignSelf(gui::AlignSelf::Center)
            .cornerRadius(S::px(5))
        (),
        div()
            .width(S::px(90))
            .height(S::px(52))
            .color(simd_float4{0.98,0.92,0.32,1.0})
            .alignSelf(gui::AlignSelf::FlexEnd)
            .cornerRadius(S::px(5))
        (),
        div()
            .color(simd_float4{0.16,0.36,0.98,1.0})
            .alignSelf(gui::AlignSelf::Stretch)
            .minWidth(S::px(90))
            .maxWidth(S::px(180))
            .minHeight(S::px(44))
            .maxHeight(S::px(90))
            .cornerRadius(S::px(5))
        ()
    ),

    div()
        .color(simd_float4{0.12,0.12,0.15,1.0})
        .display(gui::Display::Flex)
        .flexDirection(gui::FlexDirection::Row)
        .justifyContent(gui::JustifyContent::Center)
        .alignItems(gui::AlignItems::Stretch)
        .flexGap(S::px(12))
        .padding(S::px(12))
        .minWidth(S::px(260))
        .minHeight(S::px(180))
    (
        div()
            .width(S::px(74))
            .color(simd_float4{0.94,0.18,0.18,1.0})
            // .alignSelf(gui::AlignSelf::Stretch)
            // .height(S::px(100))
            .minHeight(S::px(60))
            .maxHeight(S::px(150))
            .cornerRadius(S::px(5))
        (),
        div()
            .width(S::px(120))
            .height(S::px(70))
            .color(simd_float4{0.16,0.84,0.90,1.0})
            .alignSelf(gui::AlignSelf::Center)
            .cornerRadius(S::px(5))
        (),
        div()
            .width(S::px(90))
            .height(S::px(56))
            .color(simd_float4{0.54,0.28,0.98,1.0})
            .alignSelf(gui::AlignSelf::FlexEnd)
            .cornerRadius(S::px(5))
        ()
    )
);
*/


/*
div()
    .width(S::percent(1.0))
    .height(S::percent(1.0))
    .color(simd_float4{0.04,0.04,0.05,1.0})
    .display(gui::Display::Grid)
    .gridTemplateColumns({S::fr(1.0), S::fr(1.0)})
    .gridTemplateRows({S::fr(1.0), S::fr(1.0)})
    .gridColumnGap(S::px(14))
    .gridRowGap(S::px(14))
    .padding(S::px(24))
(
    div()
        .color(simd_float4{0.12,0.12,0.15,1.0})
        .display(gui::Display::Flex)
        .flexDirection(gui::FlexDirection::Row)
        .flexGap(S::px(10))
        .padding(S::px(12))
        .minWidth(S::px(260))
        .maxWidth(S::px(700))
        .minHeight(S::px(170))
    (
        div()
            .width(S::percent(0.35))
            .height(S::percent(1.0))
            .color(simd_float4{0.95,0.18,0.22,1.0})
            .minWidth(S::px(90))
            .maxWidth(S::px(180))
            .cornerRadius(S::px(5))
        (),
        div()
            .width(S::px(140))
            .height(S::percent(1.0))
            .color(simd_float4{0.10,0.72,0.95,1.0})
            .minWidth(S::px(100))
            .maxWidth(S::px(220))
            .cornerRadius(S::px(5))
        (),
        div()
            .height(S::percent(1.0))
            .color(simd_float4{0.95,0.84,0.16,1.0})
            .flexGrow(S::px(1))
            .minWidth(S::px(80))
            .maxWidth(S::px(260))
            .cornerRadius(S::px(5))
        ()
    ),

    div()
        .color(simd_float4{0.12,0.12,0.15,1.0})
        .display(gui::Display::Flex)
        .flexDirection(gui::FlexDirection::Col)
        .flexGap(S::px(10))
        .padding(S::px(12))
        .minWidth(S::px(240))
        .minHeight(S::px(180))
        .maxHeight(S::px(360))
    (
        div()
            .width(S::percent(1.0))
            .height(S::percent(0.25))
            .color(simd_float4{0.32,0.95,0.42,1.0})
            .minHeight(S::px(42))
            .maxHeight(S::px(90))
            .cornerRadius(S::px(5))
        (),
        div()
            .width(S::px(180))
            .height(S::px(64))
            .color(simd_float4{0.68,0.28,0.96,1.0})
            .minWidth(S::px(120))
            .maxWidth(S::px(260))
            .cornerRadius(S::px(5))
        (),
        div()
            .width(S::percent(0.75))
            .color(simd_float4{1.00,0.48,0.12,1.0})
            .flexGrow(S::px(1))
            .minHeight(S::px(50))
            .maxHeight(S::px(130))
            .cornerRadius(S::px(5))
        ()
    ),

    div()
        .color(simd_float4{0.12,0.12,0.15,1.0})
        .display(gui::Display::Grid)
        .gridTemplateColumns({S::px(140), S::percent(0.5), S::fr(1.0)})
        .gridTemplateRows({S::px(70), S::percent(0.45), S::fr(1.0)})
        .gridColumnGap(S::px(10))
        .gridRowGap(S::px(10))
        .padding(S::px(12))
        .minWidth(S::px(300))
        .minHeight(S::px(190))
    (
        div()
            .color(simd_float4{0.00,0.78,0.58,1.0})
            .minWidth(S::px(90))
            .minHeight(S::px(46))
            .cornerRadius(S::px(5))
        (),
        div()
            .color(simd_float4{0.96,0.22,0.62,1.0})
            .minWidth(S::px(130))
            .maxWidth(S::px(220))
            .minHeight(S::px(50))
            .cornerRadius(S::px(5))
        (),
        div()
            .color(simd_float4{0.98,0.92,0.32,1.0})
            .minWidth(S::px(80))
            .maxWidth(S::px(180))
            .minHeight(S::px(50))
            .cornerRadius(S::px(5))
        (),
        div()
            .color(simd_float4{0.16,0.36,0.98,1.0})
            .gridColumn(1, 3)
            .minHeight(S::px(70))
            .maxHeight(S::px(120))
            .cornerRadius(S::px(5))
        (),
        div()
            .color(simd_float4{0.94,0.18,0.18,1.0})
            .minWidth(S::px(90))
            .minHeight(S::px(60))
            .cornerRadius(S::px(5))
        ()
    ),

    div()
        .color(simd_float4{0.12,0.12,0.15,1.0})
        .display(gui::Display::Flex)
        .flexDirection(gui::FlexDirection::Row)
        .flexWrap(gui::FlexWrap::Wrap)
        .flexGap(S::px(10))
        .padding(S::px(12))
        .minWidth(S::px(260))
        .minHeight(S::px(180))
        .maxHeight(S::px(320))
    (
        div()
            .width(S::percent(0.4))
            .height(S::px(54))
            .color(simd_float4{0.16,0.84,0.90,1.0})
            .minWidth(S::px(110))
            .maxWidth(S::px(220))
            .cornerRadius(S::px(5))
        (),
        div()
            .height(S::px(54))
            .color(simd_float4{0.54,0.28,0.98,1.0})
            .flexGrow(S::px(1))
            .minWidth(S::px(90))
            .maxWidth(S::px(180))
            .cornerRadius(S::px(5))
        (),
        div()
            .width(S::px(150))
            .height(S::px(54))
            .color(simd_float4{0.95,0.84,0.16,1.0})
            .minWidth(S::px(120))
            .maxWidth(S::px(210))
            .cornerRadius(S::px(5))
        (),
        div()
            .width(S::percent(0.65))
            .height(S::px(54))
            .color(simd_float4{0.32,0.95,0.42,1.0})
            .minWidth(S::px(160))
            .maxWidth(S::px(300))
            .cornerRadius(S::px(5))
        ()
    )
);
*/

// div()
//     .width(S::percent(1.0))
//     .height(S::percent(1.0))
//     .color(simd_float4{0.04,0.04,0.05,1.0})
//     .display(gui::Display::Grid)
//     .gridTemplateColumns({S::fr(1.0), S::fr(1.0)})
//     .gridTemplateRows({S::fr(1.0), S::fr(1.0)})
//     .gridColumnGap(S::px(20))
//     .gridRowGap(S::px(20))
//     .padding(S::px(40))
// (
//     div()
//         .color(simd_float4{0.12,0.12,0.15,1.0})
//         .display(gui::Display::Grid)
//         .gridTemplateColumns({S::fr(1.0)})
//         .gridTemplateRows({S::fr(1.0)})
//         .justifyItems(gui::JustifyItems::Start)
//         .alignItems(gui::AlignItems::Center)
//         .padding(S::px(20))
//     (
//         div()
//             .width(S::px(120))
//             .height(S::px(80))
//             .color(simd_float4{0.95,0.18,0.22,1.0})
//             .cornerRadius(S::px(5))
//         ()
//     ),

//     div()
//         .color(simd_float4{0.12,0.12,0.15,1.0})
//         .display(gui::Display::Grid)
//         .gridTemplateColumns({S::fr(1.0)})
//         .gridTemplateRows({S::fr(1.0)})
//         .justifyItems(gui::JustifyItems::Center)
//         .alignItems(gui::AlignItems::Center)
//         .padding(S::px(20))
//     (
//         div()
//             .width(S::px(120))
//             .height(S::px(80))
//             .color(simd_float4{0.32,0.95,0.42,1.0})
//             .cornerRadius(S::px(5))
//         ()
//     ),

//     div()
//         .color(simd_float4{0.12,0.12,0.15,1.0})
//         .display(gui::Display::Grid)
//         .gridTemplateColumns({S::fr(1.0)})
//         .gridTemplateRows({S::fr(1.0)})
//         .justifyItems(gui::JustifyItems::End)
//         .alignItems(gui::AlignItems::Center)
//         .padding(S::px(20))
//     (
//         div()
//             .width(S::px(120))
//             .height(S::px(80))
//             .justifySelf(gui::JustifySelf::Start)
//             .color(simd_float4{0.10,0.72,0.95,1.0})
//             .cornerRadius(S::px(5))
//         ()
//     ),

//     div()
//         .color(simd_float4{0.12,0.12,0.15,1.0})
//         .display(gui::Display::Grid)
//         .gridTemplateColumns({S::fr(1.0)})
//         .gridTemplateRows({S::fr(1.0)})
//         .justifyItems(gui::JustifyItems::Start)
//         .alignItems(gui::AlignItems::Center)
//         .padding(S::px(20))
//     (
//         div()
//             .height(S::px(80))
//             .justifySelf(gui::JustifySelf::Stretch)
//             .color(simd_float4{0.54,0.28,0.98,1.0})
//             .cornerRadius(S::px(5))
//         ()
//     )
// );



// using S = gui::Size;
// // complex test scene
// div()
//     .width(S::percent(1.0))
//     .height(S::percent(1.0))
//     .color(simd_float4{0.04,0.04,0.05,1.0})
//     .display(gui::Display::Grid)
//     .gridTemplateColumns({S::px(260), S::percent(0.35), S::fr(1.0)})
//     .gridTemplateRows({S::px(178), S::fr(1.0), S::px(148)})
//     .gridColumnGap(S::px(14))
//     .gridRowGap(S::px(14))
//     .padding(S::px(22))
// (
//     div()
//         .gridColumn(1, 4)
//         .gridRow(1, 2)
//         .color(simd_float4{0.12,0.12,0.15,1.0})
//         .display(gui::Display::Flex)
//         .flexDirection(gui::FlexDirection::Row)
//         .flexGap(S::px(12))
//         .padding(S::px(12))
//         .minHeight(S::px(150))
//     (
//         div().width(S::px(170)).height(S::percent(1.0)).minWidth(S::px(120)).maxWidth(S::px(220)).color(simd_float4{0.00,0.78,0.58,1.0}).cornerRadius(S::px(5))(),
//         div().width(S::percent(0.28)).height(S::percent(1.0)).minWidth(S::px(180)).maxWidth(S::px(420)).color(simd_float4{0.96,0.22,0.62,1.0}).cornerRadius(S::px(5))(),
//         div().height(S::percent(1.0)).flexGrow(S::px(1)).minWidth(S::px(180)).maxWidth(S::px(520)).color(simd_float4{0.98,0.92,0.32,1.0}).cornerRadius(S::px(5))(),
//         div().width(S::px(140)).height(S::percent(1.0)).minWidth(S::px(110)).maxWidth(S::px(180)).color(simd_float4{0.16,0.84,0.90,1.0}).cornerRadius(S::px(5))()
//     ),

//     div()
//         .gridColumn(1, 2)
//         .gridRow(2, 3)
//         .color(simd_float4{0.12,0.12,0.15,1.0})
//         .display(gui::Display::Flex)
//         .flexDirection(gui::FlexDirection::Col)
//         .flexGap(S::px(10))
//         .padding(S::px(12))
//         .minWidth(S::px(220))
//         .minHeight(S::px(220))
//     (
//         div().width(S::percent(1.0)).height(S::percent(0.22)).minHeight(S::px(46)).maxHeight(S::px(90)).color(simd_float4{0.95,0.18,0.22,1.0}).cornerRadius(S::px(5))(),
//         div().width(S::px(155)).height(S::px(62)).minWidth(S::px(120)).maxWidth(S::px(210)).color(simd_float4{0.10,0.72,0.95,1.0}).cornerRadius(S::px(5))(),
//         div().width(S::percent(0.72)).flexGrow(S::px(1)).minHeight(S::px(70)).maxHeight(S::px(180)).color(simd_float4{1.00,0.48,0.12,1.0}).cornerRadius(S::px(5))(),
//         div().width(S::percent(0.45)).height(S::px(48)).minWidth(S::px(90)).maxWidth(S::px(150)).color(simd_float4{0.54,0.28,0.98,1.0}).cornerRadius(S::px(5))()
//     ),

//     div()
//         .gridColumn(2, 3)
//         .gridRow(2, 3)
//         .color(simd_float4{0.12,0.12,0.15,1.0})
//         .display(gui::Display::Grid)
//         .gridTemplateColumns({S::px(120), S::percent(0.45), S::fr(1.0)})
//         .gridTemplateRows({S::px(64), S::percent(0.5), S::fr(1.0)})
//         .gridColumnGap(S::px(10))
//         .gridRowGap(S::px(10))
//         .padding(S::px(12))
//         .minWidth(S::px(300))
//         .minHeight(S::px(220))
//     (
//         div().color(simd_float4{0.32,0.95,0.42,1.0}).minWidth(S::px(90)).minHeight(S::px(46)).cornerRadius(S::px(5))(),
//         div().color(simd_float4{0.68,0.28,0.96,1.0}).minWidth(S::px(130)).maxWidth(S::px(240)).minHeight(S::px(50)).cornerRadius(S::px(5))(),
//         div().color(simd_float4{0.98,0.92,0.32,1.0}).minWidth(S::px(80)).maxWidth(S::px(170)).minHeight(S::px(50)).cornerRadius(S::px(5))(),
//         div().gridColumn(1, 3).color(simd_float4{0.16,0.36,0.98,1.0}).minHeight(S::px(74)).maxHeight(S::px(130)).cornerRadius(S::px(5))(),
//         div().color(simd_float4{0.94,0.18,0.18,1.0}).minWidth(S::px(90)).minHeight(S::px(60)).cornerRadius(S::px(5))(),
//         div().gridColumn(1, 4).color(simd_float4{0.16,0.84,0.90,1.0}).minHeight(S::px(46)).maxHeight(S::px(80)).cornerRadius(S::px(5))()
//     ),

//     div()
//         .gridColumn(3, 4)
//         .gridRow(2, 3)
//         .color(simd_float4{0.12,0.12,0.15,1.0})
//         .display(gui::Display::Flex)
//         .flexDirection(gui::FlexDirection::Row)
//         .flexWrap(gui::FlexWrap::Wrap)
//         .flexGap(S::px(10))
//         .padding(S::px(12))
//         .minWidth(S::px(260))
//         .minHeight(S::px(220))
//     (
//         div().width(S::percent(0.36)).height(S::px(58)).minWidth(S::px(110)).maxWidth(S::px(210)).color(simd_float4{0.95,0.84,0.16,1.0}).cornerRadius(S::px(5))(),
//         div().height(S::px(58)).flexGrow(S::px(1)).minWidth(S::px(120)).maxWidth(S::px(240)).color(simd_float4{0.00,0.78,0.58,1.0}).cornerRadius(S::px(5))(),
//         div().width(S::px(180)).height(S::px(58)).minWidth(S::px(140)).maxWidth(S::px(220)).color(simd_float4{0.96,0.22,0.62,1.0}).cornerRadius(S::px(5))(),
//         div().width(S::percent(0.62)).height(S::px(58)).minWidth(S::px(180)).maxWidth(S::px(340)).color(simd_float4{0.16,0.36,0.98,1.0}).cornerRadius(S::px(5))(),
//         div().height(S::px(58)).flexGrow(S::px(1)).minWidth(S::px(90)).maxWidth(S::px(160)).color(simd_float4{1.00,0.48,0.12,1.0}).cornerRadius(S::px(5))()
//     ),

//     div()
//         .gridColumn(1, 4)
//         .gridRow(3, 4)
//         .color(simd_float4{0.12,0.12,0.15,1.0})
//         .display(gui::Display::Grid)
//         .gridTemplateColumns({S::percent(0.25), S::px(220), S::fr(1.0), S::px(160)})
//         .gridTemplateRows({S::fr(1.0)})
//         .gridColumnGap(S::px(12))
//         .padding(S::px(12))
//         .minHeight(S::px(120))
//     (
//         div().color(simd_float4{0.68,0.28,0.96,1.0}).minWidth(S::px(120)).cornerRadius(S::px(5))(),
//         div().color(simd_float4{0.10,0.72,0.95,1.0}).minWidth(S::px(160)).maxWidth(S::px(220)).cornerRadius(S::px(5))(),
//         div().color(simd_float4{0.32,0.95,0.42,1.0}).minWidth(S::px(220)).cornerRadius(S::px(5))(),
//         div().color(simd_float4{0.95,0.18,0.22,1.0}).minWidth(S::px(120)).cornerRadius(S::px(5))()
//     )
// );


// using S = gui::Size;
// constexpr auto butterflyPath = "/Users/treja/projects/gui/assets/butterfly.png";

// div()
//     .width(S::percent(1.0))
//     .height(S::percent(1.0))
//     .padding(S::px(24))
//     .overflow(gui::Overflow::Scroll)
//     .color(simd_float4{0.055,0.067,0.086,1.0})
//     .display(gui::Display::Flex)
//     .flexDirection(gui::FlexDirection::Col)
//     .flexGap(S::px(18))
// (
//     div()
//         .width(S::percent(1.0))
//         .height(S::px(56))
//         .flexShrink(S::px(0))
//         .paddingLeft(S::px(18))
//         .paddingRight(S::px(18))
//         .display(gui::Display::Flex)
//         .alignItems(gui::AlignItems::Center)
//         .justifyContent(gui::JustifyContent::SpaceBetween)
//         .color(simd_float4{0.102,0.122,0.161,1.0})
//     (
//         text("FIELD NOTES")
//             .font(ArialBold)
//             .fontSize(S::pt(24))
//             .color(simd_float4{1.0,1.0,1.0,1.0}),
//         text("Aspect-ratio layout study")
//             .font(Arial)
//             .fontSize(S::pt(16))
//             .color(simd_float4{0.522,0.569,0.678,1.0})
//     ),

//     div()
//         .width(S::percent(1.0))
//         .display(gui::Display::Flex)
//         .alignItems(gui::AlignItems::FlexStart)
//         .flexGap(S::px(18))
//     (
//         div()
//             .minWidth(S::px(0))
//             .flexGrow(S::px(1))
//             .display(gui::Display::Flex)
//             .flexDirection(gui::FlexDirection::Col)
//             .flexGap(S::px(16))
//         (
//             div()
//                 .position(gui::Position::Relative)
//                 .width(S::percent(1.0))
//                 .height(S::autoSize())
//                 .aspectRatio(4, 1)
//                 .padding(S::px(24))
//                 .color(simd_float4{0.180,0.478,0.722,1.0})
//             (
//                 div()
//                 (
//                     text("DESERT LIGHT")
//                         .font(ArialBold)
//                         .fontSize(S::pt(37))
//                         .color(simd_float4{1.0,1.0,1.0,1.0})
//                 ),
//                 div()
//                 (
//                     text("A responsive hero whose height follows its final width.")
//                         .font(Arial)
//                         .fontSize(S::pt(17))
//                         .color(simd_float4{0.820,0.902,0.980,1.0})
//                 ),
//                 div()
//                     .position(gui::Position::Absolute)
//                     .width(S::px(164))
//                     .height(S::autoSize())
//                     .aspectRatio(2, 1)
//                     .right(S::px(24))
//                     .top(S::px(24))
//                     .padding(S::px(12))
//                     .color(simd_float4{0.980,0.761,0.200,1.0})
//                 (
//                     text("FEATURED")
//                         .font(ArialBold)
//                         .fontSize(S::pt(15))
//                         .color(simd_float4{0.161,0.122,0.039,1.0})
//                 )
//             ),

//             div()
//                 .width(S::percent(1.0))
//                 .display(gui::Display::Grid)
//                 .gridTemplateColumns({S::fr(1), S::fr(1), S::fr(1)})
//                 .gridColumnGap(S::px(14))
//             (
//                 div()
//                     .width(S::percent(1.0))
//                     .height(S::autoSize())
//                     .aspectRatio(5, 3)
//                     .padding(S::px(16))
//                     .color(simd_float4{0.961,0.302,0.459,1.0})
//                 (
//                     text("CANYON").font(ArialBold).fontSize(S::pt(16)).color(simd_float4{1.0,1.0,1.0,1.0})
//                 ),
//                 div()
//                     .width(S::percent(1.0))
//                     .height(S::autoSize())
//                     .aspectRatio(5, 3)
//                     .padding(S::px(16))
//                     .color(simd_float4{0.380,0.922,0.561,1.0})
//                 (
//                     text("TIDELINE").font(ArialBold).fontSize(S::pt(16)).color(simd_float4{0.039,0.141,0.090,1.0})
//                 ),
//                 div()
//                     .width(S::percent(1.0))
//                     .height(S::autoSize())
//                     .aspectRatio(5, 3)
//                     .padding(S::px(16))
//                     .color(simd_float4{0.639,0.420,0.961,1.0})
//                 (
//                     text("NIGHTFALL").font(ArialBold).fontSize(S::pt(16)).color(simd_float4{1.0,1.0,1.0,1.0})
//                 )
//             )
//         ),

//         div()
//             .width(S::px(300))
//             .flexShrink(S::px(0))
//             .padding(S::px(14))
//             .display(gui::Display::Flex)
//             .flexDirection(gui::FlexDirection::Col)
//             .flexGap(S::px(14))
//             .color(simd_float4{0.102,0.122,0.161,1.0})
//         (
//             image(butterflyPath, S::percent(1.0), S::autoSize())
//                 .aspectRatio(1, 1),
//             text("SPECIMEN 07")
//                 .font(ArialBold)
//                 .fontSize(S::pt(19))
//                 .color(simd_float4{1.0,1.0,1.0,1.0}),
//             text("The replaced image shares the same square presentation as the GUI reference.")
//                 .font(Arial)
//                 .fontSize(S::pt(16))
//                 .color(simd_float4{0.522,0.569,0.678,1.0}),
//             div()
//                 .width(S::percent(1.0))
//                 .height(S::autoSize())
//                 .aspectRatio(3, 1)
//                 .padding(S::px(12))
//                 .color(simd_float4{0.980,0.761,0.200,1.0})
//             (
//                 text("VIEW COLLECTION")
//                     .font(ArialBold)
//                     .fontSize(S::pt(15))
//                     .color(simd_float4{0.161,0.122,0.039,1.0})
//             )
//         )
//     ),

//     div()
//         .width(S::percent(1.0))
//         .height(S::autoSize())
//         .maxHeight(S::px(120))
//         .aspectRatio(10, 1)
//         .flexShrink(S::px(0))
//         .paddingLeft(S::px(22))
//         .paddingRight(S::px(22))
//         .display(gui::Display::Flex)
//         .alignItems(gui::AlignItems::Center)
//         .justifyContent(gui::JustifyContent::SpaceBetween)
//         .color(simd_float4{0.141,0.161,0.212,1.0})
//     (
//         text("Build a collection around the dimensions that matter.")
//             .font(ArialBold)
//             .fontSize(S::pt(19))
//             .color(simd_float4{1.0,1.0,1.0,1.0}),
//         text("EXPLORE ->")
//             .font(ArialBold)
//             .fontSize(S::pt(16))
//             .color(simd_float4{0.980,0.761,0.200,1.0})
//     ),

//     div()
//         .position(gui::Position::Relative)
//         .width(S::percent(1.0))
//         .height(S::px(220))
//         .flexShrink(S::px(0))
//         .padding(S::px(22))
//         .color(simd_float4{0.102,0.122,0.161,1.0})
//     (
//         div()
//         (
//             text("FLOATING EXHIBIT")
//                 .font(ArialBold)
//                 .fontSize(S::pt(19))
//                 .color(simd_float4{1.0,1.0,1.0,1.0})
//         ),
//         div()
//         (
//             text("This inset-resolved card intentionally crosses the section boundary.")
//                 .font(Arial)
//                 .fontSize(S::pt(16))
//                 .color(simd_float4{0.522,0.569,0.678,1.0})
//         ),
//         div()
//             .position(gui::Position::Absolute)
//             .left(S::px(32))
//             .right(S::px(160))
//             .top(S::px(76))
//             .width(S::autoSize())
//             .height(S::autoSize())
//             .aspectRatio(6, 1)
//             .padding(S::px(18))
//             .color(simd_float4{0.961,0.302,0.459,1.0})
//         (
//             div()
//                 .width(S::percent(0.46))
//                 .height(S::percent(0.34))
//                 .color(simd_float4{0.980,0.761,0.200,1.0})
//             (),
//             text("OUT OF FLOW / IN PROPORTION")
//                 .font(ArialBold)
//                 .fontSize(S::pt(17))
//                 .color(simd_float4{1.0,1.0,1.0,1.0})
//         )
//     ),

//     div()
//         .width(S::percent(1.0))
//         .height(S::px(420))
//         .flexShrink(S::px(0))
//         .paddingTop(S::px(180))
//         .paddingLeft(S::px(22))
//         .paddingRight(S::px(22))
//         .color(simd_float4{0.122,0.141,0.188,1.0})
//     (
//         text("ARCHIVE")
//             .font(ArialBold)
//             .fontSize(S::pt(20))
//             .color(simd_float4{1.0,1.0,1.0,1.0}),
//         div()
//             .width(S::percent(1.0))
//             .display(gui::Display::Flex)
//             .alignItems(gui::AlignItems::FlexStart)
//             .flexGap(S::px(14))
//             .marginTop(S::px(14))
//         (
//             div()
//                 .width(S::autoSize())
//                 .height(S::px(76))
//                 .aspectRatio(2, 1)
//                 .flexShrink(S::px(0))
//                 .color(simd_float4{0.078,0.722,0.859,1.0})
//             (),
//             div()
//                 .width(S::percent(0.38))
//                 .maxWidth(S::px(360))
//                 .height(S::autoSize())
//                 .aspectRatio(3, 1)
//                 .color(simd_float4{0.380,0.922,0.561,1.0})
//             (),
//             div()
//                 .width(S::px(180))
//                 .height(S::autoSize())
//                 .aspectRatio(3, 2)
//                 .color(simd_float4{0.639,0.420,0.961,1.0})
//             ()
//         )
//     )
// );

    // // Raster downsampling/post-layout test:
    // // - Both elements share the same 2000x2000 source asset.
    // // - The first image has a fixed 128x128 rendition.
    // // - The second image is sized by flex after measurement.
    // // Resize the window to make the flex image cross rendition bins and verify
    // // its post-layout atoms continue to use the current frame-buffer slot.
    // using S = gui::Size;
    // constexpr auto butterflyPath = "/Users/treja/projects/gui/assets/butterfly.png";

    // div(gui::Size::percent(1.0), gui::Size::percent(1.0), simd_float4{0.94,0.94,0.96,1.0})
    //     .display(gui::Display::Flex)
    //     .alignItems(gui::AlignItems::Center)
    //     .justifyContent(gui::JustifyContent::Center)
    // (
    //     div()
    //         .width(gui::Size::percent(0.8))
    //         .height(gui::Size::px(260))
    //         .minWidth(gui::Size::px(360))
    //         .maxWidth(gui::Size::px(900))
    //         .padding(gui::Size::px(20))
    //         .flexGap(gui::Size::px(20))
    //         .display(gui::Display::Flex)
    //         .alignItems(gui::AlignItems::Stretch)
    //         .color(simd_float4{1.0,1.0,1.0,1.0})
    //         .cornerRadius(gui::Size::px(12))
    //     (
    //         image(butterflyPath, gui::Size::px(128), gui::Size::px(128)),
    //         image(butterflyPath, gui::Size::autoSize(), gui::Size::autoSize())
    //             .flexGrow(gui::Size::px(1.0))
    //     )
    // );

    layout_test::scenes::buildBrowser();

    // using S = gui::Size;

    // div(S::percent(1.0), S::percent(1.0), simd_float4{0.059,0.071,0.090,1.0})
    //     .padding(S::px(32))
    // (
    //     div(S::px(360), S::px(300), simd_float4{0.122,0.137,0.169,1.0})
    //         .padding(S::px(16))
    //         .overflow(gui::Overflow::Scroll)
    //     (
    //         div(S::percent(1.0), S::px(720), simd_float4{0.169,0.192,0.239,1.0})
    //         (
    //             div(S::percent(1.0), S::px(120), simd_float4{0.090,0.722,0.831,1.0})(),
    //             div(S::percent(1.0), S::px(52), simd_float4{0.910,0.243,0.549,1.0})
    //                 .position(gui::Position::Sticky)
    //                 .top(S::px(12))
    //                 .zIndex(1)
    //             (),
    //             div(S::percent(1.0), S::px(548), simd_float4{0.961,0.761,0.200,1.0})()
    //         )
    //     )
    // );

    // using S = gui::Size;
    // constexpr auto intrinsicText = "short extraordinarilylongword short words that should wrap";

    // div(S::percent(1.0), S
    // ::percent(1.0), simd_float4{0.94,0.94,0.96,1.0})
    //     .padding(S::px(32))
    // (
    //     div(S::minContent(), S::autoSize(), simd_float4{0.20,0.70,0.90,1.0})
    //         .padding(S::px(8))
    //     (
    //         text(intrinsicText)
    //             .font(Arial)
    //             .fontSize(S::pt(18))
    //             .color(simd_float4{0.05,0.05,0.05,1.0})
    //     ),
    //     div(S::maxContent(), S::autoSize(), simd_float4{0.95,0.55,0.25,1.0})
    //         .padding(S::px(8))
    //         .marginTop(S::px(24))
    //     (
    //         text(intrinsicText)
    //             .font(Arial)
    //             .fontSize(S::pt(18))
    //             .color(simd_float4{0.05,0.05,0.05,1.0})
    //     )
    // );

    // using S = gui::Size;

    // div()
    //     // .width(S::px(50))
    //     .color(simd_float4{0.678, 0.847, 0.902, 1.0})
    //     .marginTop(S::px(32))
    // (
    //     text("hello world").width(S::px(50))
    // );

    // // Crates: a record-collection browser.
    // // Exercises sticky section headers inside a scrollport, hover/click state
    // // via MouseEnter/MouseLeave/Click, aspect-ratio + absolute badge in the
    // // detail panel, per-corner radii on chips, and flex-wrap for tags.
    // using S = gui::Size;
    // using gui::Display;
    // using gui::FlexDirection;
    // using gui::AlignItems;
    // using gui::JustifyContent;
    // using runtime::EventType;

    // constexpr simd_float4 bg          {0.043, 0.063, 0.086, 1.0};
    // constexpr simd_float4 panel       {0.082, 0.110, 0.149, 1.0};
    // constexpr simd_float4 raised      {0.114, 0.149, 0.200, 1.0};
    // constexpr simd_float4 hover       {0.153, 0.196, 0.259, 1.0};
    // constexpr simd_float4 outline     {0.165, 0.208, 0.267, 1.0};
    // constexpr simd_float4 ink         {0.949, 0.957, 0.969, 1.0};
    // constexpr simd_float4 muted       {0.541, 0.592, 0.671, 1.0};
    // constexpr simd_float4 coral       {1.000, 0.420, 0.290, 1.0};
    // constexpr simd_float4 coralDim    {0.286, 0.157, 0.141, 1.0};
    // constexpr simd_float4 cyan        {0.290, 0.816, 1.000, 1.0};
    // constexpr simd_float4 transparent {0.0, 0.0, 0.0, 0.0};

    // constexpr simd_float4 avatarA {0.980, 0.761, 0.200, 1.0};
    // constexpr simd_float4 avatarB {0.380, 0.922, 0.561, 1.0};
    // constexpr simd_float4 avatarC {0.639, 0.420, 0.961, 1.0};
    // constexpr simd_float4 avatarD {0.961, 0.302, 0.459, 1.0};
    // constexpr simd_float4 avatarE {0.290, 0.816, 1.000, 1.0};

    // constexpr auto coverPath = "/Users/treja/projects/gui/assets/loveless.jpg";
    // const std::string SF = "/System/Library/Fonts/SFNS.ttf";
    // const std::string NewYork = "/System/Library/Fonts/NewYork.ttf";

    // auto chip = [&](const char* label, simd_float4 fg, simd_float4 fill) {
    //     return div()
    //         .color(fill)
    //         .height(S::px(22))
    //         .paddingLeft(S::px(9))
    //         .paddingRight(S::px(9))
    //         .cornerRadiusTopLeft(S::px(11))
    //         .cornerRadiusBottomRight(S::px(11))
    //         .cornerRadiusTopRight(S::px(3))
    //         .cornerRadiusBottomLeft(S::px(3))
    //         .display(Display::Flex)
    //         .alignItems(AlignItems::Center)
    //     (
    //         text(label).font(SFMono).fontSize(S::pt(10)).color(fg)
    //     );
    // };

    // auto button = [&](const char* label, simd_float4 fill, simd_float4 hoverFill, simd_float4 fg) {
    //     return div()
    //         .color(fill)
    //         .height(S::px(36))
    //         .flexGrow(S::px(1))
    //         .cornerRadius(S::px(8))
    //         .display(Display::Flex)
    //         .alignItems(AlignItems::Center)
    //         .justifyContent(JustifyContent::Center)
    //         .addEventListener(EventType::MouseEnter, [hoverFill](auto& node, Event&) {
    //             node.color(hoverFill);
    //         })
    //         .addEventListener(EventType::MouseLeave, [fill](auto& node, Event&) {
    //             node.color(fill);
    //         })
    //     (
    //         text(label).font(ArialBold).fontSize(S::pt(12)).color(fg)
    //     );
    // };

    // auto row = [&](const char* initials, const char* artist, const char* meta, const char* genre, simd_float4 avatar) {
    //     auto selected = std::make_shared<bool>(false);

    //     return div()
    //         .color(transparent)
    //         .height(S::px(58))
    //         .flexShrink(S::px(0))
    //         .paddingLeft(S::px(12))
    //         .paddingRight(S::px(12))
    //         .cornerRadius(S::px(10))
    //         .display(Display::Flex)
    //         .alignItems(AlignItems::Center)
    //         .flexGap(S::px(12))
    //         .addEventListener(EventType::MouseEnter, [selected](auto& node, Event&) {
    //             if (!*selected) {
    //                 node.color(hover);
    //             }
    //         })
    //         .addEventListener(EventType::MouseLeave, [selected](auto& node, Event&) {
    //             if (!*selected) {
    //                 node.color(transparent);
    //             }
    //         })
    //         .addEventListener(EventType::Click, [selected](auto& node, Event&) {
    //             *selected = !*selected;

    //             if (*selected) {
    //                 node.color(coralDim);
    //             } else {
    //                 node.color(hover);
    //             }
    //         })
    //     (
    //         div(S::px(36), S::px(36), avatar)
    //             .flexShrink(S::px(0))
    //             .cornerRadius(S::px(18))
    //             .display(Display::Flex)
    //             .alignItems(AlignItems::Center)
    //             .justifyContent(JustifyContent::Center)
    //         (
    //             text(initials).font(DINAlternateBold).fontSize(S::pt(12)).color(bg)
    //         ),
    //         div()
    //             .flexGrow(S::px(1))
    //             .minWidth(S::px(0))
    //             .display(Display::Flex)
    //             .flexDirection(FlexDirection::Col)
    //             .flexGap(S::px(3))
    //         (
    //             text(artist).font(ArialBold).fontSize(S::pt(13)).color(ink),
    //             text(meta).font(SFMono).fontSize(S::pt(10)).color(muted)
    //         ),
    //         chip(genre, muted, raised)
    //     );
    // };

    // auto section = [&](const char* letter, const char* count, auto&&... rows) {
    //     return div()
    //         .display(Display::Flex)
    //         .flexDirection(FlexDirection::Col)
    //         .flexGap(S::px(2))
    //     (
    //         div()
    //             .position(gui::Position::Sticky)
    //             .top(S::px(0))
    //             .zIndex(1)
    //             .color(panel)
    //             .height(S::px(34))
    //             .flexShrink(S::px(0))
    //             .paddingLeft(S::px(12))
    //             .paddingRight(S::px(12))
    //             .display(Display::Flex)
    //             .alignItems(AlignItems::Center)
    //             .justifyContent(JustifyContent::SpaceBetween)
    //         (
    //             text(letter).font(DINAlternateBold).fontSize(S::pt(15)).color(coral),
    //             text(count).font(SFMono).fontSize(S::pt(10)).color(muted)
    //         ),
    //         rows...
    //     );
    // };

    // auto stat = [&](const char* label, const char* value) {
    //     return div()
    //         .display(Display::Flex)
    //         .justifyContent(JustifyContent::SpaceBetween)
    //     (
    //         text(label).font(SF).fontSize(S::pt(11)).color(muted),
    //         text(value).font(SFMono).fontSize(S::pt(11)).color(ink)
    //     );
    // };

    // div(S::percent(1.0), S::percent(1.0), bg)
    //     .paddingTop(S::px(28))
    //     .display(Display::Flex)
    //     .flexDirection(FlexDirection::Col)
    // (
    //     // Top bar
    //     div()
    //         .height(S::px(56))
    //         .flexShrink(S::px(0))
    //         .paddingLeft(S::px(20))
    //         .paddingRight(S::px(20))
    //         .display(Display::Flex)
    //         .alignItems(AlignItems::Center)
    //         .justifyContent(JustifyContent::SpaceBetween)
    //     (
    //         div()
    //             .display(Display::Flex)
    //             .alignItems(AlignItems::Center)
    //             .flexGap(S::px(12))
    //         (
    //             div(S::px(30), S::px(30), coral)
    //                 .cornerRadiusTopLeft(S::px(15))
    //                 .cornerRadiusBottomRight(S::px(15))
    //                 .cornerRadiusTopRight(S::px(4))
    //                 .cornerRadiusBottomLeft(S::px(4))
    //             (),
    //             text("CRATES").font(DINAlternateBold).fontSize(S::pt(20)).color(ink),
    //             text("16 records · 6 shelves").font(SFMono).fontSize(S::pt(11)).color(muted)
    //         ),
    //         div()
    //             .display(Display::Flex)
    //             .alignItems(AlignItems::Center)
    //             .flexGap(S::px(8))
    //         (
    //             div()
    //                 .color(panel)
    //                 .borderColor(outline)
    //                 .borderWidth(S::px(1))
    //                 .height(S::px(34))
    //                 .paddingLeft(S::px(14))
    //                 .paddingRight(S::px(80))
    //                 .cornerRadius(S::px(17))
    //                 .display(Display::Flex)
    //                 .alignItems(AlignItems::Center)
    //             (
    //                 text("Search the shelf").font(SF).fontSize(S::pt(12)).color(muted)
    //             ),
    //             div()
    //                 .color(raised)
    //                 .height(S::px(34))
    //                 .paddingLeft(S::px(14))
    //                 .paddingRight(S::px(14))
    //                 .cornerRadius(S::px(17))
    //                 .display(Display::Flex)
    //                 .alignItems(AlignItems::Center)
    //                 .addEventListener(EventType::Click, [on = false](auto& node, Event&) mutable {
    //                     on = !on;

    //                     if (on) {
    //                         node.color(coral);
    //                     } else {
    //                         node.color(raised);
    //                     }
    //                 })
    //             (
    //                 text("Shuffle").font(ArialBold).fontSize(S::pt(12)).color(ink)
    //             )
    //         )
    //     ),

    //     // Body
    //     div()
    //         .flexGrow(S::px(1))
    //         .minHeight(S::px(0))
    //         .paddingLeft(S::px(20))
    //         .paddingRight(S::px(20))
    //         .paddingBottom(S::px(20))
    //         .display(Display::Flex)
    //         .flexGap(S::px(16))
    //     (
    //         // Shelf: scrollport with sticky letter headers
    //         div()
    //             .color(panel)
    //             .borderColor(outline)
    //             .borderWidth(S::px(1))
    //             .cornerRadius(S::px(14))
    //             .flexGrow(S::px(1))
    //             .minWidth(S::px(0))
    //             .paddingLeft(S::px(8))
    //             .paddingRight(S::px(8))
    //             .overflow(gui::Overflow::Scroll)
    //         (
    //             div()
    //                 .display(Display::Flex)
    //                 .flexDirection(FlexDirection::Col)
    //                 .flexGap(S::px(10))
    //                 .paddingBottom(S::px(12))
    //             (
    //                 section("A", "3 records",
    //                     row("AT", "Aphex Twin",      "Selected Ambient Works 85-92 · 1992", "ambient",   avatarA),
    //                     row("AU", "Autechre",        "Amber · 1994",                        "idm",       avatarB),
    //                     row("AL", "Alvvays",         "Antisocialites · 2017",               "indie",     avatarC)
    //                 ),
    //                 section("B", "4 records",
    //                     row("BC", "Boards of Canada","Music Has the Right to Children · 1998", "electronic", avatarD),
    //                     row("BR", "Broadcast",       "Tender Buttons · 2005",               "pop",       avatarE),
    //                     row("BU", "Burial",          "Untrue · 2007",                       "garage",    avatarA),
    //                     row("BH", "Beach House",     "Teen Dream · 2010",                   "dream pop", avatarB)
    //                 ),
    //                 section("C", "2 records",
    //                     row("CT", "Cocteau Twins",   "Heaven or Las Vegas · 1990",          "dream pop", avatarC),
    //                     row("CA", "Caribou",         "Swim · 2010",                         "electronic", avatarD)
    //                 ),
    //                 section("D", "3 records",
    //                     row("DP", "Daft Punk",       "Discovery · 2001",                    "house",     avatarE),
    //                     row("DS", "DJ Shadow",       "Endtroducing..... · 1996",            "trip hop",  avatarA),
    //                     row("DH", "Deerhunter",      "Halcyon Digest · 2010",               "indie",     avatarB)
    //                 ),
    //                 section("M", "2 records",
    //                     row("MB", "My Bloody Valentine", "Loveless · 1991",                 "shoegaze",  avatarD),
    //                     row("MA", "Massive Attack",  "Mezzanine · 1998",                    "trip hop",  avatarC)
    //                 ),
    //                 section("S", "2 records",
    //                     row("SL", "Slowdive",        "Souvlaki · 1993",                     "shoegaze",  avatarE),
    //                     row("ST", "Stereolab",       "Dots and Loops · 1997",               "post-rock", avatarA)
    //                 )
    //             )
    //         ),

    //         // Now playing
    //         div()
    //             .width(S::px(300))
    //             .flexShrink(S::px(0))
    //             .color(panel)
    //             .borderColor(outline)
    //             .borderWidth(S::px(1))
    //             .cornerRadius(S::px(14))
    //             .padding(S::px(16))
    //             .display(Display::Flex)
    //             .flexDirection(FlexDirection::Col)
    //             .flexGap(S::px(14))
    //         (
    //             div()
    //                 .position(gui::Position::Relative)
    //                 .width(S::percent(1.0))
    //                 .height(S::autoSize())
    //                 .aspectRatio(1, 1)
    //             (
    //                 image(coverPath, S::percent(1.0), S::autoSize())
    //                     .aspectRatio(1, 1)
    //                     .cornerRadius(S::px(10)),
    //                 div()
    //                     .position(gui::Position::Absolute)
    //                     .top(S::px(10))
    //                     .right(S::px(10))
    //                     .color(coral)
    //                     .height(S::px(22))
    //                     .paddingLeft(S::px(9))
    //                     .paddingRight(S::px(9))
    //                     .cornerRadius(S::px(11))
    //                     .display(Display::Flex)
    //                     .alignItems(AlignItems::Center)
    //                 (
    //                     text("NOW PLAYING").font(DINAlternateBold).fontSize(S::pt(10)).color(bg)
    //                 )
    //             ),
    //             div()
    //                 .display(Display::Flex)
    //                 .flexDirection(FlexDirection::Col)
    //                 .flexGap(S::px(3))
    //             (
    //                 text("Loveless").font(NewYork).fontSize(S::pt(22)).color(ink),
    //                 text("My Bloody Valentine · 1991").font(SF).fontSize(S::pt(12)).color(muted)
    //             ),
    //             div()
    //                 .display(Display::Flex)
    //                 .flexWrap(gui::FlexWrap::Wrap)
    //                 .flexGap(S::px(6))
    //             (
    //                 chip("shoegaze", cyan, raised),
    //                 chip("creation records", cyan, raised),
    //                 chip("remastered", cyan, raised),
    //                 chip("vinyl", cyan, raised),
    //                 chip("11 tracks", cyan, raised)
    //             ),
    //             div()
    //                 .display(Display::Flex)
    //                 .flexDirection(FlexDirection::Col)
    //                 .flexGap(S::px(6))
    //             (
    //                 div()
    //                     .display(Display::Flex)
    //                     .justifyContent(JustifyContent::SpaceBetween)
    //                 (
    //                     text("Only Shallow").font(ArialBold).fontSize(S::pt(12)).color(ink),
    //                     text("01:58 / 04:17").font(SFMono).fontSize(S::pt(10)).color(muted)
    //                 ),
    //                 div()
    //                     .width(S::percent(1.0))
    //                     .height(S::px(6))
    //                     .color(raised)
    //                     .cornerRadius(S::px(3))
    //                 (
    //                     div()
    //                         .width(S::percent(0.46))
    //                         .height(S::percent(1.0))
    //                         .color(coral)
    //                         .cornerRadius(S::px(3))
    //                     ()
    //                 )
    //             ),
    //             div()
    //                 .display(Display::Flex)
    //                 .flexGap(S::px(8))
    //             (
    //                 button("Play", coral, avatarA, bg),
    //                 button("Queue", raised, hover, ink)
    //             ),
    //             div().height(S::px(1)).color(outline)(),
    //             div()
    //                 .display(Display::Flex)
    //                 .flexDirection(FlexDirection::Col)
    //                 .flexGap(S::px(8))
    //             (
    //                 stat("Pressing", "1991 · UK first"),
    //                 stat("Condition", "VG+ / VG+"),
    //                 stat("Last played", "3 days ago"),
    //                 stat("Plays", "27")
    //             )
    //         )
    //     )
    // );

    // // Box shadows: a light desk with objects at different elevations.
    // // Exercises outer blur/spread/offset, hard-edged (blur 0) shadows, inset shadows
    // // on divs and images, spread against elliptical per-corner radii, percent-based
    // // offsets, and hover/press state driving shadow changes at finalize only.
    // using S = gui::Size;
    // using gui::Display;
    // using gui::FlexDirection;
    // using gui::AlignItems;
    // using gui::JustifyContent;
    // using runtime::EventType;

    // constexpr simd_float4 desk        {0.941, 0.937, 0.925, 1.0};
    // constexpr simd_float4 paper       {1.000, 1.000, 1.000, 1.0};
    // constexpr simd_float4 cream       {0.992, 0.973, 0.925, 1.0};
    // constexpr simd_float4 ink         {0.129, 0.129, 0.153, 1.0};
    // constexpr simd_float4 muted       {0.478, 0.478, 0.510, 1.0};
    // constexpr simd_float4 rule        {0.871, 0.863, 0.839, 1.0};
    // constexpr simd_float4 tangerine   {1.000, 0.502, 0.200, 1.0};
    // constexpr simd_float4 grape       {0.435, 0.271, 0.831, 1.0};
    // constexpr simd_float4 mint        {0.180, 0.741, 0.545, 1.0};
    // constexpr simd_float4 slate       {0.239, 0.271, 0.325, 1.0};

    // constexpr simd_float4 umbraSoft   {0.0, 0.0, 0.0, 0.16};
    // constexpr simd_float4 umbraHard   {0.0, 0.0, 0.0, 1.00};
    // constexpr simd_float4 umbraLift   {0.0, 0.0, 0.0, 0.26};
    // constexpr simd_float4 umbraWell   {0.0, 0.0, 0.0, 0.32};
    // constexpr simd_float4 grapeGlow   {0.435, 0.271, 0.831, 0.55};
    // constexpr simd_float4 mintGlow    {0.180, 0.741, 0.545, 0.60};

    // constexpr auto butterflyPath = "/Users/treja/projects/gui/assets/butterfly.png";
    // constexpr auto coverPath = "/Users/treja/projects/gui/assets/loveless.jpg";

    // auto caption = [&](const char* title, const char* spec) {
    //     return div()
    //         .display(Display::Flex)
    //         .flexDirection(FlexDirection::Col)
    //         .flexGap(S::px(3))
    //     (
    //         text(title).font(ArialBold).fontSize(S::pt(12)).color(ink),
    //         text(spec).font(SFMono).fontSize(S::pt(9)).color(muted)
    //     );
    // };

    // auto cell = [&](auto&& subject, const char* title, const char* spec) {
    //     return div()
    //         .width(S::px(220))
    //         .display(Display::Flex)
    //         .flexDirection(FlexDirection::Col)
    //         .alignItems(AlignItems::Center)
    //         .flexGap(S::px(18))
    //         .paddingTop(S::px(24))
    //         .paddingBottom(S::px(12))
    //     (
    //         subject,
    //         caption(title, spec)
    //     );
    // };

    // div(S::percent(1.0), S::percent(1.0), desk)
    //     .padding(S::px(36))
    //     .overflow(gui::Overflow::Scroll)
    // (
    //     div()
    //         .display(Display::Flex)
    //         .flexDirection(FlexDirection::Col)
    //         .flexGap(S::px(8))
    //         .marginBottom(S::px(28))
    //     (
    //         text("ELEVATION").font(DINAlternateBold).fontSize(S::pt(26)).color(ink),
    //         text("box-shadow on divs and images · hover the button, click to press it")
    //             .font(SFMono).fontSize(S::pt(11)).color(muted)
    //     ),
    //     div().height(S::px(1)).color(rule)(),

    //     div()
    //         .display(Display::Flex)
    //         .flexWrap(gui::FlexWrap::Wrap)
    //         .flexGap(S::px(12))
    //         .justifyContent(JustifyContent::Center)
    //     (
    //         // 1. Soft ambient lift; the bread-and-butter card shadow.
    //         cell(
    //             div(S::px(140), S::px(100), paper)
    //                 .cornerRadius(S::px(12))
    //                 .shadowOffsetY(S::px(8))
    //                 .shadowBlur(S::px(24))
    //                 .shadowColor(umbraSoft)
    //             (),
    //             "Soft card", "0 8 24 0 · rgba(0,0,0,.16)"
    //         ),

    //         // 2. Hard offset; blur 0 goes through the sharp-edge AA path.
    //         cell(
    //             div(S::px(140), S::px(100), cream)
    //                 .borderWidth(S::px(2))
    //                 .borderColor(ink)
    //                 .shadowOffsetX(S::px(8))
    //                 .shadowOffsetY(S::px(8))
    //                 .shadowColor(umbraHard)
    //             (),
    //             "Brutalist", "8 8 0 0 · #000"
    //         ),

    //         // 3. Negative spread pulls the shadow in under the box; only the blur leaks out.
    //         cell(
    //             div(S::px(140), S::px(100), paper)
    //                 .cornerRadius(S::px(50))
    //                 .shadowOffsetY(S::px(14))
    //                 .shadowBlur(S::px(20))
    //                 .shadowSpread(S::px(-10))
    //                 .shadowColor(umbraLift)
    //             (),
    //             "Pill, negative spread", "0 14 20 -10"
    //         ),

    //         // 4. Colored glow; spread without offset.
    //         cell(
    //             div(S::px(140), S::px(100), grape)
    //                 .cornerRadius(S::px(16))
    //                 .shadowBlur(S::px(32))
    //                 .shadowSpread(S::px(2))
    //                 .shadowColor(grapeGlow)
    //             (),
    //             "Glow", "0 0 32 2 · grape 55%"
    //         ),

    //         // 5. Inset well on a div; shadow sits above the fill, under the border.
    //         cell(
    //             div(S::px(140), S::px(100), cream)
    //                 .cornerRadius(S::px(10))
    //                 .borderWidth(S::px(1))
    //                 .borderColor(rule)
    //                 .shadowInset(true)
    //                 .shadowOffsetY(S::px(4))
    //                 .shadowBlur(S::px(10))
    //                 .shadowColor(umbraWell)
    //             (),
    //             "Inset well", "inset 0 4 10 0"
    //         ),

    //         // 6. Mismatched elliptical corners with positive spread; exercises the
    //         //    nonlinear radius growth where radius < spread.
    //         cell(
    //             div(S::px(140), S::px(100), tangerine)
    //                 .cornerRadiusTopLeft(S::px(48))
    //                 .cornerRadiusTopRight(S::px(4))
    //                 .cornerRadiusBottomRight(S::px(30))
    //                 .cornerRadiusBottomLeft(S::px(0))
    //                 .shadowOffsetY(S::px(6))
    //                 .shadowBlur(S::px(10))
    //                 .shadowSpread(S::px(12))
    //                 .shadowColor(umbraSoft)
    //             (),
    //             "Uneven corners + spread", "0 6 10 12"
    //         ),

    //         // 7. Percent offsets resolve against the box.
    //         cell(
    //             div(S::px(140), S::px(100), slate)
    //                 .cornerRadius(S::px(6))
    //                 .shadowOffsetX(S::percent(0.1))
    //                 .shadowOffsetY(S::percent(0.1))
    //                 .shadowBlur(S::px(2))
    //                 .shadowColor(mintGlow)
    //             (),
    //             "Percent offset", "10% 10% 2 0"
    //         ),

    //         // 8. Image with a drop shadow; the quad grows but UVs stay on the original rect.
    //         cell(
    //             image(butterflyPath, S::px(140), S::px(100))
    //                 .cornerRadius(S::px(12))
    //                 .shadowOffsetY(S::px(10))
    //                 .shadowBlur(S::px(22))
    //                 .shadowColor(umbraLift),
    //             "Image drop", "0 10 22 0"
    //         ),

    //         // 9. Image with an inset vignette over the texture.
    //         cell(
    //             image(coverPath, S::px(140), S::px(100))
    //                 .cornerRadius(S::px(12))
    //                 .borderWidth(S::px(2))
    //                 .borderColor(paper)
    //                 .shadowInset(true)
    //                 .shadowBlur(S::px(28))
    //                 .shadowSpread(S::px(4))
    //                 .shadowColor(umbraWell),
    //             "Image vignette", "inset 0 0 28 4"
    //         ),

    //         // 10. Interactive: hover lifts, click presses into an inset shadow.
    //         cell(
    //             div(S::px(140), S::px(100), mint)
    //                 .cornerRadius(S::px(14))
    //                 .display(Display::Flex)
    //                 .alignItems(AlignItems::Center)
    //                 .justifyContent(JustifyContent::Center)
    //                 .shadowOffsetY(S::px(4))
    //                 .shadowBlur(S::px(10))
    //                 .shadowColor(umbraSoft)
    //                 .addEventListener(EventType::MouseEnter, [](auto& node, Event&) {
    //                     if (node.shadowInset()) {
    //                         return;
    //                     }

    //                     node.shadowOffsetY(S::px(12))
    //                         .shadowBlur(S::px(28))
    //                         .shadowColor(umbraLift);
    //                 })
    //                 .addEventListener(EventType::MouseLeave, [](auto& node, Event&) {
    //                     if (node.shadowInset()) {
    //                         return;
    //                     }

    //                     node.shadowOffsetY(S::px(4))
    //                         .shadowBlur(S::px(10))
    //                         .shadowColor(umbraSoft);
    //                 })
    //                 .addEventListener(EventType::Click, [](auto& node, Event&) {
    //                     bool pressed = !node.shadowInset();
    //                     node.shadowInset(pressed);

    //                     if (pressed) {
    //                         node.shadowOffsetY(S::px(3))
    //                             .shadowBlur(S::px(8))
    //                             .shadowColor(umbraWell);
    //                     } else {
    //                         node.shadowOffsetY(S::px(12))
    //                             .shadowBlur(S::px(28))
    //                             .shadowColor(umbraLift);
    //                     }
    //                 })
    //             (
    //                 text("press me").font(ArialBold).fontSize(S::pt(12)).color(paper)
    //             ),
    //             "Interactive", "hover: lift · click: inset"
    //         )
    //     )
    // );

    // // Transforms: rendering, composition, origin, hit testing, clipping,
    // // containing blocks (none vs identity), and scrollable overflow.
    // {
    //     using S = gui::Size;
    //     using gui::Display;
    //     using gui::FlexDirection;
    //     using gui::AlignItems;
    //     using gui::JustifyContent;
    //     using gui::Position;
    //     using gui::Overflow;
    //     using runtime::EventType;

    //     constexpr simd_float4 desk      {0.941, 0.937, 0.925, 1.0};
    //     constexpr simd_float4 paper     {1.000, 1.000, 1.000, 1.0};
    //     constexpr simd_float4 cream     {0.992, 0.973, 0.925, 1.0};
    //     constexpr simd_float4 ink       {0.129, 0.129, 0.153, 1.0};
    //     constexpr simd_float4 muted     {0.478, 0.478, 0.510, 1.0};
    //     constexpr simd_float4 rule      {0.871, 0.863, 0.839, 1.0};
    //     constexpr simd_float4 grape     {0.435, 0.271, 0.831, 1.0};
    //     constexpr simd_float4 mint      {0.180, 0.741, 0.545, 1.0};
    //     constexpr simd_float4 tangerine {1.000, 0.502, 0.200, 1.0};
    //     constexpr simd_float4 slate     {0.239, 0.271, 0.325, 1.0};
    //     constexpr simd_float4 ghost     {0.129, 0.129, 0.153, 0.08};

    //     auto label = [&](const char* title, const char* spec) {
    //         return div()
    //             .display(Display::Flex)
    //             .flexDirection(FlexDirection::Col)
    //             .flexGap(S::px(3))
    //         (
    //             text(title).font(ArialBold).fontSize(S::pt(12)).color(ink),
    //             text(spec).font(SFMono).fontSize(S::pt(9)).color(muted)
    //         );
    //     };

    //     auto cell = [&](const char* title, const char* spec, auto&& subject) {
    //         return div()
    //             .width(S::px(220))
    //             .display(Display::Flex)
    //             .flexDirection(FlexDirection::Col)
    //             .alignItems(AlignItems::Center)
    //             .flexGap(S::px(14))
    //             .paddingTop(S::px(20))
    //             .paddingBottom(S::px(12))
    //         (
    //             subject,
    //             label(title, spec)
    //         );
    //     };

    //     auto section = [&](const char* title) {
    //         return div()
    //             .display(Display::Flex)
    //             .flexDirection(FlexDirection::Col)
    //             .flexGap(S::px(6))
    //             .marginTop(S::px(36))
    //             .marginBottom(S::px(8))
    //         (
    //             text(title).font(DINAlternateBold).fontSize(S::pt(18)).color(ink),
    //             div().height(S::px(1)).color(rule)()
    //         );
    //     };

    //     auto row = [&](auto&&... cells) {
    //         return div()
    //             .display(Display::Flex)
    //             .flexWrap(gui::FlexWrap::Wrap)
    //             .flexGap(S::px(12))
    //             .justifyContent(JustifyContent::Center)
    //         (cells...);
    //     };

    //     // Untransformed footprint drawn under a transformed subject so the offset is visible.
    //     auto ghosted = [&](auto&& subject) {
    //         return div(S::px(140), S::px(100), ghost)
    //             .cornerRadius(S::px(12))
    //             .position(Position::Relative)
    //         (
    //             subject.position(Position::Absolute).top(S::px(0)).left(S::px(0))
    //         );
    //     };

    //     auto card = [&](simd_float4 color) {
    //         return div(S::px(140), S::px(100), color)
    //             .cornerRadius(S::px(12))
    //             .borderWidth(S::px(2))
    //             .borderColor(ink);
    //     };

    //     auto probe = [&](Position position, simd_float4 color = grape) {
    //         return div(S::px(120), S::px(36), color)
    //             .position(position)
    //             .top(S::px(420))
    //             .left(S::px(20))
    //             .cornerRadius(S::px(6))
    //             .display(Display::Flex)
    //             .alignItems(AlignItems::Center)
    //             .paddingLeft(S::px(10))
    //         (
    //             text("probe").font(ArialBold).fontSize(S::pt(11)).color(paper)
    //         );
    //     };

    //     auto filler = [&]() {
    //         return div(S::px(160), S::px(90), cream)
    //             .cornerRadius(S::px(6))
    //             .padding(S::px(10))
    //         (
    //             text("in-flow").font(SFMono).fontSize(S::pt(10)).color(muted)
    //         );
    //     };

    //     auto scroller = [&]() {
    //         return div(S::px(200), S::px(160), paper)
    //             .overflow(Overflow::Scroll)
    //             .padding(S::px(12))
    //             .borderWidth(S::px(1))
    //             .borderColor(rule)
    //             .cornerRadius(S::px(8));
    //     };

    //     div(S::percent(1.0), S::percent(1.0), desk)
    //         .padding(S::px(36))
    //         .overflow(Overflow::Scroll)
    //     (
    //         div()
    //             .display(Display::Flex)
    //             .flexDirection(FlexDirection::Col)
    //             .flexGap(S::px(8))
    //         (
    //             text("TRANSFORMS").font(DINAlternateBold).fontSize(S::pt(26)).color(ink),
    //             text("ghost = layout box · transforms never move layout")
    //                 .font(SFMono).fontSize(S::pt(11)).color(muted)
    //         ),

    //         section("Rendering • origin is box center • order is T•R•S"),
    //         row(
    //             cell("translate", "(24, 12)",
    //                 ghosted(card(mint).translate(simd_float2{24.0f, 12.0f})())
    //             ),
    //             cell("rotate", "0.35 rad · cw",
    //                 ghosted(card(grape).rotate(0.35f)())
    //             ),
    //             cell("scale", "(1.3, 0.7)",
    //                 ghosted(card(tangerine).scale(simd_float2{1.3f, 0.7f})())
    //             ),
    //             cell("all three", "T(20,0) R(0.25) S(0.8)",
    //                 ghosted(card(slate).translate(simd_float2{20.0f, 0.0f}).rotate(0.25f).scale(simd_float2{0.8f, 0.8f})())
    //             ),
    //             cell("nested", "parent R(0.3) · child S(0.6)",
    //                 ghosted(
    //                     card(mint).rotate(0.3f)
    //                         .display(Display::Flex)
    //                         .alignItems(AlignItems::Center)
    //                         .justifyContent(JustifyContent::Center)
    //                     (
    //                         div(S::px(100), S::px(60), grape)
    //                             .cornerRadius(S::px(8))
    //                             .scale(simd_float2{0.6f, 0.6f})
    //                         ()
    //                     )
    //                 )
    //             ),
    //             cell("border + radius under rotate", "SDF stays crisp",
    //                 ghosted(
    //                     card(paper).rotate(0.6f).borderWidth(S::px(6)).cornerRadius(S::px(40))()
    //                 )
    //             )
    //         ),

    //         section("Hit testing · handlers fire on the transformed shape, not the ghost"),
    //         row(
    //             cell("click toggles", "rotated 0.5",
    //                 ghosted(
    //                     card(mint).rotate(0.5f)
    //                         .addEventListener(EventType::Click, [](auto& node, Event&) {
    //                             node.color(node.color().y > 0.5f ? grape : mint);
    //                         })
    //                     ()
    //                 )
    //             ),
    //             cell("hover scales", "1.0 ↔ 1.2 · must not compound",
    //                 ghosted(
    //                     card(tangerine)
    //                         .addEventListener(EventType::MouseEnter, [](auto& node, Event&) {
    //                             node.scale(simd_float2{1.2f, 1.2f});
    //                         })
    //                         .addEventListener(EventType::MouseLeave, [](auto& node, Event&) {
    //                             node.scale(simd_float2{1.0f, 1.0f});
    //                         })
    //                     ()
    //                 )
    //             ),
    //             cell("click inside rotated parent", "child inherits parent transform",
    //                 ghosted(
    //                     card(slate).rotate(0.4f)
    //                         .display(Display::Flex)
    //                         .alignItems(AlignItems::Center)
    //                         .justifyContent(JustifyContent::Center)
    //                     (
    //                         div(S::px(70), S::px(40), grape)
    //                             .cornerRadius(S::px(8))
    //                             .addEventListener(EventType::Click, [](auto& node, Event&) {
    //                                 node.color(node.color().x > 0.5f ? grape : tangerine);
    //                             })
    //                         ()
    //                     )
    //                 )
    //             )
    //         ),

    //         section("Clipping · overflow: hidden under transforms"),
    //         row(
    //             cell("rotated clipper", "child clipped by rotated rect",
    //                 ghosted(
    //                     card(paper).rotate(0.4f).overflow(Overflow::Hidden)
    //                     (
    //                         div(S::px(240), S::px(30), grape).marginTop(S::px(35)).marginLeft(S::px(-50))()
    //                     )
    //                 )
    //             ),
    //             cell("rotated child", "clipped by axis-aligned parent",
    //                 ghosted(
    //                     card(paper).overflow(Overflow::Hidden)
    //                     (
    //                         div(S::px(120), S::px(120), mint).cornerRadius(S::px(8)).rotate(0.7f)()
    //                     )
    //                 )
    //             ),
    //             cell("nested clippers", "both rects apply",
    //                 ghosted(
    //                     card(paper).rotate(0.3f).overflow(Overflow::Hidden)
    //                     (
    //                         div(S::px(140), S::px(100), cream).overflow(Overflow::Hidden).rotate(-0.5f)
    //                         (
    //                             div(S::px(200), S::px(200), tangerine).marginTop(S::px(-40)).marginLeft(S::px(-30))()
    //                         )
    //                     )
    //                 )
    //             )
    //         ),

    //         section("Containing blocks · none vs identity"),
    //         row(
    //             cell("abs in static", "no CB → escapes to root",
    //                 card(paper).overflow(Overflow::Hidden)
    //                 (
    //                     div(S::px(40), S::px(40), grape).position(Position::Absolute).top(S::px(10)).left(S::px(10))()
    //                 )
    //             ),
    //             cell("abs in scale(1)", "identity CB → stays in card",
    //                 card(paper).overflow(Overflow::Hidden).scale(simd_float2{1.0f, 1.0f})
    //                 (
    //                     div(S::px(40), S::px(40), grape).position(Position::Absolute).top(S::px(10)).left(S::px(10))()
    //                 )
    //             ),
    //             cell("fixed in scale(1)", "identity CB → stays in card",
    //                 card(paper).overflow(Overflow::Hidden).scale(simd_float2{1.0f, 1.0f})
    //                 (
    //                     div(S::px(40), S::px(40), tangerine).position(Position::Fixed).top(S::px(10)).left(S::px(10))()
    //                 )
    //             ),
    //             cell("abs in rotated", "rotated CB → probe rides along",
    //                 ghosted(
    //                     card(mint).rotate(0.35f)
    //                     (
    //                         div(S::px(50), S::px(24), grape)
    //                             .position(Position::Absolute).top(S::px(30)).left(S::px(80))
    //                             .cornerRadius(S::px(4))
    //                         ()
    //                     )
    //                 )
    //             )
    //         ),

    //         section("Scrollable overflow · scrolls only if the probe counts"),
    //         row(
    //             cell("abs, static scroller", "no CB in scroller → no scroll",
    //                 scroller()(filler(), probe(Position::Absolute))
    //             ),
    //             cell("abs, relative scroller", "CB is scroller → scrolls",
    //                 scroller().position(Position::Relative)(filler(), probe(Position::Absolute))
    //             ),
    //             cell("abs, relative wrapper", "CB inside scroller → scrolls",
    //                 scroller()(filler(), div().position(Position::Relative)(probe(Position::Absolute)))
    //             ),
    //             cell("abs inside escaped abs", "wrapper escapes → no scroll",
    //                 scroller()(
    //                     filler(),
    //                     div().position(Position::Absolute).top(S::px(0)).left(S::px(0))(probe(Position::Absolute))
    //                 )
    //             ),
    //             cell("fixed, static scroller", "viewport CB → no scroll",
    //                 scroller()(filler(), probe(Position::Fixed, tangerine))
    //             ),
    //             cell("fixed, scale(1) scroller", "identity CB → scrolls",
    //                 scroller().scale(simd_float2{1.0f, 1.0f})(filler(), probe(Position::Fixed, tangerine))
    //             ),
    //             cell("translated in-flow", "transformed box → scrolls",
    //                 scroller()(filler(), div(S::px(120), S::px(36), mint).cornerRadius(S::px(6)).translate(simd_float2{0.0f, 300.0f})())
    //             ),
    //             cell("rotated in-flow", "AABB of rotated box → scrolls",
    //                 scroller()(filler(), div(S::px(30), S::px(260), mint).cornerRadius(S::px(6)).rotate(1.2f)())
    //             )
    //         )
    //     );

    // }

    // // Border styles: solid, dashed, dotted, double across widths, radii
    // // (circular, elliptical, uneven, sharp), on divs and images, with shadows,
    // // and a click that cycles the style at finalize only.
    // {
    //     using S = gui::Size;
    //     using gui::Display;
    //     using gui::FlexDirection;
    //     using gui::AlignItems;
    //     using gui::JustifyContent;
    //     using gui::BorderStyle;
    //     using runtime::EventType;

    //     constexpr simd_float4 desk        {0.941, 0.937, 0.925, 1.0};
    //     constexpr simd_float4 paper       {1.000, 1.000, 1.000, 1.0};
    //     constexpr simd_float4 cream       {0.992, 0.973, 0.925, 1.0};
    //     constexpr simd_float4 ink         {0.129, 0.129, 0.153, 1.0};
    //     constexpr simd_float4 muted       {0.478, 0.478, 0.510, 1.0};
    //     constexpr simd_float4 rule        {0.871, 0.863, 0.839, 1.0};
    //     constexpr simd_float4 tangerine   {1.000, 0.502, 0.200, 1.0};
    //     constexpr simd_float4 grape       {0.435, 0.271, 0.831, 1.0};
    //     constexpr simd_float4 mint        {0.180, 0.741, 0.545, 1.0};
    //     constexpr simd_float4 slate       {0.239, 0.271, 0.325, 1.0};
    //     constexpr simd_float4 umbraSoft   {0.0, 0.0, 0.0, 0.16};

    //     constexpr auto butterflyPath = "/Users/treja/projects/gui/assets/butterfly.png";

    //     auto caption = [&](const char* title, const char* spec) {
    //         return div()
    //             .display(Display::Flex)
    //             .flexDirection(FlexDirection::Col)
    //             .flexGap(S::px(3))
    //         (
    //             text(title).font(ArialBold).fontSize(S::pt(12)).color(ink),
    //             text(spec).font(SFMono).fontSize(S::pt(9)).color(muted)
    //         );
    //     };

    //     auto cell = [&](auto&& subject, const char* title, const char* spec) {
    //         return div()
    //             .width(S::px(220))
    //             .display(Display::Flex)
    //             .flexDirection(FlexDirection::Col)
    //             .alignItems(AlignItems::Center)
    //             .flexGap(S::px(18))
    //             .paddingTop(S::px(24))
    //             .paddingBottom(S::px(12))
    //         (
    //             subject,
    //             caption(title, spec)
    //         );
    //     };

    //     auto swatch = [&](simd_float4 fill, float width, BorderStyle style, simd_float4 stroke = ink) {
    //         return div(S::px(140), S::px(100), fill)
    //             .borderWidth(S::px(width))
    //             .borderStyle(style)
    //             .borderColor(stroke);
    //     };

    //     div(S::percent(1.0), S::percent(1.0), desk)
    //         .padding(S::px(36))
    //         .overflow(gui::Overflow::Scroll)
    //     (
    //         div()
    //             .display(Display::Flex)
    //             .flexDirection(FlexDirection::Col)
    //             .flexGap(S::px(8))
    //             .marginBottom(S::px(28))
    //         (
    //             text("STROKES").font(DINAlternateBold).fontSize(S::pt(26)).color(ink),
    //             text("border-style on divs and images · click the last card to cycle its style")
    //                 .font(SFMono).fontSize(S::pt(11)).color(muted)
    //         ),
    //         div().height(S::px(1)).color(rule)(),

    //         div()
    //             .display(Display::Flex)
    //             .flexWrap(gui::FlexWrap::Wrap)
    //             .flexGap(S::px(12))
    //             .justifyContent(JustifyContent::Center)
    //         (
    //             cell(swatch(paper, 1, BorderStyle::Solid).cornerRadius(S::px(12))(),
    //                 "Solid hairline", "1 solid · r12"),
    //             cell(swatch(cream, 6, BorderStyle::Solid)(),
    //                 "Solid thick", "6 solid · sharp"),
    //             cell(swatch(paper, 1, BorderStyle::Dashed)(),
    //                 "Dashed hairline", "1 dashed · sharp"),
    //             cell(swatch(paper, 3, BorderStyle::Dashed).cornerRadius(S::px(12))(),
    //                 "Dashed", "3 dashed · r12"),
    //             cell(swatch(cream, 6, BorderStyle::Dashed, grape).cornerRadius(S::px(50))(),
    //                 "Dashed pill", "6 dashed · r50"),
    //             cell(swatch(paper, 3, BorderStyle::Dashed, tangerine).cornerRadius(S::percent(0.5))(),
    //                 "Dashed ellipse", "3 dashed · r50%"),
    //             cell(swatch(paper, 4, BorderStyle::Dashed, slate)
    //                     .cornerRadiusTopLeft(S::px(48))
    //                     .cornerRadiusTopRight(S::px(4))
    //                     .cornerRadiusBottomRight(S::px(30))
    //                     .cornerRadiusBottomLeft(S::px(0))(),
    //                 "Dashed uneven", "4 dashed · 48 4 30 0"),
    //             cell(swatch(paper, 2, BorderStyle::Dotted).cornerRadius(S::px(8))(),
    //                 "Dotted", "2 dotted · r8"),
    //             cell(swatch(cream, 6, BorderStyle::Dotted, mint).cornerRadius(S::px(20))(),
    //                 "Dotted thick", "6 dotted · r20"),
    //             cell(swatch(paper, 5, BorderStyle::Dotted, grape).cornerRadius(S::percent(0.5))(),
    //                 "Dotted ellipse", "5 dotted · r50%"),
    //             cell(swatch(paper, 6, BorderStyle::Double).cornerRadius(S::px(10))(),
    //                 "Double", "6 double · r10"),
    //             cell(swatch(cream, 3, BorderStyle::Double, slate)(),
    //                 "Double thin", "3 double · sharp"),
    //             cell(swatch(paper, 3, BorderStyle::Dashed)
    //                     .cornerRadius(S::px(12))
    //                     .shadowOffsetY(S::px(8))
    //                     .shadowBlur(S::px(24))
    //                     .shadowColor(umbraSoft)(),
    //                 "Dashed + shadow", "3 dashed · 0 8 24"),
    //             cell(image(butterflyPath, S::px(140), S::px(100))
    //                     .cornerRadius(S::px(12))
    //                     .borderWidth(S::px(4))
    //                     .borderStyle(BorderStyle::Dotted)
    //                     .borderColor(paper),
    //                 "Image dotted", "4 dotted · r12"),
    //             cell(image(butterflyPath, S::px(140), S::px(100))
    //                     .cornerRadius(S::px(12))
    //                     .borderWidth(S::px(3))
    //                     .borderStyle(BorderStyle::Dashed)
    //                     .borderColor(ink),
    //                 "Image dashed", "3 dashed · r12"),
    //             cell(swatch(mint, 4, BorderStyle::Solid, ink)
    //                     .cornerRadius(S::px(14))
    //                     .display(Display::Flex)
    //                     .alignItems(AlignItems::Center)
    //                     .justifyContent(JustifyContent::Center)
    //                     .addEventListener(EventType::Click, [](auto& node, Event&) {
    //                         switch (node.borderStyle()) {
    //                             case BorderStyle::Solid:  node.borderStyle(BorderStyle::Dashed); break;
    //                             case BorderStyle::Dashed: node.borderStyle(BorderStyle::Dotted); break;
    //                             case BorderStyle::Dotted: node.borderStyle(BorderStyle::Double); break;
    //                             case BorderStyle::Double: node.borderStyle(BorderStyle::Solid); break;
    //                         }
    //                     })
    //                 (
    //                     text("click me").font(ArialBold).fontSize(S::pt(12)).color(paper)
    //                 ),
    //                 "Interactive", "click: solid → dashed → dotted → double")
    //         )
    //     );
    // }

//     // Opacity: multiplies down the tree, applies to fill/border/shadow/text/image,
//     // composes with color alpha, and is per-primitive (no group compositing).
//     // Divergence from browsers: overlapping content under one opacity double-blends.
//     {
//         using S = gui::Size;
//         using gui::Display;
//         using gui::FlexDirection;
//         using gui::AlignItems;
//         using gui::JustifyContent;
//         using gui::Position;
//         using runtime::EventType;
// 
//         constexpr simd_float4 desk      {0.941, 0.937, 0.925, 1.0};
//         constexpr simd_float4 paper     {1.000, 1.000, 1.000, 1.0};
//         constexpr simd_float4 ink       {0.129, 0.129, 0.153, 1.0};
//         constexpr simd_float4 muted     {0.478, 0.478, 0.510, 1.0};
//         constexpr simd_float4 rule      {0.871, 0.863, 0.839, 1.0};
//         constexpr simd_float4 grape     {0.435, 0.271, 0.831, 1.0};
//         constexpr simd_float4 mint      {0.180, 0.741, 0.545, 1.0};
//         constexpr simd_float4 tangerine {1.000, 0.502, 0.200, 1.0};
//         constexpr simd_float4 slate     {0.239, 0.271, 0.325, 1.0};
//         constexpr simd_float4 umbra     {0.000, 0.000, 0.000, 0.5};
// 
//         constexpr auto butterflyPath = "/Users/treja/projects/gui/assets/butterfly.png";
// 
//         auto label = [&](const char* title, const char* spec) {
//             return div()
//                 .display(Display::Flex)
//                 .flexDirection(FlexDirection::Col)
//                 .flexGap(S::px(3))
//             (
//                 text(title).font(ArialBold).fontSize(S::pt(12)).color(ink),
//                 text(spec).font(SFMono).fontSize(S::pt(9)).color(muted)
//             );
//         };
// 
//         auto cell = [&](const char* title, const char* spec, auto&& subject) {
//             return div()
//                 .width(S::px(220))
//                 .display(Display::Flex)
//                 .flexDirection(FlexDirection::Col)
//                 .alignItems(AlignItems::Center)
//                 .flexGap(S::px(14))
//                 .paddingTop(S::px(20))
//                 .paddingBottom(S::px(12))
//             (
//                 subject,
//                 label(title, spec)
//             );
//         };
// 
//         auto section = [&](const char* title) {
//             return div()
//                 .display(Display::Flex)
//                 .flexDirection(FlexDirection::Col)
//                 .flexGap(S::px(6))
//                 .marginTop(S::px(36))
//                 .marginBottom(S::px(8))
//             (
//                 text(title).font(DINAlternateBold).fontSize(S::pt(18)).color(ink),
//                 div().height(S::px(1)).color(rule)()
//             );
//         };
// 
//         auto row = [&](auto&&... cells) {
//             return div()
//                 .display(Display::Flex)
//                 .flexWrap(gui::FlexWrap::Wrap)
//                 .flexGap(S::px(12))
//                 .justifyContent(JustifyContent::Center)
//             (cells...);
//         };
// 
//         auto card = [&](simd_float4 color) {
//             return div(S::px(140), S::px(100), color)
//                 .cornerRadius(S::px(12))
//                 .borderWidth(S::px(2))
//                 .borderColor(ink);
//         };
// 
//         // Striped backdrop so partial alpha is visible against something other than the desk.
//         auto backdrop = [&](auto&& subject) {
//             return div().width(S::px(160)).height(S::px(120))
//                 .display(Display::Flex)
//                 .alignItems(AlignItems::Center)
//                 .justifyContent(JustifyContent::Center)
//                 .position(Position::Relative)
//             (
//                 div(S::px(160), S::px(40), ink).position(Position::Absolute).top(S::px(40)).left(S::px(0))(),
//                 subject
//             );
//         };
// 
//         auto centered = [&](auto&& builder) {
//             return builder
//                 .display(Display::Flex)
//                 .alignItems(AlignItems::Center)
//                 .justifyContent(JustifyContent::Center);
//         };
// 
//         div(S::percent(1.0), S::percent(1.0), desk)
//             .padding(S::px(36))
//             .overflow(gui::Overflow::Scroll)
//         (
//             div()
//                 .display(Display::Flex)
//                 .flexDirection(FlexDirection::Col)
//                 .flexGap(S::px(8))
//             (
//                 text("OPACITY").font(DINAlternateBold).fontSize(S::pt(26)).color(ink),
//                 text("effective = product of ancestors · per-primitive, no group compositing")
//                     .font(SFMono).fontSize(S::pt(11)).color(muted)
//             ),
// 
//             section("Levels · fill and border fade together"),
//             row(
//                 cell("1.0", "solid", backdrop(card(grape)())),
//                 cell("0.75", "", backdrop(card(grape).opacity(0.75f)())),
//                 cell("0.5", "", backdrop(card(grape).opacity(0.5f)())),
//                 cell("0.25", "", backdrop(card(grape).opacity(0.25f)())),
//                 cell("0.0", "invisible", backdrop(card(grape).opacity(0.0f)()))
//             ),
// 
//             section("Composition with color alpha"),
//             row(
//                 cell("alpha 0.5, opacity 1", "fill 0.5 · border 1",
//                     backdrop(card(simd_float4{grape.x, grape.y, grape.z, 0.5f})())
//                 ),
//                 cell("alpha 1, opacity 0.5", "fill 0.5 · border 0.5",
//                     backdrop(card(grape).opacity(0.5f)())
//                 ),
//                 cell("alpha 0.5, opacity 0.5", "fill 0.25 · border 0.5",
//                     backdrop(card(simd_float4{grape.x, grape.y, grape.z, 0.5f}).opacity(0.5f)())
//                 ),
//                 cell("shadow", "0.5 · shadow halves too",
//                     backdrop(
//                         card(paper).opacity(0.5f)
//                             .shadowOffsetY(S::px(8))
//                             .shadowBlur(S::px(24))
//                             .shadowColor(umbra)
//                         ()
//                     )
//                 )
//             ),
// 
//             section("Inheritance · child effective = parent × child"),
//             row(
//                 cell("parent 0.5, child 1", "child renders at 0.5",
//                     backdrop(
//                         centered(card(mint).opacity(0.5f))
//                         (
//                             div(S::px(80), S::px(50), grape).cornerRadius(S::px(8))()
//                         )
//                     )
//                 ),
//                 cell("parent 0.5, child 0.5", "child renders at 0.25",
//                     backdrop(
//                         centered(card(mint).opacity(0.5f))
//                         (
//                             div(S::px(80), S::px(50), grape).cornerRadius(S::px(8)).opacity(0.5f)()
//                         )
//                     )
//                 ),
//                 cell("parent 1, child 0.5", "only child fades",
//                     backdrop(
//                         centered(card(mint))
//                         (
//                             div(S::px(80), S::px(50), grape).cornerRadius(S::px(8)).opacity(0.5f)()
//                         )
//                     )
//                 ),
//                 cell("three deep", "0.8 × 0.8 × 0.8 ≈ 0.51",
//                     backdrop(
//                         centered(card(mint).opacity(0.8f))
//                         (
//                             centered(div(S::px(100), S::px(70), grape).cornerRadius(S::px(8)).opacity(0.8f))
//                             (
//                                 div(S::px(60), S::px(40), tangerine).cornerRadius(S::px(6)).opacity(0.8f)()
//                             )
//                         )
//                     )
//                 ),
//                 cell("text under parent", "parent 0.4 · glyphs fade",
//                     backdrop(
//                         centered(card(paper).opacity(0.4f))
//                         (
//                             text("Opacity").font(DINAlternateBold).fontSize(S::pt(22)).color(ink)
//                         )
//                     )
//                 ),
//                 cell("image under parent", "parent 0.5",
//                     backdrop(
//                         centered(card(paper).opacity(0.5f))
//                         (
//                             image(butterflyPath, S::px(120), S::px(80)).cornerRadius(S::px(8))
//                         )
//                     )
//                 )
//             ),
// 
//             section("Primitives · each shader multiplies its own alpha"),
//             row(
//                 cell("text 0.5", "premultiplied path",
//                     backdrop(
//                         text("Opacity").font(DINAlternateBold).fontSize(S::pt(26)).color(ink).opacity(0.5f)
//                     )
//                 ),
//                 cell("image 0.5", "",
//                     backdrop(image(butterflyPath, S::px(140), S::px(100)).cornerRadius(S::px(12)).opacity(0.5f))
//                 ),
//                 cell("image + border 0.5", "border fades with texels",
//                     backdrop(
//                         image(butterflyPath, S::px(140), S::px(100))
//                             .cornerRadius(S::px(12))
//                             .borderWidth(S::px(4))
//                             .borderColor(tangerine)
//                             .opacity(0.5f)
//                     )
//                 ),
//                 cell("transformed 0.5", "rotate 0.35",
//                     backdrop(card(tangerine).opacity(0.5f).rotate(0.35f)())
//                 )
//             ),
// 
//             section("Divergence · overlaps double-blend (browsers flatten a group)"),
//             row(
//                 cell("siblings both 0.5", "overlap darker · same in browsers",
//                     div().width(S::px(140)).height(S::px(100)).position(Position::Relative)
//                     (
//                         div(S::px(90), S::px(70), grape).cornerRadius(S::px(8)).opacity(0.5f)
//                             .position(Position::Absolute).top(S::px(0)).left(S::px(0))(),
//                         div(S::px(90), S::px(70), grape).cornerRadius(S::px(8)).opacity(0.5f)
//                             .position(Position::Absolute).top(S::px(30)).left(S::px(50))()
//                     )
//                 ),
//                 cell("parent 0.5, opaque children", "overlap darker · browsers: flat",
//                     div().width(S::px(140)).height(S::px(100)).position(Position::Relative).opacity(0.5f)
//                     (
//                         div(S::px(90), S::px(70), grape).cornerRadius(S::px(8))
//                             .position(Position::Absolute).top(S::px(0)).left(S::px(0))(),
//                         div(S::px(90), S::px(70), grape).cornerRadius(S::px(8))
//                             .position(Position::Absolute).top(S::px(30)).left(S::px(50))()
//                     )
//                 ),
//                 cell("child over parent fill", "parent 0.5 · child shows parent through it",
//                     backdrop(
//                         centered(card(grape).opacity(0.5f))
//                         (
//                             div(S::px(80), S::px(50), grape).cornerRadius(S::px(8))()
//                         )
//                     )
//                 )
//             ),
// 
//             section("Interaction · opacity is finalize-only, hit testing ignores it"),
//             row(
//                 cell("click cycles", "1 → 0.5 → 0.1 → 1",
//                     backdrop(
//                         card(slate)
//                             .addEventListener(EventType::Click, [](auto& node, Event&) {
//                                 float o = node.opacity();
//                                 node.opacity(o > 0.75f ? 0.5f : o > 0.25f ? 0.1f : 1.0f);
//                             })
//                         ()
//                     )
//                 ),
//                 cell("hover fades", "1 ↔ 0.4",
//                     backdrop(
//                         card(mint)
//                             .addEventListener(EventType::MouseEnter, [](auto& node, Event&) {
//                                 node.opacity(0.4f);
//                             })
//                             .addEventListener(EventType::MouseLeave, [](auto& node, Event&) {
//                                 node.opacity(1.0f);
//                             })
//                         ()
//                     )
//                 ),
//                 cell("opacity 0 still clickable", "click → 1",
//                     backdrop(
//                         card(grape).opacity(0.0f)
//                             .addEventListener(EventType::Click, [](auto& node, Event&) {
//                                 node.opacity(node.opacity() < 0.5f ? 1.0f : 0.0f);
//                             })
//                         ()
//                     )
//                 ),
//                 cell("parent click fades subtree", "child inherits the change",
//                     backdrop(
//                         centered(card(mint))
//                             .addEventListener(EventType::Click, [](auto& node, Event&) {
//                                 node.opacity(node.opacity() > 0.5f ? 0.3f : 1.0f);
//                             })
//                         (
//                             div(S::px(80), S::px(50), grape).cornerRadius(S::px(8))()
//                         )
//                     )
//                 )
//             )
//         );
//     }

    // // Links: a link is just an inline text node with a Click listener that
    // // hands the URL to the OS. Sits in an inline formatting context with
    // // regular text on either side.
    // {
    //     using S = gui::Size;
    //     using runtime::EventType;

    //     constexpr simd_float4 desk {0.941, 0.937, 0.925, 1.0};
    //     constexpr simd_float4 ink  {0.129, 0.129, 0.153, 1.0};
    //     constexpr simd_float4 blue {0.000, 0.400, 0.850, 1.0};

    //     auto link = [&](const std::string& label, std::string url) {
    //         return text(label).fontSize(S::pt(14)).color(blue)
    //             .addEventListener(EventType::Click, [url = std::move(url)](auto&, Event&) {
    //                 AppKit_Extensions::openURL(url.c_str());
    //             });
    //     };

    //     div(S::percent(1.0), S::percent(1.0), desk)
    //         .padding(S::px(32))
    //     (
    //         div()
    //         (
    //             text("This paragraph has a ").fontSize(S::pt(14)).color(ink),
    //             link("link to the repo", "https://github.com/taanishr/butterfly"),
    //             text(" in the middle of it, and another one at the end: ").fontSize(S::pt(14)).color(ink),
    //             link("apple.com", "https://www.apple.com")
    //         )
    //     );
    // }

    // // ── inline margins on multi word text ──
    // // each row is ONE text element inside a shrink to fit box. margin-left and
    // // margin-right should each apply once, at the element's start and end edge, not
    // // once per word. four words means four fragment pushes, so a per fragment margin
    // // reads as W+80 instead of W+20 and opens a gap between every word.
    // //
    // // base direction is not a builder property; it is inherited from the default at
    // // layout.hpp InheritedProperties::direction. flip that to rtl and recompile for the
    // // rtl base pass, which is why these rows are written to be read under either base.
    // {
    //     using S = gui::Size;

    //     auto marginRow = [&](const char* label, const char* content, auto configure) {
    //         return div()
    //             .display(gui::Display::Flex)
    //             .flexDirection(gui::FlexDirection::Col)
    //             .alignItems(gui::AlignItems::FlexStart)
    //             .flexGap(S::px(6))
    //         (
    //             text(label)
    //                 .font(Arial)
    //                 .fontSize(S::pt(12))
    //                 .color(simd_float4{0.38,0.92,0.56,1.0}),
    //             div(S::autoSize(), S::autoSize(), simd_float4{0.15,0.16,0.20,1.0})
    //             (
    //                 configure(
    //                     text(content)
    //                         .font(Arial)
    //                         .fontSize(S::pt(18))
    //                         .color(simd_float4{0.98,0.76,0.20,1.0})
    //                 )
    //             )
    //         );
    //     };

    //     auto adjacentRow = [&](const char* label, auto configureFirst, auto configureSecond) {
    //         return div()
    //             .display(gui::Display::Flex)
    //             .flexDirection(gui::FlexDirection::Col)
    //             .alignItems(gui::AlignItems::FlexStart)
    //             .flexGap(S::px(6))
    //         (
    //             text(label)
    //                 .font(Arial)
    //                 .fontSize(S::pt(12))
    //                 .color(simd_float4{0.38,0.92,0.56,1.0}),
    //             div(S::autoSize(), S::autoSize(), simd_float4{0.15,0.16,0.20,1.0})
    //             (
    //                 configureFirst(
    //                     text("alpha")
    //                         .font(Arial)
    //                         .fontSize(S::pt(18))
    //                         .color(simd_float4{0.98,0.76,0.20,1.0})
    //                 ),
    //                 configureSecond(
    //                     text("beta")
    //                         .font(Arial)
    //                         .fontSize(S::pt(18))
    //                         .color(simd_float4{0.45,0.80,0.98,1.0})
    //                 )
    //             )
    //         );
    //     };

    //     constexpr const char* latin = "alpha beta gamma delta";
    //     // pure rtl run: under an ltr base every fragment takes an odd bidi level, so
    //     // reorderLineFragments reverses the whole run.
    //     constexpr const char* hebrew = "שלום עולם שלום עולם";
    //     // mixed runs: only the hebrew spans reverse, so the element's logically first
    //     // and last fragments are NOT the visually leftmost and rightmost ones.
    //     constexpr const char* mixed = "alpha שלום beta עולם";

    //     div(S::percent(1.0), S::percent(1.0), simd_float4{0.06,0.07,0.09,1.0})
    //         .display(gui::Display::Flex)
    //         .flexDirection(gui::FlexDirection::Col)
    //         .alignItems(gui::AlignItems::FlexStart)
    //         .padding(S::px(48))
    //         .flexGap(S::px(20))
    //         .overflow(gui::Overflow::Scroll)
    //     (
    //         text("INLINE MARGINS ON MULTI WORD TEXT")
    //             .font(Arial)
    //             .fontSize(S::pt(22))
    //             .color(simd_float4{0.96,0.97,1.0,1.0}),

    //         marginRow("LATIN, NO MARGIN (CONTROL)", latin, [](auto t) { return t; }),
    //         marginRow("LATIN, MARGIN RIGHT 20", latin, [&](auto t) { return t.marginRight(S::px(20)); }),
    //         marginRow("LATIN, MARGIN LEFT 20", latin, [&](auto t) { return t.marginLeft(S::px(20)); }),
    //         marginRow("LATIN, MARGIN LEFT 20 + RIGHT 20", latin, [&](auto t) {
    //             return t.marginLeft(S::px(20)).marginRight(S::px(20));
    //         }),

    //         marginRow("RTL RUN, NO MARGIN (CONTROL)", hebrew, [](auto t) { return t; }),
    //         marginRow("RTL RUN, MARGIN RIGHT 20", hebrew, [&](auto t) { return t.marginRight(S::px(20)); }),
    //         marginRow("RTL RUN, MARGIN LEFT 20", hebrew, [&](auto t) { return t.marginLeft(S::px(20)); }),

    //         marginRow("MIXED, NO MARGIN (CONTROL)", mixed, [](auto t) { return t; }),
    //         marginRow("MIXED, MARGIN RIGHT 20", mixed, [&](auto t) { return t.marginRight(S::px(20)); }),
    //         marginRow("MIXED, MARGIN LEFT 20", mixed, [&](auto t) { return t.marginLeft(S::px(20)); }),

    //         adjacentRow("ADJACENT, NO MARGIN (CONTROL)",
    //             [](auto t) { return t; },
    //             [](auto t) { return t; }),
    //         adjacentRow("ADJACENT, FIRST MARGIN RIGHT 20",
    //             [&](auto t) { return t.marginRight(S::px(20)); },
    //             [](auto t) { return t; }),
    //         adjacentRow("ADJACENT, SECOND MARGIN LEFT 20",
    //             [](auto t) { return t; },
    //             [&](auto t) { return t.marginLeft(S::px(20)); }),
    //         adjacentRow("ADJACENT, BOTH 20",
    //             [&](auto t) { return t.marginRight(S::px(20)); },
    //             [&](auto t) { return t.marginLeft(S::px(20)); })
    //     );
    // }

    // // ── inline padding on multi word text ──
    // {
    //     using S = gui::Size;

    //     auto paddingRow = [&](const char* label, const char* content, auto configure) {
    //         return div()
    //             .display(gui::Display::Flex)
    //             .flexDirection(gui::FlexDirection::Col)
    //             .alignItems(gui::AlignItems::FlexStart)
    //             .flexGap(S::px(6))
    //         (
    //             text(label)
    //                 .font(Arial)
    //                 .fontSize(S::pt(12))
    //                 .color(simd_float4{0.38,0.92,0.56,1.0}),
    //             div(S::autoSize(), S::autoSize(), simd_float4{0.15,0.16,0.20,1.0})
    //             (
    //                 configure(
    //                     text(content)
    //                         .font(Arial)
    //                         .fontSize(S::pt(18))
    //                         .color(simd_float4{0.98,0.76,0.20,1.0})
    //                 )
    //             )
    //         );
    //     };

    //     constexpr const char* latin = "alpha beta gamma delta";
    //     constexpr const char* hebrew = "שלום עולם שלום עולם";
    //     constexpr const char* mixed = "alpha שלום beta עולם";

    //     div(S::percent(1.0), S::percent(1.0), simd_float4{0.06,0.07,0.09,1.0})
    //         .display(gui::Display::Flex)
    //         .flexDirection(gui::FlexDirection::Col)
    //         .alignItems(gui::AlignItems::FlexStart)
    //         .padding(S::px(48))
    //         .flexGap(S::px(20))
    //         .overflow(gui::Overflow::Scroll)
    //     (
    //         text("INLINE PADDING ON MULTI WORD TEXT")
    //             .font(Arial)
    //             .fontSize(S::pt(22))
    //             .color(simd_float4{0.96,0.97,1.0,1.0}),

    //         paddingRow("LATIN, NO PADDING (CONTROL)", latin, [](auto t) { return t; }),
    //         paddingRow("LATIN, PADDING RIGHT 20", latin, [&](auto t) { return t.paddingRight(S::px(20)); }),
    //         paddingRow("LATIN, PADDING LEFT 20", latin, [&](auto t) { return t.paddingLeft(S::px(20)); }),
    //         paddingRow("LATIN, PADDING LEFT 20 + RIGHT 20", latin, [&](auto t) {
    //             return t.paddingLeft(S::px(20)).paddingRight(S::px(20));
    //         }),

    //         paddingRow("RTL RUN, NO PADDING (CONTROL)", hebrew, [](auto t) { return t; }),
    //         paddingRow("RTL RUN, PADDING RIGHT 20", hebrew, [&](auto t) { return t.paddingRight(S::px(20)); }),
    //         paddingRow("RTL RUN, PADDING LEFT 20", hebrew, [&](auto t) { return t.paddingLeft(S::px(20)); }),
    //         paddingRow("RTL RUN, PADDING LEFT 20 + RIGHT 20", hebrew, [&](auto t) {
    //             return t.paddingLeft(S::px(20)).paddingRight(S::px(20));
    //         }),

    //         paddingRow("MIXED, NO PADDING (CONTROL)", mixed, [](auto t) { return t; }),
    //         paddingRow("MIXED, PADDING RIGHT 20", mixed, [&](auto t) { return t.paddingRight(S::px(20)); }),
    //         paddingRow("MIXED, PADDING LEFT 20", mixed, [&](auto t) { return t.paddingLeft(S::px(20)); }),
    //         paddingRow("MIXED, PADDING LEFT 20 + RIGHT 20", mixed, [&](auto t) {
    //             return t.paddingLeft(S::px(20)).paddingRight(S::px(20));
    //         }),

    //         paddingRow("LATIN, MARGIN LEFT 20 + PADDING LEFT 10", latin, [&](auto t) {
    //             return t.marginLeft(S::px(20)).paddingLeft(S::px(10));
    //         }),
    //         paddingRow("LATIN, MARGIN RIGHT 20 + PADDING RIGHT 10", latin, [&](auto t) {
    //             return t.marginRight(S::px(20)).paddingRight(S::px(10));
    //         }),
    //         paddingRow("LATIN, MARGIN 20 + PADDING 10, BOTH SIDES", latin, [&](auto t) {
    //             return t.marginLeft(S::px(20)).marginRight(S::px(20)).paddingLeft(S::px(10)).paddingRight(S::px(10));
    //         }),

    //         paddingRow("RTL RUN, MARGIN LEFT 20 + PADDING LEFT 10", hebrew, [&](auto t) {
    //             return t.marginLeft(S::px(20)).paddingLeft(S::px(10));
    //         }),
    //         paddingRow("RTL RUN, MARGIN RIGHT 20 + PADDING RIGHT 10", hebrew, [&](auto t) {
    //             return t.marginRight(S::px(20)).paddingRight(S::px(10));
    //         }),
    //         paddingRow("RTL RUN, MARGIN 20 + PADDING 10, BOTH SIDES", hebrew, [&](auto t) {
    //             return t.marginLeft(S::px(20)).marginRight(S::px(20)).paddingLeft(S::px(10)).paddingRight(S::px(10));
    //         }),

    //         paddingRow("MIXED, MARGIN LEFT 20 + PADDING LEFT 10", mixed, [&](auto t) {
    //             return t.marginLeft(S::px(20)).paddingLeft(S::px(10));
    //         }),
    //         paddingRow("MIXED, MARGIN RIGHT 20 + PADDING RIGHT 10", mixed, [&](auto t) {
    //             return t.marginRight(S::px(20)).paddingRight(S::px(10));
    //         }),
    //         paddingRow("MIXED, MARGIN 20 + PADDING 10, BOTH SIDES", mixed, [&](auto t) {
    //             return t.marginLeft(S::px(20)).marginRight(S::px(20)).paddingLeft(S::px(10)).paddingRight(S::px(10));
    //         })
    //     );
    // }

    // // Inline siblings wrapping at the element boundary: the first text fills line 0,
    // // the second text's first fragment starts on line 1.
    // {
    //     using S = gui::Size;

    //     const auto label = [&](const char* s) {
    //         return text(s)
    //             .font(Arial)
    //             .fontSize(S::pt(12))
    //             .color(simd_float4{0.38,0.92,0.56,1.0});
    //     };

    //     const auto box = [&]() {
    //         return div(S::px(160), S::autoSize(), simd_float4{0.15,0.16,0.20,1.0});
    //     };

    //     const auto yellow = simd_float4{0.98,0.76,0.20,1.0};
    //     const auto blue = simd_float4{0.45,0.80,0.98,1.0};

    //     div(S::percent(1.0), S::percent(1.0), simd_float4{0.06,0.07,0.09,1.0})
    //         .padding(S::px(48))
    //     (
    //         label("TWO ELEMENTS, WRAP BETWEEN THEM"),
    //         box()
    //         (
    //             text("alpha ").font(Arial).fontSize(S::pt(18)).color(yellow),
    //             text("betabetabetabeta").font(Arial).fontSize(S::pt(18)).color(blue)
    //         ),
    //         label("SINGLE ELEMENT (CONTROL)").marginTop(S::px(24)),
    //         box()
    //         (
    //             text("alpha betabetabetabeta").font(Arial).fontSize(S::pt(18)).color(yellow)
    //         )
    //     );
    // }

    // // ═══════════════════════════════════════════════════════════════════
    // // Clone: linear.app landing page
    // // ═══════════════════════════════════════════════════════════════════
    // {
    //     using S = gui::Size;
    //     using gui::Display;
    //     using gui::FlexDirection;
    //     using gui::AlignItems;
    //     using gui::JustifyContent;
    //     using gui::Position;

    //     constexpr auto rgb = [](int r, int g, int b, float a = 1.0f) {
    //         return simd_float4{r / 255.0f, g / 255.0f, b / 255.0f, a};
    //     };

    //     const auto bg        = rgb(8, 9, 10);
    //     const auto panel     = rgb(16, 17, 19);
    //     const auto panel2    = rgb(22, 23, 26);
    //     const auto hairline  = rgb(255, 255, 255, 0.08f);
    //     const auto hairline2 = rgb(255, 255, 255, 0.14f);
    //     const auto white     = rgb(247, 248, 248);
    //     const auto grey      = rgb(138, 143, 152);
    //     const auto dim       = rgb(98, 102, 109);
    //     const auto indigo    = rgb(94, 106, 210);
    //     const auto clear     = rgb(0, 0, 0, 0.0f);

    //     const std::string Inter     = Helvetica;
    //     const std::string InterBold = ArialBold;
    //     constexpr auto A = "/Users/treja/projects/gui/assets/clones/";

    //     auto navLink = [&](const char* label) {
    //         return text(label).font(Inter).fontSize(S::pt(11.5)).color(white);
    //     };

    //     auto issueRow = [&](const char* icon, const char* id, const char* title, const char* tag, simd_float4 tagColor) {
    //         return div()
    //             .width(S::percent(1.0))
    //             .height(S::px(34))
    //             .display(Display::Flex)
    //             .alignItems(AlignItems::Center)
    //             .flexGap(S::px(10))
    //             .paddingLeft(S::px(14))
    //             .paddingRight(S::px(14))
    //             .borderColor(hairline)
    //             .borderWidth(S::px(1))
    //         (
    //             text(id).font(SFMono).fontSize(S::pt(9)).color(dim),
    //             svg(std::string(A) + icon, S::px(13), S::px(13)),
    //             text(title).font(Inter).fontSize(S::pt(10.5)).color(white).flexGrow(S::px(1)),
    //             div(S::autoSize(), S::autoSize(), clear)
    //                 .display(Display::Flex)
    //                 .alignItems(AlignItems::Center)
    //                 .flexGap(S::px(5))
    //                 .paddingLeft(S::px(7)).paddingRight(S::px(7))
    //                 .paddingTop(S::px(2)).paddingBottom(S::px(2))
    //                 .cornerRadius(S::px(4))
    //                 .borderColor(hairline2)
    //                 .borderWidth(S::px(1))
    //             (
    //                 div(S::px(7), S::px(7), tagColor).cornerRadius(S::px(4))(),
    //                 text(tag).font(Inter).fontSize(S::pt(9)).color(grey)
    //             ),
    //             div(S::px(18), S::px(18), rgb(60, 64, 72)).cornerRadius(S::px(9))()
    //         );
    //     };

    //     auto featureCard = [&](const char* icon, const char* title, const char* body) {
    //         return div()
    //             .flexGrow(S::px(1))
    //             .minWidth(S::px(180))
    //             .color(panel)
    //             .cornerRadius(S::px(12))
    //             .borderColor(hairline)
    //             .borderWidth(S::px(1))
    //             .padding(S::px(22))
    //             .display(Display::Flex)
    //             .flexDirection(FlexDirection::Col)
    //             .flexGap(S::px(12))
    //         (
    //             svg(std::string(A) + icon, S::px(22), S::px(22)),
    //             text(title).font(InterBold).fontSize(S::pt(13)).color(white),
    //             div().width(S::percent(1.0))(
    //                 text(body).font(Inter).fontSize(S::pt(11)).color(grey).lineHeight(1.5)
    //             )
    //         );
    //     };

    //     auto customer = [&](const char* name) {
    //         return text(name).font(InterBold).fontSize(S::pt(13)).color(rgb(120, 124, 132));
    //     };

    //     div(S::percent(1.0), S::percent(1.0), bg)
    //         .overflow(gui::Overflow::Scroll)
    //     (
    //         // ── Nav ──
    //         div(S::percent(1.0), S::px(56), bg)
    //             .position(Position::Sticky)
    //             .top(S::px(0))
    //             .zIndex(10)
    //             .display(Display::Flex)
    //             .alignItems(AlignItems::Center)
    //             .justifyContent(JustifyContent::SpaceBetween)
    //             .paddingLeft(S::px(28))
    //             .paddingRight(S::px(28))
    //             .borderColor(hairline)
    //             .borderWidth(S::px(1))
    //         (
    //             div().display(Display::Flex).alignItems(AlignItems::Center).flexGap(S::px(26))(
    //                 div().display(Display::Flex).alignItems(AlignItems::Center).flexGap(S::px(8))(
    //                     svg(std::string(A) + "linear-logo.svg", S::px(20), S::px(20)),
    //                     text("Linear").font(InterBold).fontSize(S::pt(13)).color(white)
    //                 ),
    //                 navLink("Product"),
    //                 navLink("Resources"),
    //                 navLink("Pricing"),
    //                 navLink("Customers"),
    //                 navLink("Now"),
    //                 navLink("Contact")
    //             ),
    //             div().display(Display::Flex).alignItems(AlignItems::Center).flexGap(S::px(16))(
    //                 navLink("Log in"),
    //                 div(S::autoSize(), S::px(30), white)
    //                     .cornerRadius(S::px(8))
    //                     .paddingLeft(S::px(12)).paddingRight(S::px(12))
    //                     .display(Display::Flex)
    //                     .alignItems(AlignItems::Center)
    //                 (
    //                     text("Sign up").font(InterBold).fontSize(S::pt(11)).color(bg)
    //                 )
    //             )
    //         ),

    //         // ── Hero ──
    //         div()
    //             .width(S::percent(1.0))
    //             .paddingTop(S::px(88))
    //             .paddingLeft(S::px(48))
    //             .paddingRight(S::px(48))
    //             .display(Display::Flex)
    //             .flexDirection(FlexDirection::Col)
    //             .flexGap(S::px(22))
    //         (
    //             div().maxWidth(S::px(620))(
    //                 text("Linear is a purpose-built tool for planning and building products")
    //                     .font(InterBold).fontSize(S::pt(40)).color(white).lineHeight(1.08)
    //             ),
    //             div().maxWidth(S::px(460))(
    //                 text("Meet the system for modern software development. Streamline issues, projects, and product roadmaps.")
    //                     .font(Inter).fontSize(S::pt(15)).color(grey).lineHeight(1.45)
    //             ),
    //             div().display(Display::Flex).alignItems(AlignItems::Center).flexGap(S::px(18)).marginTop(S::px(6))(
    //                 div(S::autoSize(), S::px(38), white)
    //                     .cornerRadius(S::px(9))
    //                     .paddingLeft(S::px(16)).paddingRight(S::px(16))
    //                     .display(Display::Flex)
    //                     .alignItems(AlignItems::Center)
    //                 (
    //                     text("Start building").font(InterBold).fontSize(S::pt(12)).color(bg)
    //                 ),
    //                 div().display(Display::Flex).alignItems(AlignItems::Center).flexGap(S::px(4))(
    //                     text("Introducing Linear for Agents").font(Inter).fontSize(S::pt(12)).color(grey),
    //                     svg(std::string(A) + "chevron-right.svg", S::px(12), S::px(12))
    //                 )
    //             )
    //         ),

    //         // ── App mock ──
    //         div()
    //             .width(S::percent(1.0))
    //             .paddingTop(S::px(64))
    //             .paddingLeft(S::px(48))
    //             .paddingRight(S::px(48))
    //         (
    //             div(S::percent(1.0), S::px(380), panel)
    //                 .cornerRadius(S::px(14))
    //                 .borderColor(hairline2)
    //                 .borderWidth(S::px(1))
    //                 .overflow(gui::Overflow::Hidden)
    //                 .display(Display::Flex)
    //                 .shadowOffsetY(S::px(30))
    //                 .shadowBlur(S::px(80))
    //                 .shadowColor(rgb(94, 106, 210, 0.18f))
    //             (
    //                 // sidebar
    //                 div(S::px(170), S::percent(1.0), panel2)
    //                     .flexShrink(S::px(0))
    //                     .padding(S::px(14))
    //                     .display(Display::Flex)
    //                     .flexDirection(FlexDirection::Col)
    //                     .flexGap(S::px(9))
    //                 (
    //                     div().display(Display::Flex).alignItems(AlignItems::Center).flexGap(S::px(7)).marginBottom(S::px(10))(
    //                         div(S::px(16), S::px(16), indigo).cornerRadius(S::px(4))(),
    //                         text("Butterfly").font(InterBold).fontSize(S::pt(10)).color(white)
    //                     ),
    //                     text("Inbox").font(Inter).fontSize(S::pt(10)).color(grey),
    //                     text("My issues").font(Inter).fontSize(S::pt(10)).color(grey),
    //                     text("Views").font(Inter).fontSize(S::pt(10)).color(grey),
    //                     text("Workspace").font(Inter).fontSize(S::pt(8.5)).color(dim).marginTop(S::px(10)),
    //                     text("Initiatives").font(Inter).fontSize(S::pt(10)).color(grey),
    //                     text("Projects").font(Inter).fontSize(S::pt(10)).color(grey),
    //                     text("Teams").font(Inter).fontSize(S::pt(10)).color(grey),
    //                     text("Your teams").font(Inter).fontSize(S::pt(8.5)).color(dim).marginTop(S::px(10)),
    //                     div(S::autoSize(), S::px(24), rgb(255,255,255,0.05f)).cornerRadius(S::px(5))
    //                         .paddingLeft(S::px(6)).display(Display::Flex).alignItems(AlignItems::Center)
    //                     (
    //                         text("Renderer").font(Inter).fontSize(S::pt(10)).color(white)
    //                     ),
    //                     text("Layout").font(Inter).fontSize(S::pt(10)).color(grey).marginLeft(S::px(6)),
    //                     text("Text").font(Inter).fontSize(S::pt(10)).color(grey).marginLeft(S::px(6))
    //                 ),
    //                 // issue list
    //                 div()
    //                     .flexGrow(S::px(1))
    //                     .display(Display::Flex)
    //                     .flexDirection(FlexDirection::Col)
    //                 (
    //                     div(S::percent(1.0), S::px(40), panel)
    //                         .display(Display::Flex)
    //                         .alignItems(AlignItems::Center)
    //                         .justifyContent(JustifyContent::SpaceBetween)
    //                         .paddingLeft(S::px(14)).paddingRight(S::px(14))
    //                         .borderColor(hairline)
    //                         .borderWidth(S::px(1))
    //                     (
    //                         div().display(Display::Flex).alignItems(AlignItems::Center).flexGap(S::px(8))(
    //                             text("Renderer").font(InterBold).fontSize(S::pt(10.5)).color(white),
    //                             svg(std::string(A) + "chevron-right.svg", S::px(11), S::px(11)),
    //                             text("Active issues").font(Inter).fontSize(S::pt(10.5)).color(grey)
    //                         ),
    //                         div().display(Display::Flex).alignItems(AlignItems::Center).flexGap(S::px(8))(
    //                             text("Filter").font(Inter).fontSize(S::pt(9.5)).color(grey),
    //                             text("Display").font(Inter).fontSize(S::pt(9.5)).color(grey)
    //                         )
    //                     ),
    //                     div(S::percent(1.0), S::px(26), rgb(255,255,255,0.03f))
    //                         .display(Display::Flex).alignItems(AlignItems::Center).flexGap(S::px(8))
    //                         .paddingLeft(S::px(14))
    //                     (
    //                         svg(std::string(A) + "lin-inprogress.svg", S::px(12), S::px(12)),
    //                         text("In Progress").font(InterBold).fontSize(S::pt(9.5)).color(white),
    //                         text("3").font(Inter).fontSize(S::pt(9.5)).color(dim)
    //                     ),
    //                     issueRow("lin-inprogress.svg", "BUT-142", "Sticky positioning inside nested scroll containers", "Layout", rgb(94,106,210)),
    //                     issueRow("lin-inprogress.svg", "BUT-139", "SDF border joins for dashed elliptical corners", "Renderer", rgb(38,166,154)),
    //                     issueRow("lin-inprogress.svg", "BUT-131", "Bidi reordering across inline spans", "Text", rgb(242,153,74)),
    //                     div(S::percent(1.0), S::px(26), rgb(255,255,255,0.03f))
    //                         .display(Display::Flex).alignItems(AlignItems::Center).flexGap(S::px(8))
    //                         .paddingLeft(S::px(14))
    //                     (
    //                         svg(std::string(A) + "lin-backlog.svg", S::px(12), S::px(12)),
    //                         text("Todo").font(InterBold).fontSize(S::pt(9.5)).color(white),
    //                         text("4").font(Inter).fontSize(S::pt(9.5)).color(dim)
    //                     ),
    //                     issueRow("lin-backlog.svg", "BUT-150", "Glyph atlas eviction under memory pressure", "Text", rgb(242,153,74)),
    //                     issueRow("lin-backlog.svg", "BUT-148", "Grid auto-placement with dense packing", "Layout", rgb(94,106,210)),
    //                     issueRow("lin-backlog.svg", "BUT-147", "Image rendition bins for HiDPI resizes", "Renderer", rgb(38,166,154)),
    //                     issueRow("lin-done.svg",    "BUT-120", "openURL binding for link elements", "Runtime", rgb(235,87,87))
    //                 )
    //             )
    //         ),

    //         // ── Customers ──
    //         div()
    //             .width(S::percent(1.0))
    //             .paddingTop(S::px(72))
    //             .paddingLeft(S::px(48))
    //             .paddingRight(S::px(48))
    //             .display(Display::Flex)
    //             .flexDirection(FlexDirection::Col)
    //             .alignItems(AlignItems::Center)
    //             .flexGap(S::px(26))
    //         (
    //             text("Powering the world's best product teams.").font(Inter).fontSize(S::pt(12)).color(grey),
    //             div().display(Display::Flex).flexWrap(gui::FlexWrap::Wrap).justifyContent(JustifyContent::Center).flexGap(S::px(40))(
    //                 customer("ramp"), customer("Vercel"), customer("CashApp"), customer("scale"),
    //                 customer("perplexity"), customer("Retool"), customer("Mercury"), customer("Brex")
    //             )
    //         ),

    //         // ── Features ──
    //         div()
    //             .width(S::percent(1.0))
    //             .paddingTop(S::px(88))
    //             .paddingBottom(S::px(96))
    //             .paddingLeft(S::px(48))
    //             .paddingRight(S::px(48))
    //             .display(Display::Flex)
    //             .flexDirection(FlexDirection::Col)
    //             .flexGap(S::px(34))
    //         (
    //             div().maxWidth(S::px(520))(
    //                 text("Made for modern product teams")
    //                     .font(InterBold).fontSize(S::pt(28)).color(white).lineHeight(1.1)
    //             ),
    //             div().maxWidth(S::px(520))(
    //                 text("Linear is shaped by the practices and principles that distinguish world-class product teams from the rest: relentless focus, fast execution, and a commitment to the quality of craft.")
    //                     .font(Inter).fontSize(S::pt(12.5)).color(grey).lineHeight(1.5)
    //             ),
    //             div().width(S::percent(1.0)).display(Display::Flex).flexWrap(gui::FlexWrap::Wrap).flexGap(S::px(14))(
    //                 featureCard("lin-bolt.svg",   "Purpose-built for product development", "Every part of Linear is designed to keep you in flow, from keyboard-first navigation to instant sync."),
    //                 featureCard("lin-layers.svg", "Designed to move fast",                  "Optimistic updates and a local-first data layer make every interaction feel immediate."),
    //                 featureCard("lin-doc.svg",    "Crafted to perfection",                  "Obsessive attention to typography, spacing, and motion, so the tool disappears behind the work.")
    //             )
    //         )
    //     );
    // }

    // // ═══════════════════════════════════════════════════════════════════
    // // Clone: github.com repository page
    // // ═══════════════════════════════════════════════════════════════════
    // {
    //     using S = gui::Size;
    //     using gui::Display;
    //     using gui::FlexDirection;
    //     using gui::AlignItems;
    //     using gui::JustifyContent;
    //     using gui::Position;
    //     using runtime::EventType;

    //     constexpr auto rgb = [](int r, int g, int b, float a = 1.0f) {
    //         return simd_float4{r / 255.0f, g / 255.0f, b / 255.0f, a};
    //     };

    //     const auto canvas   = rgb(255, 255, 255);
    //     const auto subtle   = rgb(246, 248, 250);
    //     const auto border   = rgb(209, 217, 224);
    //     const auto fg       = rgb(31, 35, 40);
    //     const auto muted    = rgb(89, 99, 110);
    //     const auto link     = rgb(9, 105, 218);
    //     const auto green    = rgb(31, 136, 61);
    //     const auto orange   = rgb(253, 140, 115);
    //     const auto clear    = rgb(0, 0, 0, 0.0f);

    //     const std::string Sans     = Helvetica;
    //     const std::string SansBold = ArialBold;
    //     constexpr auto A = "/Users/treja/projects/gui/assets/clones/";

    //     auto icon = [&](const char* name, float size = 16) {
    //         return svg(std::string(A) + name, S::px(size), S::px(size)).flexShrink(S::px(0));
    //     };

    //     auto button = [&](simd_float4 bgColor, auto&&... contents) {
    //         return div(S::autoSize(), S::px(28), bgColor)
    //             .cornerRadius(S::px(6))
    //             .borderColor(border)
    //             .borderWidth(S::px(1))
    //             .paddingLeft(S::px(10)).paddingRight(S::px(10))
    //             .display(Display::Flex)
    //             .alignItems(AlignItems::Center)
    //             .flexGap(S::px(6))
    //             .flexShrink(S::px(0))
    //         (std::forward<decltype(contents)>(contents)...);
    //     };

    //     auto counter = [&](const char* n) {
    //         return div(S::autoSize(), S::px(18), rgb(175, 184, 193, 0.2f))
    //             .cornerRadius(S::px(9))
    //             .paddingLeft(S::px(6)).paddingRight(S::px(6))
    //             .display(Display::Flex)
    //             .alignItems(AlignItems::Center)
    //         (
    //             text(n).font(SansBold).fontSize(S::pt(9)).color(fg)
    //         );
    //     };

    //     auto tab = [&](const char* label, const char* count, bool active) {
    //         auto t = div()
    //             .height(S::px(46))
    //             .paddingLeft(S::px(8)).paddingRight(S::px(8))
    //             .display(Display::Flex)
    //             .alignItems(AlignItems::Center)
    //             .flexGap(S::px(8))
    //             .flexShrink(S::px(0))
    //             .position(Position::Relative)
    //         (
    //             text(label).font(active ? SansBold : Sans).fontSize(S::pt(10.5)).color(fg),
    //             count ? counter(count) : div(S::px(0), S::px(0), clear)(),
    //             div(S::percent(1.0), S::px(2), active ? orange : clear)
    //                 .position(Position::Absolute)
    //                 .bottom(S::px(0))
    //                 .left(S::px(0))
    //                 .cornerRadius(S::px(1))()
    //         );
    //         return t;
    //     };

    //     auto fileRow = [&](const char* iconName, const char* name, const char* message, const char* age) {
    //         return div(S::autoSize(), S::px(38), canvas)
    //             .display(Display::Flex)
    //             .alignItems(AlignItems::Center)
    //             .flexGap(S::px(10))
    //             .paddingLeft(S::px(14)).paddingRight(S::px(14))
    //             .borderColor(border)
    //             .borderTopWidth(S::px(1))
    //         (
    //             icon(iconName),
    //             div().width(S::px(150)).flexShrink(S::px(0)).overflow(gui::Overflow::Hidden)(
    //                 text(name).font(Sans).fontSize(S::pt(10.5)).color(fg)
    //             ),
    //             div().flexGrow(S::px(1)).overflow(gui::Overflow::Hidden)(
    //                 text(message).font(Sans).fontSize(S::pt(10.5)).color(muted).whiteSpace(gui::WhiteSpace::NoWrap)
    //             ),
    //             text(age).font(Sans).fontSize(S::pt(10.5)).color(muted).flexShrink(S::px(0))
    //         );
    //     };

    //     auto pill = [&](const char* label) {
    //         return div(S::autoSize(), S::px(22), rgb(221, 244, 255))
    //             .cornerRadius(S::px(11))
    //             .paddingLeft(S::px(9)).paddingRight(S::px(9))
    //             .display(Display::Flex)
    //             .alignItems(AlignItems::Center)
    //         (
    //             text(label).font(Sans).fontSize(S::pt(9.5)).color(link)
    //         );
    //     };

    //     auto sideHeading = [&](const char* label) {
    //         return text(label).font(SansBold).fontSize(S::pt(11.5)).color(fg);
    //     };

    //     auto langDot = [&](simd_float4 c, const char* name, const char* pct) {
    //         return div().display(Display::Flex).alignItems(AlignItems::Center).flexGap(S::px(6))(
    //             div(S::px(8), S::px(8), c).cornerRadius(S::px(4))(),
    //             text(name).font(SansBold).fontSize(S::pt(9.5)).color(fg),
    //             text(pct).font(Sans).fontSize(S::pt(9.5)).color(muted)
    //         );
    //     };

    //     auto paragraph = [&](const char* body) {
    //         return div().marginBottom(S::px(14))(
    //             text(body).font(Sans).fontSize(S::pt(11.5)).color(fg).lineHeight(1.55)
    //         );
    //     };

    //     auto readmeHeading = [&](const char* title, float size) {
    //         return div().paddingBottom(S::px(6)).marginTop(S::px(10)).marginBottom(S::px(14))
    //             .borderColor(rgb(209, 217, 224, 0.7f)).borderBottomWidth(S::px(1))
    //         (
    //             text(title).font(SansBold).fontSize(S::pt(size)).color(fg)
    //         );
    //     };

    //     div(S::percent(1.0), S::percent(1.0), canvas)
    //         .overflow(gui::Overflow::Scroll)
    //     (
    //         // ── Global header ──
    //         div(S::percent(1.0), S::px(60), subtle)
    //             .display(Display::Flex)
    //             .alignItems(AlignItems::Center)
    //             .flexGap(S::px(12))
    //             .paddingLeft(S::px(16)).paddingRight(S::px(16))
    //         (
    //             div(S::px(32), S::px(32), clear).cornerRadius(S::px(6)).borderColor(border).borderWidth(S::px(1))
    //                 .display(Display::Flex).alignItems(AlignItems::Center).justifyContent(JustifyContent::Center)
    //             (
    //                 div(S::px(14), S::px(2), fg).cornerRadius(S::px(1))()
    //             ),
    //             icon("gh-mark.svg", 32),
    //             div().display(Display::Flex).alignItems(AlignItems::Center).flexGap(S::px(4))(
    //                 text("taanishr").font(Sans).fontSize(S::pt(11)).color(fg),
    //                 text("/").font(Sans).fontSize(S::pt(11)).color(muted),
    //                 text("butterfly").font(SansBold).fontSize(S::pt(11)).color(fg)
    //             ),
    //             div().flexGrow(S::px(1))(),
    //             div(S::px(240), S::px(30), canvas)
    //                 .cornerRadius(S::px(6))
    //                 .borderColor(border)
    //                 .borderWidth(S::px(1))
    //                 .display(Display::Flex)
    //                 .alignItems(AlignItems::Center)
    //                 .justifyContent(JustifyContent::SpaceBetween)
    //                 .paddingLeft(S::px(8)).paddingRight(S::px(6))
    //             (
    //                 div().display(Display::Flex).alignItems(AlignItems::Center).flexGap(S::px(6))(
    //                     icon("gh-search.svg", 14),
    //                     text("Type / to search").font(Sans).fontSize(S::pt(10.5)).color(muted)
    //                 ),
    //                 div(S::px(20), S::px(20), clear).cornerRadius(S::px(4)).borderColor(border).borderWidth(S::px(1))
    //                     .display(Display::Flex).alignItems(AlignItems::Center).justifyContent(JustifyContent::Center)
    //                 (
    //                     text("/").font(Sans).fontSize(S::pt(9)).color(muted)
    //                 )
    //             ),
    //             div(S::px(32), S::px(32), rgb(94, 106, 210)).cornerRadius(S::px(16))()
    //         ),

    //         // ── Repo header ──
    //         div(S::percent(1.0), S::autoSize(), subtle)
    //             .paddingLeft(S::px(24)).paddingRight(S::px(24))
    //             .paddingTop(S::px(4))
    //             .borderColor(border)
    //             .borderBottomWidth(S::px(1))
    //             .display(Display::Flex)
    //             .flexDirection(FlexDirection::Col)
    //         (
    //             div().display(Display::Flex).alignItems(AlignItems::Center).justifyContent(JustifyContent::SpaceBetween)(
    //                 div().display(Display::Flex).alignItems(AlignItems::Center).flexGap(S::px(8))(
    //                     icon("gh-repo.svg"),
    //                     text("butterfly").font(SansBold).fontSize(S::pt(14)).color(fg)
    //                         .addEventListener(EventType::Click, [](auto&, Event&) {
    //                             AppKit_Extensions::openURL("https://github.com/taanishr/butterfly");
    //                         }),
    //                     div(S::autoSize(), S::px(20), clear)
    //                         .cornerRadius(S::px(10))
    //                         .borderColor(border).borderWidth(S::px(1))
    //                         .paddingLeft(S::px(7)).paddingRight(S::px(7))
    //                         .display(Display::Flex).alignItems(AlignItems::Center)
    //                     (
    //                         text("Public").font(Sans).fontSize(S::pt(9)).color(muted)
    //                     )
    //                 ),
    //                 div().display(Display::Flex).alignItems(AlignItems::Center).flexGap(S::px(8))(
    //                     button(subtle, icon("gh-eye.svg"),  text("Watch").font(SansBold).fontSize(S::pt(10)).color(fg), counter("4")),
    //                     button(subtle, icon("gh-fork.svg"), text("Fork").font(SansBold).fontSize(S::pt(10)).color(fg),  counter("12")),
    //                     button(subtle, icon("gh-star.svg"), text("Star").font(SansBold).fontSize(S::pt(10)).color(fg),  counter("318"))
    //                 )
    //             ),
    //             div().display(Display::Flex).alignItems(AlignItems::Center).flexGap(S::px(6)).marginTop(S::px(10))(
    //                 tab("Code", nullptr, true),
    //                 tab("Issues", "7", false),
    //                 tab("Pull requests", "2", false),
    //                 tab("Actions", nullptr, false),
    //                 tab("Projects", nullptr, false),
    //                 tab("Wiki", nullptr, false),
    //                 tab("Security", nullptr, false),
    //                 tab("Insights", nullptr, false)
    //             )
    //         ),

    //         // ── Body ──
    //         div()
    //             .width(S::percent(1.0))
    //             .padding(S::px(24))
    //             .display(Display::Grid)
    //             .gridTemplateColumns({S::fr(1), S::px(230)})
    //             .gridColumnGap(S::px(24))
    //         (
    //             // main column
    //             div()(
    //                 // branch bar
    //                 div().display(Display::Flex).alignItems(AlignItems::Center).justifyContent(JustifyContent::SpaceBetween)(
    //                     div().display(Display::Flex).alignItems(AlignItems::Center).flexGap(S::px(8))(
    //                         button(subtle, icon("gh-branch.svg"), text("main").font(SansBold).fontSize(S::pt(10)).color(fg), text("▾").font(Sans).fontSize(S::pt(9)).color(muted)),
    //                         div().display(Display::Flex).alignItems(AlignItems::Center).flexGap(S::px(5))(
    //                             icon("gh-branch.svg", 14),
    //                             text("3 Branches").font(Sans).fontSize(S::pt(10)).color(muted)
    //                         )
    //                     ),
    //                     div().display(Display::Flex).alignItems(AlignItems::Center).flexGap(S::px(8))(
    //                         button(subtle, text("Add file").font(SansBold).fontSize(S::pt(10)).color(fg), text("▾").font(Sans).fontSize(S::pt(9)).color(muted)),
    //                         button(green, text("<> Code").font(SansBold).fontSize(S::pt(10)).color(canvas), text("▾").font(Sans).fontSize(S::pt(9)).color(canvas))
    //                     )
    //                 ),
    //                 // file table
    //                 div().marginTop(S::px(16)).cornerRadius(S::px(6)).borderColor(border).borderWidth(S::px(1)).overflow(gui::Overflow::Hidden)(
    //                     div(S::autoSize(), S::px(52), subtle)
    //                         .display(Display::Flex)
    //                         .alignItems(AlignItems::Center)
    //                         .justifyContent(JustifyContent::SpaceBetween)
    //                         .paddingLeft(S::px(14)).paddingRight(S::px(14))
    //                     (
    //                         div().display(Display::Flex).alignItems(AlignItems::Center).flexGap(S::px(8))(
    //                             div(S::px(20), S::px(20), rgb(94, 106, 210)).cornerRadius(S::px(10))(),
    //                             text("taanishr").font(SansBold).fontSize(S::pt(10.5)).color(fg),
    //                             text("removal of measure, added an openURL binding").font(Sans).fontSize(S::pt(10.5)).color(muted)
    //                         ),
    //                         div().display(Display::Flex).alignItems(AlignItems::Center).flexGap(S::px(8))(
    //                             icon("gh-check.svg", 14),
    //                             text("5f55633").font(SFMono).fontSize(S::pt(9.5)).color(muted),
    //                             text("· 2 hours ago").font(Sans).fontSize(S::pt(10.5)).color(muted),
    //                             text("184 Commits").font(SansBold).fontSize(S::pt(10.5)).color(fg).marginLeft(S::px(8))
    //                         )
    //                     ),
    //                     fileRow("gh-folder.svg", "apple-extensions", "Swift AppKit/MTK bridge for the metal-cpp side",   "3 weeks ago"),
    //                     fileRow("gh-folder.svg", "assets",           "Add plane.svg for resvg smoke test",                "2 months ago"),
    //                     fileRow("gh-folder.svg", "blog",             "Post: SDF borders and why dashed corners are hard", "last week"),
    //                     fileRow("gh-folder.svg", "scripts",          "ship.sh: add --frame-width/--frame-height",         "2 weeks ago"),
    //                     fileRow("gh-folder.svg", "src",              "removal of measure, added an openURL binding",      "2 hours ago"),
    //                     fileRow("gh-folder.svg", "tests",            "Layout: browser reference compare harness",         "3 weeks ago"),
    //                     fileRow("gh-file.svg",   "AGENTS.md",        "Agent guidance",                                    "last month"),
    //                     fileRow("gh-file.svg",   "CMakeLists.txt",   "Frame size cache variables",                        "2 weeks ago"),
    //                     fileRow("gh-file.svg",   "CMakePresets.json","Add profile-inspector preset",                      "last month"),
    //                     fileRow("gh-file.svg",   "LICENSE",          "Initial commit",                                    "last year"),
    //                     fileRow("gh-file.svg",   "README.md",        "Fix typo in Scene I section of README",             "yesterday"),
    //                     fileRow("gh-file.svg",   "ROADMAP.md",       "Roadmap: text shaping, grid dense packing",         "3 weeks ago")
    //                 ),
    //                 // README
    //                 div().marginTop(S::px(16)).cornerRadius(S::px(6)).borderColor(border).borderWidth(S::px(1))(
    //                     div(S::autoSize(), S::px(44), canvas)
    //                         .display(Display::Flex).alignItems(AlignItems::Center).flexGap(S::px(16))
    //                         .paddingLeft(S::px(14))
    //                         .borderColor(border).borderBottomWidth(S::px(1))
    //                     (
    //                         text("README").font(SansBold).fontSize(S::pt(10.5)).color(fg),
    //                         text("MIT license").font(Sans).fontSize(S::pt(10.5)).color(muted)
    //                     ),
    //                     div().padding(S::px(28))(
    //                         readmeHeading("Butterfly - A declarative, GPU-rendered UI library", 20),
    //                         readmeHeading("Synopsis:", 15),
    //                         paragraph("Butterfly is a declarative, GPU-rendered UI library that aims to implement a useful enough subset of the HTML/CSS standard. The library itself loosely follows the CSS spec (it is a pain to read)."),
    //                         paragraph("Butterfly is largely written in C++, with a few Swift bindings. It currently only supports MacOS, as it relies on Metal as its graphics api."),
    //                         div().marginBottom(S::px(14))(
    //                             text("Currently, there is support for:").font(Sans).fontSize(S::pt(11.5)).color(fg)
    //                         ),
    //                         div().paddingLeft(S::px(22)).display(Display::Flex).flexDirection(FlexDirection::Col).flexGap(S::px(4))(
    //                             text("•  Four semantic elements: SVGs, Images, Divs, Text").font(Sans).fontSize(S::pt(11.5)).color(fg),
    //                             text("•  Display modes: Flexbox, Grid, Block, Inline").font(Sans).fontSize(S::pt(11.5)).color(fg),
    //                             text("•  Positioning: Static, Relative, Absolute, Fixed, Sticky").font(Sans).fontSize(S::pt(11.5)).color(fg),
    //                             text("•  Many many many borders").font(Sans).fontSize(S::pt(11.5)).color(fg),
    //                             text("•  Super smooth text, unicode support, bidi support").font(Sans).fontSize(S::pt(11.5)).color(fg)
    //                         )
    //                     )
    //                 )
    //             ),
    //             // sidebar
    //             div().display(Display::Flex).flexDirection(FlexDirection::Col).flexGap(S::px(14))(
    //                 sideHeading("About"),
    //                 div()(
    //                     text("A declarative, GPU-rendered UI library for macOS. Flexbox, grid, sticky positioning, SDF borders, and real text — on Metal.")
    //                         .font(Sans).fontSize(S::pt(10.5)).color(fg).lineHeight(1.5)
    //                 ),
    //                 div().display(Display::Flex).flexWrap(gui::FlexWrap::Wrap).flexGap(S::px(5))(
    //                     pill("metal"), pill("cpp"), pill("gui"), pill("flexbox"), pill("css-grid"), pill("sdf"), pill("freetype")
    //                 ),
    //                 div().display(Display::Flex).flexDirection(FlexDirection::Col).flexGap(S::px(8))(
    //                     text("Readme").font(Sans).fontSize(S::pt(10.5)).color(muted),
    //                     text("MIT license").font(Sans).fontSize(S::pt(10.5)).color(muted),
    //                     div().display(Display::Flex).alignItems(AlignItems::Center).flexGap(S::px(5))(
    //                         icon("gh-star.svg", 14), text("318 stars").font(Sans).fontSize(S::pt(10.5)).color(muted)
    //                     ),
    //                     div().display(Display::Flex).alignItems(AlignItems::Center).flexGap(S::px(5))(
    //                         icon("gh-eye.svg", 14), text("4 watching").font(Sans).fontSize(S::pt(10.5)).color(muted)
    //                     ),
    //                     div().display(Display::Flex).alignItems(AlignItems::Center).flexGap(S::px(5))(
    //                         icon("gh-fork.svg", 14), text("12 forks").font(Sans).fontSize(S::pt(10.5)).color(muted)
    //                     )
    //                 ),
    //                 div(S::percent(1.0), S::px(1), border).marginTop(S::px(6))(),
    //                 div().display(Display::Flex).alignItems(AlignItems::Center).flexGap(S::px(6))(
    //                     sideHeading("Releases"), counter("3")
    //                 ),
    //                 div().display(Display::Flex).flexDirection(FlexDirection::Col).flexGap(S::px(3))(
    //                     text("v0.1.0 — Sticky & SDF borders").font(SansBold).fontSize(S::pt(10.5)).color(fg),
    //                     text("Latest · 2 weeks ago").font(Sans).fontSize(S::pt(9.5)).color(muted)
    //                 ),
    //                 div(S::percent(1.0), S::px(1), border).marginTop(S::px(6))(),
    //                 sideHeading("Languages"),
    //                 div(S::percent(1.0), S::px(8), clear).cornerRadius(S::px(4)).overflow(gui::Overflow::Hidden).display(Display::Flex).flexGap(S::px(2))(
    //                     div(S::percent(0.78), S::percent(1.0), rgb(243, 75, 125))(),
    //                     div(S::percent(0.11), S::percent(1.0), rgb(240, 80, 50))(),
    //                     div(S::percent(0.07), S::percent(1.0), rgb(175, 210, 155))(),
    //                     div(S::percent(0.04), S::percent(1.0), rgb(218, 91, 11))()
    //                 ),
    //                 div().display(Display::Flex).flexWrap(gui::FlexWrap::Wrap).flexGap(S::px(12))(
    //                     langDot(rgb(243, 75, 125), "C++", "78.4%"),
    //                     langDot(rgb(240, 80, 50), "Swift", "10.9%"),
    //                     langDot(rgb(175, 210, 155), "Metal", "6.8%"),
    //                     langDot(rgb(218, 91, 11), "CMake", "3.9%")
    //                 )
    //             )
    //         )
    //     );
    // }

    // // ═══════════════════════════════════════════════════════════════════
    // // Clone: en.wikipedia.org article (Vector 2022 skin)
    // // ═══════════════════════════════════════════════════════════════════
    // {
    //     using S = gui::Size;
    //     using gui::Display;
    //     using gui::FlexDirection;
    //     using gui::AlignItems;
    //     using gui::JustifyContent;
    //     using gui::Position;

    //     constexpr auto rgb = [](int r, int g, int b, float a = 1.0f) {
    //         return simd_float4{r / 255.0f, g / 255.0f, b / 255.0f, a};
    //     };

    //     const auto paper    = rgb(255, 255, 255);
    //     const auto chrome   = rgb(248, 249, 250);
    //     const auto rule     = rgb(162, 169, 177);
    //     const auto softRule = rgb(234, 236, 240);
    //     const auto ink      = rgb(32, 33, 34);
    //     const auto muted    = rgb(84, 89, 93);
    //     const auto link     = rgb(51, 102, 204);
    //     const auto visited  = rgb(121, 92, 161);
    //     const auto infobox  = rgb(248, 249, 250);
    //     const auto taxon    = rgb(211, 211, 211);
    //     const auto clear    = rgb(0, 0, 0, 0.0f);

    //     const std::string Sans     = Helvetica;
    //     const std::string SansBold = ArialBold;
    //     const std::string Serif    = "/System/Library/Fonts/Supplemental/Georgia.ttf";
    //     constexpr auto A = "/Users/treja/projects/gui/assets/clones/";
    //     constexpr auto butterflyPath = "/Users/treja/projects/gui/assets/butterfly.png";

    //     auto body = [&](const char* s) { return text(s).font(Sans).fontSize(S::pt(11)).color(ink).lineHeight(1.6); };
    //     auto a    = [&](const char* s) { return text(s).font(Sans).fontSize(S::pt(11)).color(link).lineHeight(1.6); };
    //     auto sup  = [&](const char* s) { return text(s).font(Sans).fontSize(S::pt(7.5)).color(link); };

    //     auto heading = [&](const char* title) {
    //         return div().width(S::percent(1.0)).marginTop(S::px(22)).marginBottom(S::px(8))
    //             .paddingBottom(S::px(3))
    //             .borderColor(rule).borderBottomWidth(S::px(1))
    //         (
    //             text(title).font(Serif).fontSize(S::pt(17)).color(ink)
    //         );
    //     };

    //     auto tocItem = [&](const char* label, bool active, int indent = 0) {
    //         return div().paddingLeft(S::px(indent * 14))(
    //             text(label).font(active ? SansBold : Sans).fontSize(S::pt(10)).color(active ? ink : link)
    //         );
    //     };

    //     auto tab = [&](const char* label, bool active) {
    //         return div()
    //             .height(S::px(38))
    //             .paddingLeft(S::px(8)).paddingRight(S::px(8))
    //             .display(Display::Flex)
    //             .alignItems(AlignItems::Center)
    //             .position(Position::Relative)
    //         (
    //             text(label).font(Sans).fontSize(S::pt(10.5)).color(active ? ink : link),
    //             div(S::percent(1.0), S::px(2), active ? ink : clear)
    //                 .position(Position::Absolute).bottom(S::px(0)).left(S::px(0))()
    //         );
    //     };

    //     auto infoRow = [&](const char* k, const char* v, bool italic = false) {
    //         return div().width(S::percent(1.0)).display(Display::Flex).alignItems(AlignItems::Center)
    //             .paddingTop(S::px(1)).paddingBottom(S::px(1))
    //         (
    //             div().width(S::px(80)).flexShrink(S::px(0))(
    //                 text(k).font(SansBold).fontSize(S::pt(9)).color(ink)
    //             ),
    //             text(v).font(Sans).fontSize(S::pt(9)).color(italic ? ink : link)
    //         );
    //     };

    //     auto taxonHeader = [&](const char* label) {
    //         return div(S::percent(1.0), S::autoSize(), taxon)
    //             .paddingTop(S::px(3)).paddingBottom(S::px(3))
    //             .display(Display::Flex).justifyContent(JustifyContent::Center)
    //         (
    //             text(label).font(SansBold).fontSize(S::pt(9.5)).color(ink)
    //         );
    //     };

    //     div(S::percent(1.0), S::percent(1.0), paper)
    //         .overflow(gui::Overflow::Scroll)
    //     (
    //         // ── Header ──
    //         div(S::percent(1.0), S::px(50), paper)
    //             .position(Position::Sticky)
    //             .top(S::px(0))
    //             .zIndex(10)
    //             .display(Display::Flex)
    //             .alignItems(AlignItems::Center)
    //             .justifyContent(JustifyContent::SpaceBetween)
    //             .paddingLeft(S::px(16)).paddingRight(S::px(16))
    //             .borderColor(softRule).borderBottomWidth(S::px(1))
    //         (
    //             div().display(Display::Flex).alignItems(AlignItems::Center).flexGap(S::px(14))(
    //                 div().display(Display::Flex).flexDirection(FlexDirection::Col).flexGap(S::px(3)).width(S::px(16))(
    //                     div(S::percent(1.0), S::px(2), ink)(),
    //                     div(S::percent(1.0), S::px(2), ink)(),
    //                     div(S::percent(1.0), S::px(2), ink)()
    //                 ),
    //                 div().display(Display::Flex).alignItems(AlignItems::Center).flexGap(S::px(8))(
    //                     div(S::px(36), S::px(36), rgb(230, 232, 235)).cornerRadius(S::px(18))
    //                         .display(Display::Flex).alignItems(AlignItems::Center).justifyContent(JustifyContent::Center)
    //                     (
    //                         text("W").font(Serif).fontSize(S::pt(20)).color(ink)
    //                     ),
    //                     div().display(Display::Flex).flexDirection(FlexDirection::Col)(
    //                         text("WIKIPEDIA").font(Serif).fontSize(S::pt(15)).color(ink),
    //                         text("The Free Encyclopedia").font(Serif).fontSize(S::pt(7.5)).color(ink)
    //                     )
    //                 ),
    //                 div(S::px(300), S::px(32), paper)
    //                     .cornerRadius(S::px(2))
    //                     .borderColor(rule).borderWidth(S::px(1))
    //                     .display(Display::Flex).alignItems(AlignItems::Center).flexGap(S::px(8))
    //                     .paddingLeft(S::px(8))
    //                     .marginLeft(S::px(10))
    //                 (
    //                     svg(std::string(A) + "wiki-search.svg", S::px(14), S::px(14)),
    //                     text("Search Wikipedia").font(Sans).fontSize(S::pt(10.5)).color(muted).flexGrow(S::px(1)),
    //                     div(S::autoSize(), S::percent(1.0), chrome)
    //                         .paddingLeft(S::px(12)).paddingRight(S::px(12))
    //                         .borderColor(rule).borderLeftWidth(S::px(1))
    //                         .display(Display::Flex).alignItems(AlignItems::Center)
    //                     (
    //                         text("Search").font(Sans).fontSize(S::pt(10.5)).color(ink)
    //                     )
    //                 )
    //             ),
    //             div().display(Display::Flex).alignItems(AlignItems::Center).flexGap(S::px(14))(
    //                 text("Donate").font(Sans).fontSize(S::pt(10.5)).color(link),
    //                 text("Create account").font(Sans).fontSize(S::pt(10.5)).color(link),
    //                 text("Log in").font(Sans).fontSize(S::pt(10.5)).color(link),
    //                 text("···").font(Sans).fontSize(S::pt(10.5)).color(muted)
    //             )
    //         ),

    //         // ── Page grid: TOC | article ──
    //         div()
    //             .width(S::percent(1.0))
    //             .display(Display::Grid)
    //             .gridTemplateColumns({S::px(180), S::fr(1)})
    //             .gridColumnGap(S::px(20))
    //             .paddingLeft(S::px(16)).paddingRight(S::px(16))
    //             .paddingTop(S::px(16))
    //         (
    //             // TOC
    //             div().display(Display::Flex).flexDirection(FlexDirection::Col).flexGap(S::px(9)).paddingTop(S::px(48))(
    //                 div().display(Display::Flex).alignItems(AlignItems::Center).justifyContent(JustifyContent::SpaceBetween).paddingBottom(S::px(6))(
    //                     text("Contents").font(SansBold).fontSize(S::pt(10.5)).color(ink),
    //                     text("hide").font(Sans).fontSize(S::pt(9)).color(link)
    //                 ),
    //                 tocItem("(Top)", true),
    //                 tocItem("Etymology", false),
    //                 tocItem("Distribution and diversity", false),
    //                 tocItem("Phylogeny and evolution", false),
    //                 tocItem("Life cycle", false),
    //                 tocItem("Egg", false, 1),
    //                 tocItem("Larva", false, 1),
    //                 tocItem("Pupa", false, 1),
    //                 tocItem("Adult", false, 1),
    //                 tocItem("Ecology", false),
    //                 tocItem("Behaviour", false),
    //                 tocItem("In culture", false),
    //                 tocItem("See also", false),
    //                 tocItem("References", false)
    //             ),

    //             // Article
    //             div().display(Display::Flex).flexDirection(FlexDirection::Col)(
    //                 // title
    //                 div().width(S::percent(1.0)).paddingBottom(S::px(4))
    //                     .borderColor(rule).borderBottomWidth(S::px(1))
    //                 (
    //                     text("Butterfly").font(Serif).fontSize(S::pt(25)).color(ink)
    //                 ),
    //                 // tabs
    //                 div().width(S::percent(1.0)).display(Display::Flex).justifyContent(JustifyContent::SpaceBetween)(
    //                     div().display(Display::Flex)(
    //                         tab("Article", true), tab("Talk", false)
    //                     ),
    //                     div().display(Display::Flex)(
    //                         tab("Read", true), tab("Edit", false), tab("View history", false), tab("Tools ▾", false)
    //                     )
    //                 ),
    //                 div().marginTop(S::px(12)).marginBottom(S::px(12))(
    //                     text("From Wikipedia, the free encyclopedia").font(Sans).fontSize(S::pt(9.5)).color(muted)
    //                 ),

    //                 // body grid: text | infobox
    //                 div().width(S::percent(1.0)).display(Display::Flex).flexGap(S::px(20))(
    //                     div().flexGrow(S::px(1)).minWidth(S::px(200))(
    //                         div().width(S::percent(1.0)).marginBottom(S::px(12))(
    //                             body("For other uses, see "), a("Butterfly (disambiguation)"), body(".")
    //                         ),
    //                         div().width(S::percent(1.0)).marginBottom(S::px(12))(
    //                             text("Butterflies").font(SansBold).fontSize(S::pt(11)).color(ink).lineHeight(1.6),
    //                             body(" are winged "), a("insects"), body(" from the "), a("lepidopteran"),
    //                             body(" suborder "), a("Rhopalocera"),
    //                             body(", characterized by large, often brightly coloured wings that often fold together when at rest, and a conspicuous, fluttering flight. The oldest butterfly fossils have been dated to the "),
    //                             a("Paleocene"), body(", about 56 million years ago, though molecular evidence suggests that they may have originated in the "),
    //                             a("Late Cretaceous"), body(".")
    //                         ),
    //                         div().width(S::percent(1.0)).marginBottom(S::px(12))(
    //                             body("Butterflies have a four-stage "), a("life cycle"), body(", and like other "), a("holometabolous"),
    //                             body(" insects they undergo complete "), a("metamorphosis"),
    //                             body(". Winged adults lay eggs on the food plant on which their "), a("larvae"),
    //                             body(", known as "), a("caterpillars"), body(", will feed. The caterpillars grow, sometimes very rapidly, and when fully developed, "),
    //                             a("pupate"), body(" in a "), a("chrysalis"),
    //                             body(". When metamorphosis is complete, the pupal skin splits, the adult insect climbs out, expands its wings to dry, and flies off."), sup("[1]")
    //                         ),
    //                         div().width(S::percent(1.0)).marginBottom(S::px(12))(
    //                             body("Some butterflies, especially in the tropics, have several generations in a year, while others have a single generation, and a few in cold locations may take several years to pass through their entire life cycle. Butterflies are often "),
    //                             a("polymorphic"), body(", and many species make use of "), a("camouflage"), body(", "), a("mimicry"),
    //                             body(", and "), a("aposematism"), body(" to evade their predators."), sup("[2]")
    //                         ),

    //                         heading("Etymology"),
    //                         div().width(S::percent(1.0)).marginBottom(S::px(12))(
    //                             body("The "), a("Oxford English Dictionary"),
    //                             body(" derives the word straightforwardly from "), a("Old English"),
    //                             text(" butorflēoge").font(Sans).fontSize(S::pt(11)).color(ink).lineHeight(1.6),
    //                             body(", butter-fly; similar names in Old Dutch and Old High German show that the name is ancient, but modern Dutch and German use different words ("),
    //                             text("vlinder").font(Sans).fontSize(S::pt(11)).color(ink), body(" and "),
    //                             text("Schmetterling").font(Sans).fontSize(S::pt(11)).color(ink),
    //                             body("). Butterflies were thought to steal milk and butter, or that they fly around in fields during the spring, when the grass is green and the butter is yellow."), sup("[3]")
    //                         ),

    //                         heading("Distribution and diversity"),
    //                         div().width(S::percent(1.0)).marginBottom(S::px(12))(
    //                             body("Butterflies are distributed worldwide except "), a("Antarctica"),
    //                             body(", totalling some 18,500 species. Of these, 775 are "), a("Nearctic"),
    //                             body("; 7,700 "), a("Neotropical"), body("; 1,575 "), a("Palearctic"), body("; 3,650 "), a("Afrotropical"),
    //                             body("; and 4,800 are distributed across the combined "), a("Oriental"), body(" and "), a("Australian/Oceania"),
    //                             body(" regions."), sup("[4]"),
    //                             body(" The "), a("monarch butterfly"),
    //                             body(" is native to the Americas, but in the nineteenth century or before, spread across the world, and is now found in Australia, New Zealand, other parts of Oceania, and the Iberian Peninsula.")
    //                         ),

    //                         heading("Phylogeny and evolution"),
    //                         div().width(S::percent(1.0)).marginBottom(S::px(12))(
    //                             body("The earliest "), a("Lepidoptera"), body(" fossils date to the "), a("Triassic"), body("–"), a("Jurassic"),
    //                             body(" boundary, around 200 million years ago."), sup("[5]"),
    //                             body(" Butterflies evolved from "), a("moths"),
    //                             body(", so while the butterflies are "), a("monophyletic"),
    //                             body(" (forming a single clade), the moths are not. The oldest known butterfly is "),
    //                             text("Protocoeliades kristenseni").font(Sans).fontSize(S::pt(11)).color(link),
    //                             body(" from the Palaeocene aged "), a("Fur Formation"), body(" of Denmark, approximately 55 million years old.")
    //                         )
    //                     ),

    //                     // infobox
    //                     div(S::px(264), S::autoSize(), infobox)
    //                         .flexShrink(S::px(0))
    //                         .alignSelf(gui::AlignSelf::FlexStart)
    //                         .borderColor(rgb(162, 169, 177)).borderWidth(S::px(1))
    //                         .padding(S::px(4))
    //                         .display(Display::Flex)
    //                         .flexDirection(FlexDirection::Col)
    //                         .flexGap(S::px(4))
    //                     (
    //                         div(S::percent(1.0), S::autoSize(), rgb(211, 211, 211))
    //                             .paddingTop(S::px(5)).paddingBottom(S::px(5))
    //                             .display(Display::Flex).flexDirection(FlexDirection::Col).alignItems(AlignItems::Center)
    //                         (
    //                             text("Butterflies").font(SansBold).fontSize(S::pt(12)).color(ink),
    //                             text("Temporal range: Paleocene–present").font(Sans).fontSize(S::pt(8)).color(ink)
    //                         ),
    //                         image(butterflyPath, S::percent(1.0), S::autoSize()).aspectRatio(1, 1),
    //                         div().width(S::percent(1.0)).display(Display::Flex).justifyContent(JustifyContent::Center).paddingBottom(S::px(4))(
    //                             text("A Peacock butterfly, Aglais io").font(Sans).fontSize(S::pt(8.5)).color(ink)
    //                         ),
    //                         taxonHeader("Scientific classification"),
    //                         div().width(S::percent(1.0)).display(Display::Flex).flexDirection(FlexDirection::Col).paddingLeft(S::px(4))(
    //                             infoRow("Kingdom:", "Animalia"),
    //                             infoRow("Phylum:", "Arthropoda"),
    //                             infoRow("Class:", "Insecta"),
    //                             infoRow("Order:", "Lepidoptera"),
    //                             infoRow("Suborder:", "Rhopalocera", true)
    //                         ),
    //                         taxonHeader("Superfamilies"),
    //                         div().width(S::percent(1.0)).display(Display::Flex).flexDirection(FlexDirection::Col).alignItems(AlignItems::Center).flexGap(S::px(2)).paddingTop(S::px(2)).paddingBottom(S::px(4))(
    //                             text("Hedyloidea").font(Sans).fontSize(S::pt(9)).color(link),
    //                             text("Hesperioidea").font(Sans).fontSize(S::pt(9)).color(link),
    //                             text("Papilionoidea").font(Sans).fontSize(S::pt(9)).color(visited)
    //                         )
    //                     )
    //                 ),

    //                 div(S::percent(1.0), S::px(64), clear)()
    //             )
    //         )
    //     );
    // }

    // // ── rtl margins: block flow, flex row, grid alignment ──
    // // flip InheritedProperties::direction in layout.hpp to rtl for this scene.
    // {
    //     using S = gui::Size;

    //     const auto label = [&](const char* s) {
    //         return text(s)
    //             .font(Arial)
    //             .fontSize(S::pt(12))
    //             .color(simd_float4{0.38,0.92,0.56,1.0});
    //     };

    //     const auto panel = simd_float4{0.15,0.16,0.20,1.0};
    //     const auto yellow = simd_float4{0.98,0.76,0.20,1.0};
    //     const auto blue = simd_float4{0.45,0.80,0.98,1.0};
    //     const auto pink = simd_float4{0.96,0.30,0.46,1.0};
    //     const auto green = simd_float4{0.38,0.92,0.56,1.0};

    //     div(S::percent(1.0), S::percent(1.0), simd_float4{0.06,0.07,0.09,1.0})
    //         .padding(S::px(48))
    //         .overflow(gui::Overflow::Scroll)
    //     (
    //         label("BLOCK FLOW: 200 WIDE, MARGIN LEFT 20 + RIGHT 40"),
    //         div(S::px(480), S::autoSize(), panel)
    //         (
    //             div(S::px(200), S::px(30), yellow).marginLeft(S::px(20)).marginRight(S::px(40))()
    //         ),

    //         label("FLEX ROW: MARGIN LEFT 20 | MARGIN RIGHT 20 | NONE").marginTop(S::px(24)),
    //         div(S::px(480), S::px(60), panel)
    //             .display(gui::Display::Flex)
    //             .alignItems(gui::AlignItems::FlexStart)
    //         (
    //             div(S::px(80), S::px(40), yellow).marginLeft(S::px(20))(),
    //             div(S::px(80), S::px(40), blue).marginRight(S::px(20))(),
    //             div(S::px(80), S::px(40), pink)()
    //         ),

    //         label("GRID: START | CENTER / END | CENTER, NO MARGIN").marginTop(S::px(24)),
    //         div(S::px(480), S::autoSize(), panel)
    //             .display(gui::Display::Grid)
    //             .gridTemplateColumns({S::px(240), S::px(240)})
    //             .gridTemplateRows({S::px(50), S::px(50)})
    //         (
    //             div(S::px(80), S::px(30), yellow).marginLeft(S::px(10)).marginRight(S::px(30)).justifySelf(gui::JustifySelf::Start)(),
    //             div(S::px(80), S::px(30), blue).marginLeft(S::px(10)).marginRight(S::px(30)).justifySelf(gui::JustifySelf::Center)(),
    //             div(S::px(80), S::px(30), pink).marginLeft(S::px(10)).marginRight(S::px(30)).justifySelf(gui::JustifySelf::End)(),
    //             div(S::px(80), S::px(30), green).justifySelf(gui::JustifySelf::Center)()
    //         ),

    //         label("FLEX COLUMN, ALIGN START: MARGIN LEFT 10 | MARGIN RIGHT 10 | NONE").marginTop(S::px(24)),
    //         div(S::px(480), S::autoSize(), panel)
    //             .display(gui::Display::Flex)
    //             .flexDirection(gui::FlexDirection::Col)
    //             .alignItems(gui::AlignItems::FlexStart)
    //         (
    //             div(S::px(80), S::px(30), yellow).marginLeft(S::px(10))(),
    //             div(S::px(80), S::px(30), blue).marginRight(S::px(10))(),
    //             div(S::px(80), S::px(30), pink)()
    //         ),

    //         label("AUTO HEIGHT: FLEX ROW, THEN BAR").marginTop(S::px(24)),
    //         div(S::px(480), S::autoSize(), panel)
    //         (
    //             div().display(gui::Display::Flex)
    //             (
    //                 div(S::px(80), S::px(40), yellow)(),
    //                 div(S::px(80), S::px(40), blue)(),
    //                 div(S::px(80), S::px(40), pink)()
    //             ),
    //             div(S::px(200), S::px(20), green)()
    //         ),

    //         label("AUTO HEIGHT: GRID, THEN BAR").marginTop(S::px(24)),
    //         div(S::px(480), S::autoSize(), panel)
    //         (
    //             div()
    //                 .display(gui::Display::Grid)
    //                 .gridTemplateColumns({S::px(100), S::px(100)})
    //             (
    //                 div(S::px(80), S::px(40), yellow)(),
    //                 div(S::px(80), S::px(40), blue)()
    //             ),
    //             div(S::px(200), S::px(20), green)()
    //         )
    //     );
    // }

    // // ═══════════════════════════════════════════════════════════════════
    // // Clone: news.ycombinator.com front page
    // // ═══════════════════════════════════════════════════════════════════
    // {
    //     using S = gui::Size;
    //     using gui::Display;
    //     using gui::FlexDirection;
    //     using gui::AlignItems;
    //     using gui::JustifyContent;

    //     constexpr auto rgb = [](int r, int g, int b, float a = 1.0f) {
    //         return simd_float4{r / 255.0f, g / 255.0f, b / 255.0f, a};
    //     };

    //     const auto page   = rgb(255, 255, 255);
    //     const auto paper  = rgb(246, 246, 239);
    //     const auto orange = rgb(255, 102, 0);
    //     const auto ink    = rgb(0, 0, 0);
    //     const auto muted  = rgb(130, 130, 130);
    //     const auto white  = rgb(255, 255, 255);

    //     const std::string Verdana     = "/System/Library/Fonts/Supplemental/Verdana.ttf";
    //     const std::string VerdanaBold = "/System/Library/Fonts/Supplemental/Verdana Bold.ttf";
    //     constexpr auto A = "/Users/treja/projects/gui/assets/clones/";

    //     struct Story {
    //         std::string title;
    //         std::string site;
    //         int points;
    //         std::string user;
    //         std::string age;
    //         int comments;
    //     };

    //     const std::vector<Story> stories {
    //         {"Show HN: A declarative, GPU-rendered UI library that follows the CSS spec", "github.com/taanishr", 412, "taanishr", "3 hours ago", 128},
    //         {"Signed distance functions for rounded rectangles, explained", "iquilezles.org", 287, "sdfnerd", "5 hours ago", 64},
    //         {"The CSS flexbox algorithm is smaller than you think", "drafts.csswg.org", 203, "rachelandrew", "6 hours ago", 91},
    //         {"Why text shaping is still hard in 2026", "harfbuzz.github.io", 356, "behdad", "7 hours ago", 142},
    //         {"Metal 4 performance notes from a year of shipping", "developer.apple.com", 118, "gpu_person", "8 hours ago", 37},
    //         {"Ask HN: What are you building on weekends?", "", 94, "dang_fan", "9 hours ago", 311},
    //         {"A tour of grid track sizing, with diagrams", "css-tricks.com", 176, "jensimmons", "10 hours ago", 48},
    //         {"FreeType 2.14 released", "freetype.org", 88, "wlemberg", "11 hours ago", 22},
    //         {"How browsers decide what 'auto' means", "web.dev", 241, "tabatkins", "12 hours ago", 77},
    //         {"Writing a bidi algorithm from scratch", "unicode.org", 132, "rtlwriter", "13 hours ago", 29},
    //         {"Show HN: I cloned Wikipedia with my own layout engine", "github.com/taanishr", 67, "taanishr", "14 hours ago", 18},
    //         {"The surprising cost of anti-aliasing in fragment shaders", "blog.demofox.org", 154, "atrix256", "15 hours ago", 41},
    //         {"Sticky positioning is a scroll-container story", "developer.mozilla.org", 99, "mdn_writer", "16 hours ago", 25},
    //         {"Launch HN: Butterfly (YC S26) - native UI with web-grade layout", "butterfly.dev", 188, "founder", "17 hours ago", 103},
    //         {"Percentages, cycles, and intrinsic sizes", "w3.org", 73, "fantasai", "18 hours ago", 16},
    //     };

    //     auto navLink = [&](const char* label) {
    //         return text(label).font(Verdana).fontSize(S::pt(10)).color(ink);
    //     };

    //     auto navSeparator = [&]() {
    //         return text("|").font(Verdana).fontSize(S::pt(10)).color(ink);
    //     };

    //     auto footerLink = [&](const char* label) {
    //         return text(label).font(Verdana).fontSize(S::pt(8)).color(ink);
    //     };

    //     auto footerSeparator = [&]() {
    //         return text("|").font(Verdana).fontSize(S::pt(8)).color(ink);
    //     };

    //     auto storyRow = [&](int rank, const Story& story) {
    //         std::string subtext = std::to_string(story.points) + " points by " + story.user + " " + story.age
    //             + " | hide | " + std::to_string(story.comments) + " comments";

    //         auto titleLine = div().display(Display::Flex).alignItems(AlignItems::Center)(
    //             div().width(S::px(26)).flexShrink(S::px(0)).display(Display::Flex).justifyContent(JustifyContent::FlexEnd)(
    //                 text(std::to_string(rank) + ".").font(Verdana).fontSize(S::pt(10)).color(muted)
    //             ),
    //             div().width(S::px(18)).flexShrink(S::px(0)).display(Display::Flex).justifyContent(JustifyContent::Center)(
    //                 svg(std::string(A) + "hn-upvote.svg", S::px(10), S::px(10))
    //             ),
    //             text(story.title).font(Verdana).fontSize(S::pt(10)).color(ink)
    //         );

    //         if (!story.site.empty()) {
    //             titleLine(
    //                 text("(" + story.site + ")").font(Verdana).fontSize(S::pt(8)).color(muted).marginLeft(S::px(5))
    //             );
    //         }

    //         return div().width(S::percent(1.0)).marginBottom(S::px(6))(
    //             titleLine,
    //             div().paddingLeft(S::px(44)).marginTop(S::px(2))(
    //                 text(subtext).font(Verdana).fontSize(S::pt(7)).color(muted)
    //             )
    //         );
    //     };

    //     auto storyList = div().width(S::percent(1.0)).paddingTop(S::px(8));

    //     for (size_t i = 0; i < stories.size(); ++i) {
    //         storyList(storyRow(static_cast<int>(i) + 1, stories[i]));
    //     }

    //     div(S::percent(1.0), S::percent(1.0), page)
    //         .overflow(gui::Overflow::Scroll)
    //         .display(Display::Flex)
    //         .justifyContent(JustifyContent::Center)
    //         .paddingTop(S::px(8))
    //     (
    //         div(S::percent(0.85), S::autoSize(), paper)(
    //             // ── Header ──
    //             div(S::percent(1.0), S::autoSize(), orange)
    //                 .display(Display::Flex)
    //                 .alignItems(AlignItems::Center)
    //                 .justifyContent(JustifyContent::SpaceBetween)
    //                 .padding(S::px(2))
    //                 .paddingRight(S::px(6))
    //             (
    //                 div().display(Display::Flex).alignItems(AlignItems::Center).flexGap(S::px(4))(
    //                     div(S::px(18), S::px(18), orange)
    //                         .borderColor(white).borderWidth(S::px(1))
    //                         .marginRight(S::px(2))
    //                         .display(Display::Flex).alignItems(AlignItems::Center).justifyContent(JustifyContent::Center)
    //                     (
    //                         text("Y").font(VerdanaBold).fontSize(S::pt(9)).color(white)
    //                     ),
    //                     text("Hacker News").font(VerdanaBold).fontSize(S::pt(10)).color(ink).marginRight(S::px(6)),
    //                     navLink("new"), navSeparator(),
    //                     navLink("past"), navSeparator(),
    //                     navLink("comments"), navSeparator(),
    //                     navLink("ask"), navSeparator(),
    //                     navLink("show"), navSeparator(),
    //                     navLink("jobs"), navSeparator(),
    //                     navLink("submit")
    //                 ),
    //                 navLink("login")
    //             ),

    //             // ── Stories ──
    //             storyList,

    //             div().paddingLeft(S::px(44)).paddingTop(S::px(4)).paddingBottom(S::px(12))(
    //                 text("More").font(Verdana).fontSize(S::pt(10)).color(ink)
    //             ),

    //             // ── Footer ──
    //             div().width(S::percent(1.0)).paddingTop(S::px(10)).paddingBottom(S::px(24))
    //                 .borderColor(orange).borderTopWidth(S::px(2))
    //                 .display(Display::Flex).flexDirection(FlexDirection::Col).alignItems(AlignItems::Center).flexGap(S::px(12))
    //             (
    //                 div().display(Display::Flex).alignItems(AlignItems::Center).flexGap(S::px(4))(
    //                     footerLink("Guidelines"), footerSeparator(),
    //                     footerLink("FAQ"), footerSeparator(),
    //                     footerLink("Lists"), footerSeparator(),
    //                     footerLink("API"), footerSeparator(),
    //                     footerLink("Security"), footerSeparator(),
    //                     footerLink("Legal"), footerSeparator(),
    //                     footerLink("Apply to YC"), footerSeparator(),
    //                     footerLink("Contact")
    //                 ),
    //                 div().display(Display::Flex).alignItems(AlignItems::Center).flexGap(S::px(6))(
    //                     text("Search:").font(Verdana).fontSize(S::pt(10)).color(muted),
    //                     div(S::px(160), S::px(18), white).borderColor(rgb(118, 118, 118)).borderWidth(S::px(1))()
    //                 )
    //             )
    //         )
    //     );
    // }

    /*
    // ═══════════════════════════════════════════════════════════════════
    // Clone: stripe.com landing page
    // ═══════════════════════════════════════════════════════════════════
    {
        using S = gui::Size;
        using gui::Display;
        using gui::FlexDirection;
        using gui::AlignItems;
        using gui::JustifyContent;
        using gui::Position;

        constexpr auto rgb = [](int r, int g, int b, float a = 1.0f) {
            return simd_float4{r / 255.0f, g / 255.0f, b / 255.0f, a};
        };

        const auto white   = rgb(255, 255, 255);
        const auto navy    = rgb(10, 37, 64);
        const auto slate   = rgb(66, 84, 102);
        const auto blurple = rgb(99, 91, 255);
        const auto mist    = rgb(246, 249, 252);
        const auto haze    = rgb(173, 189, 204);
        const auto clear   = rgb(0, 0, 0, 0.0f);

        const std::string Sans     = Helvetica;
        const std::string SansBold = ArialBold;
        constexpr auto A = "/Users/treja/projects/gui/assets/clones/";

        auto navItem = [&](const char* label) {
            return text(label).font(SansBold).fontSize(S::pt(11)).color(navy);
        };

        auto arrowButton = [&](const char* label, simd_float4 fill, simd_float4 labelColor, const char* arrow) {
            return div(S::autoSize(), S::px(38), fill)
                .cornerRadius(S::px(19))
                .paddingLeft(S::px(18)).paddingRight(S::px(14))
                .display(Display::Flex).alignItems(AlignItems::Center).flexGap(S::px(6))
            (
                text(label).font(SansBold).fontSize(S::pt(11)).color(labelColor),
                svg(std::string(A) + arrow, S::px(10), S::px(10))
            );
        };

        auto band = [&](simd_float4 color) {
            return div(S::percent(1.0), S::px(130), color).opacity(0.9f)();
        };

        auto logo = [&](const char* name) {
            return div().display(Display::Flex).justifyContent(JustifyContent::Center)(
                text(name).font(SansBold).fontSize(S::pt(17)).color(rgb(10, 37, 64, 0.55f))
            );
        };

        auto productCard = [&](simd_float4 accent, const char* title, const char* body) {
            return div(S::autoSize(), S::autoSize(), white)
                .cornerRadius(S::px(8))
                .padding(S::px(24))
                .shadowOffsetY(S::px(15)).shadowBlur(S::px(35)).shadowColor(rgb(50, 50, 93, 0.12f))
                .display(Display::Flex).flexDirection(FlexDirection::Col).flexGap(S::px(12))
            (
                div(S::px(32), S::px(32), accent).cornerRadius(S::px(8))(),
                text(title).font(SansBold).fontSize(S::pt(13)).color(navy),
                text(body).font(Sans).fontSize(S::pt(11)).color(slate).lineHeight(1.55),
                div().display(Display::Flex).alignItems(AlignItems::Center).flexGap(S::px(4))(
                    text("Learn more").font(SansBold).fontSize(S::pt(10.5)).color(blurple),
                    svg(std::string(A) + "stripe-arrow-dark.svg", S::px(10), S::px(10))
                )
            );
        };

        auto stat = [&](const char* value, const char* label) {
            return div()
                .paddingLeft(S::px(16))
                .borderColor(blurple).borderLeftWidth(S::px(1))
                .display(Display::Flex).flexDirection(FlexDirection::Col).flexGap(S::px(8))
            (
                text(value).font(SansBold).fontSize(S::pt(26)).color(white),
                text(label).font(Sans).fontSize(S::pt(11)).color(haze).lineHeight(1.5)
            );
        };

        div(S::percent(1.0), S::percent(1.0), white)
            .overflow(gui::Overflow::Scroll)
        (
            // ── Hero ──
            div(S::percent(1.0), S::px(640), white)
                .position(Position::Relative)
                .overflow(gui::Overflow::Hidden)
            (
                div(S::px(1800), S::autoSize(), clear)
                    .position(Position::Absolute)
                    .top(S::px(-360)).left(S::percent(0.3))
                    .rotate(-0.21f)
                (
                    band(rgb(169, 96, 238)),
                    band(rgb(255, 51, 61)),
                    band(rgb(255, 203, 87)),
                    band(rgb(144, 224, 255)),
                    band(rgb(169, 96, 238))
                ),

                // nav
                div(S::percent(1.0), S::px(72), clear)
                    .position(Position::Relative)
                    .paddingLeft(S::px(64)).paddingRight(S::px(64))
                    .display(Display::Flex).alignItems(AlignItems::Center).justifyContent(JustifyContent::SpaceBetween)
                (
                    div().display(Display::Flex).alignItems(AlignItems::Center).flexGap(S::px(32))(
                        text("stripe").font(SansBold).fontSize(S::pt(22)).color(navy),
                        navItem("Products"),
                        navItem("Solutions"),
                        navItem("Developers"),
                        navItem("Resources"),
                        navItem("Pricing")
                    ),
                    div().display(Display::Flex).alignItems(AlignItems::Center).flexGap(S::px(12))(
                        arrowButton("Sign in", rgb(255, 255, 255, 0.25f), white, "stripe-arrow.svg"),
                        arrowButton("Contact sales", white, blurple, "stripe-arrow-dark.svg")
                    )
                ),

                // headline
                div()
                    .position(Position::Relative)
                    .paddingLeft(S::px(64))
                    .paddingTop(S::px(96))
                    .maxWidth(S::px(760))
                    .display(Display::Flex).flexDirection(FlexDirection::Col).flexGap(S::px(28))
                (
                    text("Financial infrastructure to grow your revenue").font(SansBold).fontSize(S::pt(52)).color(navy).lineHeight(1.05),
                    div().maxWidth(S::px(540))(
                        text("Join the millions of companies that use Stripe to accept payments online and in person, embed financial services, power custom revenue models, and build a more profitable business.")
                            .font(Sans).fontSize(S::pt(14)).color(slate).lineHeight(1.55)
                    ),
                    div().display(Display::Flex).alignItems(AlignItems::Center).flexGap(S::px(16))(
                        arrowButton("Start now", blurple, white, "stripe-arrow.svg"),
                        arrowButton("Contact sales", clear, blurple, "stripe-arrow-dark.svg")
                    )
                )
            ),

            // ── Logos ──
            div()
                .width(S::percent(1.0))
                .paddingLeft(S::px(64)).paddingRight(S::px(64))
                .paddingTop(S::px(32)).paddingBottom(S::px(48))
                .display(Display::Grid)
                .gridTemplateColumns({S::fr(1), S::fr(1), S::fr(1), S::fr(1)})
                .gridRowGap(S::px(32))
            (
                logo("OpenAI"), logo("amazon"), logo("Google"), logo("Marriott"),
                logo("shopify"), logo("airbnb"), logo("URBN"), logo("lyft")
            ),

            // ── Products ──
            div(S::percent(1.0), S::autoSize(), mist)
                .paddingLeft(S::px(64)).paddingRight(S::px(64))
                .paddingTop(S::px(96)).paddingBottom(S::px(96))
                .display(Display::Flex).flexDirection(FlexDirection::Col).flexGap(S::px(20))
            (
                text("Modular solutions").font(SansBold).fontSize(S::pt(13)).color(blurple),
                div().maxWidth(S::px(720))(
                    text("A fully integrated suite of financial and payments products").font(SansBold).fontSize(S::pt(34)).color(navy).lineHeight(1.15)
                ),
                div().maxWidth(S::px(620)).marginBottom(S::px(28))(
                    text("Reduce costs, grow revenue, and run your business more efficiently on a fully integrated, AI-powered platform. Use Stripe to handle all of your payments-related needs.")
                        .font(Sans).fontSize(S::pt(13)).color(slate).lineHeight(1.55)
                ),
                div()
                    .width(S::percent(1.0))
                    .display(Display::Grid)
                    .gridTemplateColumns({S::fr(1), S::fr(1), S::fr(1)})
                    .gridColumnGap(S::px(24))
                (
                    productCard(blurple, "Payments", "Accept and optimize payments globally, online and in person."),
                    productCard(rgb(0, 212, 255), "Billing", "Manage subscriptions, invoicing, and usage-based pricing."),
                    productCard(rgb(255, 91, 145), "Connect", "Build and scale a multiparty payments platform or marketplace.")
                )
            ),

            // ── Stats ──
            div(S::percent(1.0), S::autoSize(), navy)
                .paddingLeft(S::px(64)).paddingRight(S::px(64))
                .paddingTop(S::px(96)).paddingBottom(S::px(96))
                .display(Display::Flex).flexDirection(FlexDirection::Col).flexGap(S::px(48))
            (
                div().maxWidth(S::px(640))(
                    text("The backbone of global commerce").font(SansBold).fontSize(S::pt(30)).color(white).lineHeight(1.15)
                ),
                div()
                    .width(S::percent(1.0))
                    .display(Display::Grid)
                    .gridTemplateColumns({S::fr(1), S::fr(1), S::fr(1), S::fr(1)})
                    .gridColumnGap(S::px(32))
                (
                    stat("500M+", "API requests per day, peaking at 13,000 requests a second."),
                    stat("99.999%", "historical uptime for Stripe services."),
                    stat("47+", "countries with local acquiring, optimizing acceptance rates."),
                    stat("135+", "currencies and payment methods supported.")
                )
            )
        );
    }
    */
}

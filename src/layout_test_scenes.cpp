#include "layout_test_scenes.hpp"

#include "fonts.hpp"
#include "index.hpp"
#include "tree_manager.hpp"

#include <array>
#include <fstream>
#include <limits>
#include <print>
#include <string>
#include <vector>

namespace layout_test::scenes {
    namespace {
        constexpr std::array sceneNames{
            std::string_view{"phase-a-shrink"},
            std::string_view{"auto-percent-width"},
            std::string_view{"music-player"},
            std::string_view{"intrinsic-sizing"},
            std::string_view{"sizing"},
            std::string_view{"justify-items"},
            std::string_view{"complex"},
        };

        tree::TreeNode* newestRootChild(size_t previousCount) {
            auto* root = tree::TreeStack::getCurrentTree()->getRoot();
            return root->children.size() == previousCount + 1
                ? root->children.back().get()
                : nullptr;
        }

        tree::TreeNode* musicPlayer() {
            using S = gui::Size;
            using gui::div;
            using gui::text;
            using runtime::Event;

            auto* root = tree::TreeStack::getCurrentTree()->getRoot();
            const size_t previousCount = root->children.size();

            div(S::percent(1.0), S::percent(1.0), simd_float4{0.09,0.09,0.11,1.0})
                .display(gui::Display::Flex)
                .flexDirection(gui::FlexDirection::Col)
                .paddingTop(S::px(28))
            (
                // ── Now Playing bar ──
                div(S::percent(1.0), S::px(64), simd_float4{0.13,0.13,0.16,1.0})
                    .borderColor(simd_float4{0.22,0.22,0.26,1.0})
                    .borderWidth(S::px(1))
                    .flexShrink(S::px(0.0))
                    .display(gui::Display::Flex)
                    .alignItems(gui::AlignItems::Center)
                    .justifyContent(gui::JustifyContent::SpaceBetween)
                    .paddingLeft(S::px(24))
                    .paddingRight(S::px(24))
                (
                    div()
                        .display(gui::Display::Flex)
                        .alignItems(gui::AlignItems::Center)
                        .flexGap(S::px(12))
                    (
                        div(S::px(40), S::px(40), simd_float4{0.18,0.72,0.56,1.0})
                            .cornerRadius(S::px(8))
                            .display(gui::Display::Flex)
                            .alignItems(gui::AlignItems::Center)
                            .justifyContent(gui::JustifyContent::Center)
                        (
                            elements::span().fontSize(S::pt(20)).color(simd_float4{1.0,1.0,1.0,1.0})(text("♪"))
                        ),
                        div()
                            .display(gui::Display::Flex)
                            .flexDirection(gui::FlexDirection::Col)
                            .flexGap(S::px(3))
                        (
                            elements::span().fontSize(S::pt(14)).font(ArialBold).color(simd_float4{0.92,0.92,0.94,1.0})(text("Endless Reverie")),
                            elements::span().fontSize(S::pt(12)).color(simd_float4{0.48,0.48,0.54,1.0})(text("Glass Prism  ·  Mirrors"))
                        )
                    ),
                    div()
                        .display(gui::Display::Flex)
                        .alignItems(gui::AlignItems::Center)
                        .flexGap(S::px(16))
                    (
                        div(S::px(32), S::px(32), simd_float4{0.20,0.20,0.24,1.0})
                            .cornerRadius(S::px(16))
                            .display(gui::Display::Flex)
                            .alignItems(gui::AlignItems::Center)
                            .justifyContent(gui::JustifyContent::Center)
                        (
                            elements::span().fontSize(S::pt(11)).color(simd_float4{0.65,0.65,0.70,1.0})(text("|<"))
                        ),
                        div(S::px(44), S::px(44), simd_float4{0.18,0.72,0.56,1.0})
                            .cornerRadius(S::px(22))
                            .display(gui::Display::Flex)
                            .alignItems(gui::AlignItems::Center)
                            .justifyContent(gui::JustifyContent::Center)
                        (
                            elements::span().fontSize(S::pt(15)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})(text("||"))
                        ),
                        div(S::px(32), S::px(32), simd_float4{0.20,0.20,0.24,1.0})
                            .cornerRadius(S::px(16))
                            .display(gui::Display::Flex)
                            .alignItems(gui::AlignItems::Center)
                            .justifyContent(gui::JustifyContent::Center)
                        (
                            elements::span().fontSize(S::pt(11)).color(simd_float4{0.65,0.65,0.70,1.0})(text(">|"))
                        )
                    ),
                    div()
                        .display(gui::Display::Flex)
                        .alignItems(gui::AlignItems::Center)
                        .flexGap(S::px(10))
                    (
                        elements::span().fontSize(S::pt(12)).color(simd_float4{0.48,0.48,0.54,1.0})(text("2:14")),
                        div(S::px(100), S::px(4), simd_float4{0.24,0.24,0.28,1.0})
                            .cornerRadius(S::px(2))
                        (
                            div(S::px(48), S::px(4), simd_float4{0.18,0.72,0.56,1.0})
                                .cornerRadius(S::px(2))()
                        ),
                        elements::span().fontSize(S::pt(12)).color(simd_float4{0.48,0.48,0.54,1.0})(text("4:38"))
                    )
                ),
                // ── Body ──
                div(S::percent(1.0), S::percent(1.0), simd_float4{0.0,0.0,0.0,0.0})
                    .display(gui::Display::Flex)
                    .flexGrow(S::px(1))
                (
                    // Left: scrollable playlist
                    div(S::px(260), S::percent(1.0), simd_float4{0.11,0.11,0.14,1.0})
                        .borderColor(simd_float4{0.20,0.20,0.24,1.0})
                        .borderWidth(S::px(1))
                        .flexShrink(S::px(0.0))
                        .paddingTop(S::px(16))
                        .paddingBottom(S::px(16))
                        .overflow(gui::Overflow::Scroll)
                        .display(gui::Display::Flex)
                        .flexDirection(gui::FlexDirection::Col)
                        .flexGap(S::px(1))
                    (
                        div()
                            .paddingLeft(S::px(16))
                            .paddingRight(S::px(16))
                            .paddingBottom(S::px(10))
                            .display(gui::Display::Flex)
                            .alignItems(gui::AlignItems::Center)
                            .justifyContent(gui::JustifyContent::SpaceBetween)
                        (
                            elements::span().fontSize(S::pt(10)).font(ArialBold).color(simd_float4{0.38,0.38,0.44,1.0})(text("PLAYLIST")),
                            elements::span().fontSize(S::pt(10)).color(simd_float4{0.38,0.38,0.44,1.0})(text("12 tracks"))
                        ),
                        // 01 – active
                        div(S::percent(1.0), S::px(52), simd_float4{0.14,0.22,0.20,1.0})
                            .paddingLeft(S::px(16)).paddingRight(S::px(16))
                            .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
                        (
                            div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
                            (
                                elements::span().fontSize(S::pt(11)).color(simd_float4{0.18,0.72,0.56,1.0})(text("01")),
                                elements::span().fontSize(S::pt(13)).font(ArialBold).color(simd_float4{0.18,0.72,0.56,1.0})(text("Endless Reverie"))
                            ),
                            elements::span().fontSize(S::pt(12)).color(simd_float4{0.18,0.72,0.56,1.0})(text("4:38"))
                        ),
                        // 02
                        div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
                            .paddingLeft(S::px(16)).paddingRight(S::px(16))
                            .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
                        (
                            div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
                            (
                                elements::span().fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0})(text("02")),
                                elements::span().fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})(text("Crystalline"))
                            ),
                            elements::span().fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})(text("3:52"))
                        ),
                        // 03
                        div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
                            .paddingLeft(S::px(16)).paddingRight(S::px(16))
                            .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
                        (
                            div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
                            (
                                elements::span().fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0})(text("03")),
                                elements::span().fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})(text("Pale Shore"))
                            ),
                            elements::span().fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})(text("5:14"))
                        ),
                        // 04
                        div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
                            .paddingLeft(S::px(16)).paddingRight(S::px(16))
                            .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
                        (
                            div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
                            (
                                elements::span().fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0})(text("04")),
                                elements::span().fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})(text("Inversion"))
                            ),
                            elements::span().fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})(text("4:07"))
                        ),
                        // 05
                        div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
                            .paddingLeft(S::px(16)).paddingRight(S::px(16))
                            .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
                        (
                            div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
                            (
                                elements::span().fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0})(text("05")),
                                elements::span().fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})(text("Soft Architecture"))
                            ),
                            elements::span().fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})(text("6:21"))
                        ),
                        // 06
                        div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
                            .paddingLeft(S::px(16)).paddingRight(S::px(16))
                            .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
                        (
                            div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
                            (
                                elements::span().fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0})(text("06")),
                                elements::span().fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})(text("Between Frames"))
                            ),
                            elements::span().fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})(text("3:44"))
                        ),
                        // 07
                        div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
                            .paddingLeft(S::px(16)).paddingRight(S::px(16))
                            .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
                        (
                            div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
                            (
                                elements::span().fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0})(text("07")),
                                elements::span().fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})(text("Nocturne Loop"))
                            ),
                            elements::span().fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})(text("4:58"))
                        ),
                        // 08
                        div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
                            .paddingLeft(S::px(16)).paddingRight(S::px(16))
                            .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
                        (
                            div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
                            (
                                elements::span().fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0})(text("08")),
                                elements::span().fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})(text("Refract"))
                            ),
                            elements::span().fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})(text("3:30"))
                        ),
                        // 09
                        div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
                            .paddingLeft(S::px(16)).paddingRight(S::px(16))
                            .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
                        (
                            div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
                            (
                                elements::span().fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0})(text("09")),
                                elements::span().fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})(text("Diffusion"))
                            ),
                            elements::span().fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})(text("5:02"))
                        ),
                        // 10
                        div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
                            .paddingLeft(S::px(16)).paddingRight(S::px(16))
                            .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
                        (
                            div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
                            (
                                elements::span().fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0})(text("10")),
                                elements::span().fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})(text("Afterimage"))
                            ),
                            elements::span().fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})(text("4:15"))
                        ),
                        // 11
                        div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
                            .paddingLeft(S::px(16)).paddingRight(S::px(16))
                            .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
                        (
                            div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
                            (
                                elements::span().fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0})(text("11")),
                                elements::span().fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})(text("Threshold"))
                            ),
                            elements::span().fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})(text("7:03"))
                        ),
                        // 12
                        div(S::percent(1.0), S::px(52), simd_float4{0.0,0.0,0.0,0.0})
                            .paddingLeft(S::px(16)).paddingRight(S::px(16))
                            .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).justifyContent(gui::JustifyContent::SpaceBetween)
                        (
                            div().display(gui::Display::Flex).alignItems(gui::AlignItems::Center).flexGap(S::px(12))
                            (
                                elements::span().fontSize(S::pt(11)).color(simd_float4{0.35,0.35,0.42,1.0})(text("12")),
                                elements::span().fontSize(S::pt(13)).color(simd_float4{0.75,0.75,0.80,1.0})(text("Dissolve"))
                            ),
                            elements::span().fontSize(S::pt(12)).color(simd_float4{0.38,0.38,0.44,1.0})(text("4:49"))
                        )
                    ),
                    // Right: album info + scrollable lyrics
                    div(S::percent(1.0), S::percent(1.0), simd_float4{0.10,0.10,0.12,1.0})
                        .flexGrow(S::px(1))
                        .display(gui::Display::Flex)
                        .flexDirection(gui::FlexDirection::Col)
                        .padding(S::px(32))
                        .flexGap(S::px(20))
                    (
                        // Album banner
                        div(S::percent(1.0), S::px(160), simd_float4{0.0,0.0,0.0,0.0})
                            .display(gui::Display::Flex)
                            .alignItems(gui::AlignItems::Center)
                            .flexGap(S::px(24))
                            .flexShrink(S::px(0.0))
                        (
                            div(S::px(152), S::px(152), simd_float4{0.18,0.72,0.56,1.0})
                                .cornerRadius(S::px(14))
                                .display(gui::Display::Flex)
                                .alignItems(gui::AlignItems::Center)
                                .justifyContent(gui::JustifyContent::Center)
                            (
                                elements::span().fontSize(S::pt(52)).color(simd_float4{1.0,1.0,1.0,1.0})(text("♫"))
                            ),
                            div()
                                .display(gui::Display::Flex)
                                .flexDirection(gui::FlexDirection::Col)
                                .flexGap(S::px(6))
                            (
                                elements::span().fontSize(S::pt(10)).font(ArialBold).color(simd_float4{0.38,0.38,0.44,1.0})(text("ALBUM")),
                                elements::span()
                                    .fontSize(S::pt(30))
                                    .font(ArialBold)
                                    .color(simd_float4{0.92,0.92,0.94,1.0})(text("Mirrors")
                                    .addEventListener(runtime::EventType::MouseDown, [](auto& node, Event&) {
                                        node.text(node.text() == "Mirrors" ? "Mirrors — Extended Edition" : "Mirrors");
                                    })),
                                elements::span().fontSize(S::pt(16)).color(simd_float4{0.18,0.72,0.56,1.0})(text("Glass Prism")),
                                elements::span().fontSize(S::pt(12)).color(simd_float4{0.48,0.48,0.54,1.0})(text("2024  ·  Ambient  ·  12 tracks")),
                                div()
                                    .display(gui::Display::Flex)
                                    .flexGap(S::px(10))
                                    .paddingTop(S::px(8))
                                (
                                    div(S::px(96), S::px(32), simd_float4{0.18,0.72,0.56,1.0})
                                        .cornerRadius(S::px(16))
                                        .display(gui::Display::Flex)
                                        .alignItems(gui::AlignItems::Center)
                                        .justifyContent(gui::JustifyContent::Center)
                                    (
                                        elements::span().fontSize(S::pt(13)).font(ArialBold).color(simd_float4{1.0,1.0,1.0,1.0})(text("Play all"))
                                    ),
                                    div(S::px(96), S::px(32), simd_float4{0.20,0.20,0.24,1.0})
                                        .cornerRadius(S::px(16))
                                        .display(gui::Display::Flex)
                                        .alignItems(gui::AlignItems::Center)
                                        .justifyContent(gui::JustifyContent::Center)
                                    (
                                        elements::span().fontSize(S::pt(13)).color(simd_float4{0.72,0.72,0.78,1.0})(text("Shuffle"))
                                    )
                                )
                            )
                        ),
                        // Divider
                        div(S::percent(1.0), S::px(1), simd_float4{0.20,0.20,0.24,1.0})
                            .flexShrink(S::px(0.0))(),
                        // Lyrics label row
                        div()
                            .display(gui::Display::Flex)
                            .alignItems(gui::AlignItems::Center)
                            .justifyContent(gui::JustifyContent::SpaceBetween)
                            .flexShrink(S::px(0.0))
                        (
                            elements::span().fontSize(S::pt(10)).font(ArialBold).color(simd_float4{0.38,0.38,0.44,1.0})(text("LYRICS")),
                            elements::span().fontSize(S::pt(12)).color(simd_float4{0.48,0.48,0.54,1.0})(text("Endless Reverie"))
                        ),
                        // Scrollable lyrics
                        div(S::percent(1.0), S::percent(1.0), simd_float4{0.13,0.13,0.16,1.0})
                            .cornerRadius(S::px(12))
                            .padding(S::px(22))
                            .overflow(gui::Overflow::Scroll)
                            .display(gui::Display::Flex)
                            .flexDirection(gui::FlexDirection::Col)
                            .flexGap(S::px(7))
                            .flexGrow(S::px(1))
                        (
                            elements::span().fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0})(text("Through the glass, a world apart,")),
                            elements::span().fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0})(text("endless echoes fill the dark.")),
                            elements::span().fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0})(text("Fractures in the silver light —")),
                            div(S::percent(1.0), S::px(8), simd_float4{0.0,0.0,0.0,0.0})(),
                            elements::span().fontSize(S::pt(15)).color(simd_float4{0.92,0.92,0.94,1.0})(text("Hold a breath and feel the weight")),
                            elements::span().fontSize(S::pt(15)).color(simd_float4{0.92,0.92,0.94,1.0})(text("of every word you couldn't say,")),
                            elements::span().fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0})(text("mirrored back in shades of grey.")),
                            div(S::percent(1.0), S::px(8), simd_float4{0.0,0.0,0.0,0.0})(),
                            elements::span().fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0})(text("Reverie, reverie —")),
                            elements::span().fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0})(text("I found you at the edge of sleep.")),
                            elements::span().fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0})(text("Reverie, reverie —")),
                            elements::span().fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0})(text("a promise too fragile to keep.")),
                            div(S::percent(1.0), S::px(8), simd_float4{0.0,0.0,0.0,0.0})(),
                            elements::span().fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0})(text("Soft light bends around your face,")),
                            elements::span().fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0})(text("I chase the outline, lose the trace.")),
                            elements::span().fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0})(text("The mirror holds what time erased —")),
                            div(S::percent(1.0), S::px(8), simd_float4{0.0,0.0,0.0,0.0})(),
                            elements::span().fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0})(text("Still you linger in the seams,")),
                            elements::span().fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0})(text("half-remembered, half in dreams.")),
                            elements::span().fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0})(text("I reach — the surface bends and gleams.")),
                            div(S::percent(1.0), S::px(8), simd_float4{0.0,0.0,0.0,0.0})(),
                            elements::span().fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0})(text("Reverie, reverie —")),
                            elements::span().fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0})(text("I found you at the edge of sleep.")),
                            elements::span().fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0})(text("Reverie, reverie —")),
                            elements::span().fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0})(text("a promise too fragile to keep.")),
                            div(S::percent(1.0), S::px(8), simd_float4{0.0,0.0,0.0,0.0})(),
                            elements::span().fontSize(S::pt(14)).color(simd_float4{0.35,0.35,0.42,1.0})(text("(Instrumental)")),
                            div(S::percent(1.0), S::px(8), simd_float4{0.0,0.0,0.0,0.0})(),
                            elements::span().fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0})(text("The glass grows cold, the echo fades,")),
                            elements::span().fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0})(text("and all that's left is what remains —")),
                            elements::span().fontSize(S::pt(15)).color(simd_float4{0.52,0.52,0.58,1.0})(text("a shape of light, a broken name.")),
                            div(S::percent(1.0), S::px(8), simd_float4{0.0,0.0,0.0,0.0})(),
                            elements::span().fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0})(text("Reverie, reverie —")),
                            elements::span().fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0})(text("I found you at the edge of sleep.")),
                            elements::span().fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0})(text("Reverie, reverie —")),
                            elements::span().fontSize(S::pt(15)).color(simd_float4{0.18,0.72,0.56,1.0})(text("a promise too fragile to keep."))
                        )
                    )
                )
            );

            return newestRootChild(previousCount);
        }

        tree::TreeNode* sizing() {
            using S = gui::Size;
            using gui::div;

            auto* root = tree::TreeStack::getCurrentTree()->getRoot();
            const size_t previousCount = root->children.size();

            div()
                .width(S::percent(1.0))
                .height(S::percent(1.0))
                .backgroundColor(simd_float4{0.04,0.04,0.05,1.0})
                .display(gui::Display::Grid)
                .gridTemplateColumns({S::fr(1.0), S::fr(1.0)})
                .gridTemplateRows({S::fr(1.0), S::fr(1.0)})
                .gridColumnGap(S::px(14))
                .gridRowGap(S::px(14))
                .padding(S::px(24))
            (
                div()
                    .backgroundColor(simd_float4{0.12,0.12,0.15,1.0})
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
                        .backgroundColor(simd_float4{0.95,0.18,0.22,1.0})
                        .minWidth(S::px(90))
                        .maxWidth(S::px(180))
                        .cornerRadius(S::px(5))
                    (),
                    div()
                        .width(S::px(140))
                        .height(S::percent(1.0))
                        .backgroundColor(simd_float4{0.10,0.72,0.95,1.0})
                        .minWidth(S::px(100))
                        .maxWidth(S::px(220))
                        .cornerRadius(S::px(5))
                    (),
                    div()
                        .height(S::percent(1.0))
                        .backgroundColor(simd_float4{0.95,0.84,0.16,1.0})
                        .flexGrow(S::px(1))
                        .minWidth(S::px(80))
                        .maxWidth(S::px(260))
                        .cornerRadius(S::px(5))
                    ()
                ),
                div()
                    .backgroundColor(simd_float4{0.12,0.12,0.15,1.0})
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
                        .backgroundColor(simd_float4{0.32,0.95,0.42,1.0})
                        .minHeight(S::px(42))
                        .maxHeight(S::px(90))
                        .cornerRadius(S::px(5))
                    (),
                    div()
                        .width(S::px(180))
                        .height(S::px(64))
                        .backgroundColor(simd_float4{0.68,0.28,0.96,1.0})
                        .minWidth(S::px(120))
                        .maxWidth(S::px(260))
                        .cornerRadius(S::px(5))
                    (),
                    div()
                        .width(S::percent(0.75))
                        .backgroundColor(simd_float4{1.00,0.48,0.12,1.0})
                        .flexGrow(S::px(1))
                        .minHeight(S::px(50))
                        .maxHeight(S::px(130))
                        .cornerRadius(S::px(5))
                    ()
                ),
                div()
                    .backgroundColor(simd_float4{0.12,0.12,0.15,1.0})
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
                        .backgroundColor(simd_float4{0.00,0.78,0.58,1.0})
                        .minWidth(S::px(90))
                        .minHeight(S::px(46))
                        .cornerRadius(S::px(5))
                    (),
                    div()
                        .backgroundColor(simd_float4{0.96,0.22,0.62,1.0})
                        .minWidth(S::px(130))
                        .maxWidth(S::px(220))
                        .minHeight(S::px(50))
                        .cornerRadius(S::px(5))
                    (),
                    div()
                        .backgroundColor(simd_float4{0.98,0.92,0.32,1.0})
                        .minWidth(S::px(80))
                        .maxWidth(S::px(180))
                        .minHeight(S::px(50))
                        .cornerRadius(S::px(5))
                    (),
                    div()
                        .backgroundColor(simd_float4{0.16,0.36,0.98,1.0})
                        .gridColumn(1, 3)
                        .minHeight(S::px(70))
                        .maxHeight(S::px(120))
                        .cornerRadius(S::px(5))
                    (),
                    div()
                        .backgroundColor(simd_float4{0.94,0.18,0.18,1.0})
                        .minWidth(S::px(90))
                        .minHeight(S::px(60))
                        .cornerRadius(S::px(5))
                    ()
                ),
                div()
                    .backgroundColor(simd_float4{0.12,0.12,0.15,1.0})
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
                        .backgroundColor(simd_float4{0.16,0.84,0.90,1.0})
                        .minWidth(S::px(110))
                        .maxWidth(S::px(220))
                        .cornerRadius(S::px(5))
                    (),
                    div()
                        .height(S::px(54))
                        .backgroundColor(simd_float4{0.54,0.28,0.98,1.0})
                        .flexGrow(S::px(1))
                        .minWidth(S::px(90))
                        .maxWidth(S::px(180))
                        .cornerRadius(S::px(5))
                    (),
                    div()
                        .width(S::px(150))
                        .height(S::px(54))
                        .backgroundColor(simd_float4{0.95,0.84,0.16,1.0})
                        .minWidth(S::px(120))
                        .maxWidth(S::px(210))
                        .cornerRadius(S::px(5))
                    (),
                    div()
                        .width(S::percent(0.65))
                        .height(S::px(54))
                        .backgroundColor(simd_float4{0.32,0.95,0.42,1.0})
                        .minWidth(S::px(160))
                        .maxWidth(S::px(300))
                        .cornerRadius(S::px(5))
                    ()
                )
            );

            return newestRootChild(previousCount);
        }

        tree::TreeNode* justifyItems() {
            using S = gui::Size;
            using gui::div;

            auto* root = tree::TreeStack::getCurrentTree()->getRoot();
            const size_t previousCount = root->children.size();

            div()
                .width(S::percent(1.0))
                .height(S::percent(1.0))
                .backgroundColor(simd_float4{0.04,0.04,0.05,1.0})
                .display(gui::Display::Grid)
                .gridTemplateColumns({S::fr(1.0), S::fr(1.0)})
                .gridTemplateRows({S::fr(1.0), S::fr(1.0)})
                .gridColumnGap(S::px(20))
                .gridRowGap(S::px(20))
                .padding(S::px(40))
            (
                div()
                    .backgroundColor(simd_float4{0.12,0.12,0.15,1.0})
                    .display(gui::Display::Grid)
                    .gridTemplateColumns({S::fr(1.0)})
                    .gridTemplateRows({S::fr(1.0)})
                    .justifyItems(gui::JustifyItems::Start)
                    .alignItems(gui::AlignItems::Center)
                    .padding(S::px(20))
                (
                    div()
                        .width(S::px(120))
                        .height(S::px(80))
                        .backgroundColor(simd_float4{0.95,0.18,0.22,1.0})
                        .cornerRadius(S::px(5))
                    ()
                ),
                div()
                    .backgroundColor(simd_float4{0.12,0.12,0.15,1.0})
                    .display(gui::Display::Grid)
                    .gridTemplateColumns({S::fr(1.0)})
                    .gridTemplateRows({S::fr(1.0)})
                    .justifyItems(gui::JustifyItems::Center)
                    .alignItems(gui::AlignItems::Center)
                    .padding(S::px(20))
                (
                    div()
                        .width(S::px(120))
                        .height(S::px(80))
                        .backgroundColor(simd_float4{0.32,0.95,0.42,1.0})
                        .cornerRadius(S::px(5))
                    ()
                ),
                div()
                    .backgroundColor(simd_float4{0.12,0.12,0.15,1.0})
                    .display(gui::Display::Grid)
                    .gridTemplateColumns({S::fr(1.0)})
                    .gridTemplateRows({S::fr(1.0)})
                    .justifyItems(gui::JustifyItems::End)
                    .alignItems(gui::AlignItems::Center)
                    .padding(S::px(20))
                (
                    div()
                        .width(S::px(120))
                        .height(S::px(80))
                        .justifySelf(gui::JustifySelf::Start)
                        .backgroundColor(simd_float4{0.10,0.72,0.95,1.0})
                        .cornerRadius(S::px(5))
                    ()
                ),
                div()
                    .backgroundColor(simd_float4{0.12,0.12,0.15,1.0})
                    .display(gui::Display::Grid)
                    .gridTemplateColumns({S::fr(1.0)})
                    .gridTemplateRows({S::fr(1.0)})
                    .justifyItems(gui::JustifyItems::Start)
                    .alignItems(gui::AlignItems::Center)
                    .padding(S::px(20))
                (
                    div()
                        .height(S::px(80))
                        .justifySelf(gui::JustifySelf::Stretch)
                        .backgroundColor(simd_float4{0.54,0.28,0.98,1.0})
                        .cornerRadius(S::px(5))
                    ()
                )
            );

            return newestRootChild(previousCount);
        }

        tree::TreeNode* complex() {
            using S = gui::Size;
            using gui::div;

            auto* root = tree::TreeStack::getCurrentTree()->getRoot();
            const size_t previousCount = root->children.size();

            div()
                .width(S::percent(1.0))
                .height(S::percent(1.0))
                .backgroundColor(simd_float4{0.04,0.04,0.05,1.0})
                .display(gui::Display::Grid)
                .gridTemplateColumns({S::px(260), S::percent(0.35), S::fr(1.0)})
                .gridTemplateRows({S::px(178), S::fr(1.0), S::px(148)})
                .gridColumnGap(S::px(14))
                .gridRowGap(S::px(14))
                .padding(S::px(22))
            (
                div()
                    .gridColumn(1, 4)
                    .gridRow(1, 2)
                    .backgroundColor(simd_float4{0.12,0.12,0.15,1.0})
                    .display(gui::Display::Flex)
                    .flexDirection(gui::FlexDirection::Row)
                    .flexGap(S::px(12))
                    .padding(S::px(12))
                    .minHeight(S::px(150))
                (
                    div().width(S::px(170)).height(S::percent(1.0)).minWidth(S::px(120)).maxWidth(S::px(220)).backgroundColor(simd_float4{0.00,0.78,0.58,1.0}).cornerRadius(S::px(5))(),
                    div().width(S::percent(0.28)).height(S::percent(1.0)).minWidth(S::px(180)).maxWidth(S::px(420)).backgroundColor(simd_float4{0.96,0.22,0.62,1.0}).cornerRadius(S::px(5))(),
                    div().height(S::percent(1.0)).flexGrow(S::px(1)).minWidth(S::px(180)).maxWidth(S::px(520)).backgroundColor(simd_float4{0.98,0.92,0.32,1.0}).cornerRadius(S::px(5))(),
                    div().width(S::px(140)).height(S::percent(1.0)).minWidth(S::px(110)).maxWidth(S::px(180)).backgroundColor(simd_float4{0.16,0.84,0.90,1.0}).cornerRadius(S::px(5))()
                ),
                div()
                    .gridColumn(1, 2)
                    .gridRow(2, 3)
                    .backgroundColor(simd_float4{0.12,0.12,0.15,1.0})
                    .display(gui::Display::Flex)
                    .flexDirection(gui::FlexDirection::Col)
                    .flexGap(S::px(10))
                    .padding(S::px(12))
                    .minWidth(S::px(220))
                    .minHeight(S::px(220))
                (
                    div().width(S::percent(1.0)).height(S::percent(0.22)).minHeight(S::px(46)).maxHeight(S::px(90)).backgroundColor(simd_float4{0.95,0.18,0.22,1.0}).cornerRadius(S::px(5))(),
                    div().width(S::px(155)).height(S::px(62)).minWidth(S::px(120)).maxWidth(S::px(210)).backgroundColor(simd_float4{0.10,0.72,0.95,1.0}).cornerRadius(S::px(5))(),
                    div().width(S::percent(0.72)).flexGrow(S::px(1)).minHeight(S::px(70)).maxHeight(S::px(180)).backgroundColor(simd_float4{1.00,0.48,0.12,1.0}).cornerRadius(S::px(5))(),
                    div().width(S::percent(0.45)).height(S::px(48)).minWidth(S::px(90)).maxWidth(S::px(150)).backgroundColor(simd_float4{0.54,0.28,0.98,1.0}).cornerRadius(S::px(5))()
                ),
                div()
                    .gridColumn(2, 3)
                    .gridRow(2, 3)
                    .backgroundColor(simd_float4{0.12,0.12,0.15,1.0})
                    .display(gui::Display::Grid)
                    .gridTemplateColumns({S::px(120), S::percent(0.45), S::fr(1.0)})
                    .gridTemplateRows({S::px(64), S::percent(0.5), S::fr(1.0)})
                    .gridColumnGap(S::px(10))
                    .gridRowGap(S::px(10))
                    .padding(S::px(12))
                    .minWidth(S::px(300))
                    .minHeight(S::px(220))
                (
                    div().backgroundColor(simd_float4{0.32,0.95,0.42,1.0}).minWidth(S::px(90)).minHeight(S::px(46)).cornerRadius(S::px(5))(),
                    div().backgroundColor(simd_float4{0.68,0.28,0.96,1.0}).minWidth(S::px(130)).maxWidth(S::px(240)).minHeight(S::px(50)).cornerRadius(S::px(5))(),
                    div().backgroundColor(simd_float4{0.98,0.92,0.32,1.0}).minWidth(S::px(80)).maxWidth(S::px(170)).minHeight(S::px(50)).cornerRadius(S::px(5))(),
                    div().gridColumn(1, 3).backgroundColor(simd_float4{0.16,0.36,0.98,1.0}).minHeight(S::px(74)).maxHeight(S::px(130)).cornerRadius(S::px(5))(),
                    div().backgroundColor(simd_float4{0.94,0.18,0.18,1.0}).minWidth(S::px(90)).minHeight(S::px(60)).cornerRadius(S::px(5))(),
                    div().gridColumn(1, 4).backgroundColor(simd_float4{0.16,0.84,0.90,1.0}).minHeight(S::px(46)).maxHeight(S::px(80)).cornerRadius(S::px(5))()
                ),
                div()
                    .gridColumn(3, 4)
                    .gridRow(2, 3)
                    .backgroundColor(simd_float4{0.12,0.12,0.15,1.0})
                    .display(gui::Display::Flex)
                    .flexDirection(gui::FlexDirection::Row)
                    .flexWrap(gui::FlexWrap::Wrap)
                    .flexGap(S::px(10))
                    .padding(S::px(12))
                    .minWidth(S::px(260))
                    .minHeight(S::px(220))
                (
                    div().width(S::percent(0.36)).height(S::px(58)).minWidth(S::px(110)).maxWidth(S::px(210)).backgroundColor(simd_float4{0.95,0.84,0.16,1.0}).cornerRadius(S::px(5))(),
                    div().height(S::px(58)).flexGrow(S::px(1)).minWidth(S::px(120)).maxWidth(S::px(240)).backgroundColor(simd_float4{0.00,0.78,0.58,1.0}).cornerRadius(S::px(5))(),
                    div().width(S::px(180)).height(S::px(58)).minWidth(S::px(140)).maxWidth(S::px(220)).backgroundColor(simd_float4{0.96,0.22,0.62,1.0}).cornerRadius(S::px(5))(),
                    div().width(S::percent(0.62)).height(S::px(58)).minWidth(S::px(180)).maxWidth(S::px(340)).backgroundColor(simd_float4{0.16,0.36,0.98,1.0}).cornerRadius(S::px(5))(),
                    div().height(S::px(58)).flexGrow(S::px(1)).minWidth(S::px(90)).maxWidth(S::px(160)).backgroundColor(simd_float4{1.00,0.48,0.12,1.0}).cornerRadius(S::px(5))()
                ),
                div()
                    .gridColumn(1, 4)
                    .gridRow(3, 4)
                    .backgroundColor(simd_float4{0.12,0.12,0.15,1.0})
                    .display(gui::Display::Grid)
                    .gridTemplateColumns({S::percent(0.25), S::px(220), S::fr(1.0), S::px(160)})
                    .gridTemplateRows({S::fr(1.0)})
                    .gridColumnGap(S::px(12))
                    .padding(S::px(12))
                    .minHeight(S::px(120))
                (
                    div().backgroundColor(simd_float4{0.68,0.28,0.96,1.0}).minWidth(S::px(120)).cornerRadius(S::px(5))(),
                    div().backgroundColor(simd_float4{0.10,0.72,0.95,1.0}).minWidth(S::px(160)).maxWidth(S::px(220)).cornerRadius(S::px(5))(),
                    div().backgroundColor(simd_float4{0.32,0.95,0.42,1.0}).minWidth(S::px(220)).cornerRadius(S::px(5))(),
                    div().backgroundColor(simd_float4{0.95,0.18,0.22,1.0}).minWidth(S::px(120)).cornerRadius(S::px(5))()
                )
            );

            return newestRootChild(previousCount);
        }

        tree::TreeNode* intrinsicSizing() {
            using S = gui::Size;
            using gui::div;
            using gui::text;

            auto* root = tree::TreeStack::getCurrentTree()->getRoot();
            const size_t previousCount = root->children.size();
            constexpr auto intrinsicText = "Intrinsic sizing chooses every soft break opportunity";

            div(S::percent(1.0), S::percent(1.0), simd_float4{0.059,0.071,0.090,1.0})
                .padding(S::px(32))
                .overflow(gui::Overflow::Scroll)
            (
                div(S::minContent(), S::autoSize(), simd_float4{0.078,0.722,0.859,1.0})
                (
                    elements::span()
                        .font(Arial)
                        .fontSize(S::pt(18))
                        .color(simd_float4{0.961,0.969,1.0,1.0})(text(intrinsicText))
                ),

                div(S::maxContent(), S::autoSize(), simd_float4{0.961,0.302,0.459,1.0})
                    .marginTop(S::px(20))
                (
                    elements::span()
                        .font(Arial)
                        .fontSize(S::pt(18))
                        .color(simd_float4{0.961,0.969,1.0,1.0})(text(intrinsicText))
                ),

                div(S::px(360), S::autoSize(), simd_float4{0.149,0.161,0.200,1.0})
                    .marginTop(S::px(20))
                (
                    div(S::fitContent(), S::autoSize(), simd_float4{0.980,0.761,0.200,1.0})
                    (
                        elements::span()
                            .font(Arial)
                            .fontSize(S::pt(18))
                            .color(simd_float4{0.078,0.090,0.110,1.0})(text(intrinsicText))
                    )
                ),

                div(S::px(120), S::autoSize(), simd_float4{0.380,0.922,0.561,1.0})
                    .minWidth(S::maxContent())
                    .marginTop(S::px(20))
                (
                    elements::span()
                        .font(Arial)
                        .fontSize(S::pt(18))
                        .color(simd_float4{0.078,0.090,0.110,1.0})(text(intrinsicText))
                ),

                div(S::px(640), S::autoSize(), simd_float4{0.639,0.420,0.961,1.0})
                    .maxWidth(S::minContent())
                    .marginTop(S::px(20))
                (
                    elements::span()
                        .font(Arial)
                        .fontSize(S::pt(18))
                        .color(simd_float4{0.961,0.969,1.0,1.0})(text(intrinsicText))
                )
            );

            return newestRootChild(previousCount);
        }

        tree::TreeNode* autoPercentWidth() {
            using S = gui::Size;
            using gui::div;

            auto* root = tree::TreeStack::getCurrentTree()->getRoot();
            const size_t previousCount = root->children.size();

            div(S::percent(1.0), S::percent(1.0), simd_float4{0.06,0.07,0.09,1.0})
            (
                div(S::autoSize(), S::autoSize(), simd_float4{0.08,0.72,0.86,1.0}) // blue
                    .position(gui::Position::Absolute)
                    .left(S::px(80))
                    .top(S::px(80))
                (
                    div(S::percent(0.5), S::px(120), simd_float4{0.96,0.30,0.46,1.0}) // pink
                    (
                        div(S::px(600), S::px(80), simd_float4{0.98,0.76,0.20,1.0})()
                    )
                )
            );

            return newestRootChild(previousCount);
        }

        tree::TreeNode* phaseAShrink() {
            using S = gui::Size;
            using gui::div;
            using gui::text;

            auto* root = tree::TreeStack::getCurrentTree()->getRoot();
            const size_t previousCount = root->children.size();

            div(S::percent(1.0), S::percent(1.0), simd_float4{0.06,0.07,0.09,1.0})
                .display(gui::Display::Flex)
                .flexDirection(gui::FlexDirection::Col)
                .alignItems(gui::AlignItems::FlexStart)
                .padding(S::px(48))
                .flexGap(S::px(28))
            (
                elements::span()
                    .font(Arial)
                    .fontSize(S::pt(24))
                    .color(simd_float4{0.96,0.97,1.0,1.0})(text("PHASE A: INDEFINITE PERCENT VS AUTO")),
                elements::span()
                    .font(Arial)
                    .fontSize(S::pt(14))
                    .color(simd_float4{0.70,0.73,0.80,1.0})(text("Compare each cyan percentage target and auto control with the browser reference")),

                elements::span()
                    .font(Arial)
                    .fontSize(S::pt(13))
                    .color(simd_float4{0.98,0.76,0.24,1.0})(text("INDEFINITE WIDTH — 60% TARGET VS AUTO CONTROL")),
                div().display(gui::Display::Flex).alignItems(gui::AlignItems::FlexStart).flexGap(S::px(48))
                (
                    div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).alignItems(gui::AlignItems::FlexStart).flexGap(S::px(8))
                    (
                        elements::span().font(Arial).fontSize(S::pt(12)).color(simd_float4{0.38,0.92,0.56,1.0})(text("AUTO CONTROL")),
                        div(S::autoSize(), S::px(112), simd_float4{0.15,0.16,0.20,1.0})
                            .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).padding(S::px(14))
                        (
                            div(S::autoSize(), S::px(76), simd_float4{0.08,0.72,0.86,1.0})
                                .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).padding(S::px(10)).flexGap(S::px(8))
                            (
                                div(S::px(180), S::px(56), simd_float4{0.98,0.76,0.20,1.0})(),
                                div(S::px(120), S::px(56), simd_float4{0.96,0.30,0.46,1.0})()
                            )
                        )
                    ),
                    div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).alignItems(gui::AlignItems::FlexStart).flexGap(S::px(8))
                    (
                        elements::span().font(Arial).fontSize(S::pt(12)).color(simd_float4{1.0,0.54,0.22,1.0})(text("60% TARGET")),
                        div(S::autoSize(), S::px(112), simd_float4{0.15,0.16,0.20,1.0})
                            .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).padding(S::px(14))
                        (
                            div(S::percent(0.60), S::px(76), simd_float4{0.08,0.72,0.86,1.0})
                                .display(gui::Display::Flex).alignItems(gui::AlignItems::Center).padding(S::px(10)).flexGap(S::px(8))
                            (
                                div(S::px(180), S::px(56), simd_float4{0.98,0.76,0.20,1.0})(),
                                div(S::px(120), S::px(56), simd_float4{0.96,0.30,0.46,1.0})()
                            )
                        )
                    )
                ),

                elements::span()
                    .font(Arial)
                    .fontSize(S::pt(13))
                    .color(simd_float4{0.98,0.76,0.24,1.0})(text("INDEFINITE HEIGHT — 50% TARGET VS AUTO CONTROL")),
                div().display(gui::Display::Flex).alignItems(gui::AlignItems::FlexStart).flexGap(S::px(48))
                (
                    div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(8))
                    (
                        elements::span().font(Arial).fontSize(S::pt(12)).color(simd_float4{0.38,0.92,0.56,1.0})(text("AUTO CONTROL")),
                        div(S::px(360), S::autoSize(), simd_float4{0.15,0.16,0.20,1.0})
                            .display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).alignItems(gui::AlignItems::Stretch).padding(S::px(14))
                        (
                            div(S::percent(1.0), S::autoSize(), simd_float4{0.08,0.72,0.86,1.0})
                                .display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).padding(S::px(10)).flexGap(S::px(8))
                            (
                                div(S::percent(1.0), S::px(72), simd_float4{0.98,0.76,0.20,1.0})(),
                                div(S::percent(1.0), S::px(48), simd_float4{0.96,0.30,0.46,1.0})()
                            )
                        )
                    ),
                    div().display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).flexGap(S::px(8))
                    (
                        elements::span().font(Arial).fontSize(S::pt(12)).color(simd_float4{1.0,0.54,0.22,1.0})(text("50% TARGET")),
                        div(S::px(360), S::autoSize(), simd_float4{0.15,0.16,0.20,1.0})
                            .display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).alignItems(gui::AlignItems::Stretch).padding(S::px(14))
                        (
                            div(S::percent(1.0), S::percent(0.50), simd_float4{0.08,0.72,0.86,1.0})
                                .display(gui::Display::Flex).flexDirection(gui::FlexDirection::Col).padding(S::px(10)).flexGap(S::px(8))
                            (
                                div(S::percent(1.0), S::px(72), simd_float4{0.98,0.76,0.20,1.0})(),
                                div(S::percent(1.0), S::px(48), simd_float4{0.96,0.30,0.46,1.0})()
                            )
                        )
                    )
                )
            );

            return newestRootChild(previousCount);
        }
    }

    tree::TreeNode* build(std::string_view name) {
        if (name == "music-player") return musicPlayer();
        if (name == "intrinsic-sizing") return intrinsicSizing();
        if (name == "auto-percent-width") return autoPercentWidth();
        if (name == "phase-a-shrink") return phaseAShrink();
        if (name == "sizing") return sizing();
        if (name == "justify-items") return justifyItems();
        if (name == "complex") return complex();
        return nullptr;
    }

    void buildBrowser() {
        using S = gui::Size;
        using gui::div;
        using gui::text;
        using runtime::Event;
        using runtime::EventType;

        constexpr float offscreen = 10000.0f;
        constexpr int leftArrow = 123;
        constexpr int rightArrow = 124;
        constexpr int downArrow = 125;
        constexpr int upArrow = 126;

        auto publishSelection = [](std::string_view name) {
            std::ofstream{"/tmp/gui-layout-current-scene"} << name;
        };

        auto browser = div()
            .position(gui::Position::Relative)
            .width(S::percent(1.0))
            .height(S::percent(1.0));

        std::vector<decltype(div())> panels;
        panels.reserve(sceneNames.size());

        for (size_t index = 0; index < sceneNames.size(); ++index) {
            auto panel = div()
                .position(gui::Position::Absolute)
                .left(S::px(index == 0 ? 0.0f : offscreen))
                .top(S::px(0))
                .width(S::percent(1.0))
                .height(S::percent(1.0));

            auto* scene = build(sceneNames[index]);
            decltype(panel)::reparent(panel.treeNode(), scene);
            browser(panel);
            panels.push_back(panel);
        }

        auto sceneName = text(std::string{sceneNames.front()});
        auto label = div()
            .position(gui::Position::Fixed)
            .top(S::px(8))
            .padding(S::px(6))
            .zIndex(std::numeric_limits<int64_t>::max())
            .backgroundColor(simd_float4{0.0, 0.0, 0.0, 0.82})
            .font(ArialBold)
            .fontSize(S::pt(12))
            .color(simd_float4{1.0, 1.0, 1.0, 1.0})
        (
            sceneName
        );
        browser(label);

        browser.addEventListener(EventType::KeyDown,
            [panels, sceneName, selected = size_t{0}](auto&, Event& event) mutable {
                const int keyCode = event.get<EventType::KeyDown>().keyCode;
                const bool previous = keyCode == leftArrow || keyCode == upArrow;
                const bool next = keyCode == rightArrow || keyCode == downArrow;
                if (!previous && !next) return;

                panels[selected].left(S::px(offscreen));
                selected = previous
                    ? (selected + panels.size() - 1) % panels.size()
                    : (selected + 1) % panels.size();
                panels[selected].left(S::px(0));

                sceneName.text(std::string{sceneNames[selected]});
                std::ofstream{"/tmp/gui-layout-current-scene"} << sceneNames[selected];
                event.stopPropagation();
                std::println("layout scene [{}/{}]: {}", selected + 1, panels.size(), sceneNames[selected]);
            }
        );

        publishSelection(sceneNames.front());
        std::println("layout scene [1/{}]: {}", sceneNames.size(), sceneNames.front());
    }

    std::span<const std::string_view> names() {
        return sceneNames;
    }
}

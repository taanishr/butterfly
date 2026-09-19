# Butterfly - A declarative, GPU-rendered UI library

## Synopsis:

Butterfly is a declarative, GPU-rendered UI library that aims to implement a *useful enough* subset of the the HTML/CSS standard.  The library itself loosely follows the CSS spec (it is a pain to read).

Butterfly is largely written in C++, with a few Swift bindings. It currently only supports MacOS, as it relies on Metal as its graphics api.

*Currently, there is support for:*

Four semantic elements:
- SVGs
- Images
- Divs
- Text (inline; span esque)
  
These display modes:
- Flexbox
- Grid
- Div
- Inline

These positioning mode:
- Static
- Relative
- Absolute
- Fixed
- Sticky

Many many many borders:
- Solid/double/dashed/dotted
- Rounded/elliptical borders

Three primary transforms:
- Scale/rotate/position

*A lot* of sizing operations:
- Px
- Pt
- Fr
- Percent
- Auto
- Aspect Ratio sizing
- And more...

Colors and opacity

Box shadows

Super smooth text, unicode support, bidi support

And a much more (with more to come too).

The declarative syntax is inspired by React. Here's an example from one of my border test cases:

```
    using S = gui::Size;
    using gui::Display;
    using gui::FlexDirection;
    using gui::AlignItems;
    using gui::JustifyContent;
    using gui::BorderStyle;
    using runtime::EventType;

    constexpr simd_float4 desk        {0.941, 0.937, 0.925, 1.0};
    constexpr simd_float4 paper       {1.000, 1.000, 1.000, 1.0};
    constexpr simd_float4 cream       {0.992, 0.973, 0.925, 1.0};
    constexpr simd_float4 ink         {0.129, 0.129, 0.153, 1.0};
    constexpr simd_float4 muted       {0.478, 0.478, 0.510, 1.0};
    constexpr simd_float4 rule        {0.871, 0.863, 0.839, 1.0};
    constexpr simd_float4 tangerine   {1.000, 0.502, 0.200, 1.0};
    constexpr simd_float4 grape       {0.435, 0.271, 0.831, 1.0};
    constexpr simd_float4 mint        {0.180, 0.741, 0.545, 1.0};
    constexpr simd_float4 slate       {0.239, 0.271, 0.325, 1.0};
    constexpr simd_float4 umbraSoft   {0.0, 0.0, 0.0, 0.16};

    constexpr auto butterflyPath = "/Users/treja/projects/gui/assets/butterfly.png";

    auto caption = [&](const char* title, const char* spec) {
        return div()
            .display(Display::Flex)
            .flexDirection(FlexDirection::Col)
            .flexGap(S::px(3))
        (
            text(title).font(ArialBold).fontSize(S::pt(12)).color(ink),
            text(spec).font(SFMono).fontSize(S::pt(9)).color(muted)
        );
    };

    auto cell = [&](auto&& subject, const char* title, const char* spec) {
        return div()
            .width(S::px(220))
            .display(Display::Flex)
            .flexDirection(FlexDirection::Col)
            .alignItems(AlignItems::Center)
            .flexGap(S::px(18))
            .paddingTop(S::px(24))
            .paddingBottom(S::px(12))
        (
            subject,
            caption(title, spec)
        );
    };

    auto swatch = [&](simd_float4 fill, float width, BorderStyle style, simd_float4 stroke = ink) {
        return div(S::px(140), S::px(100), fill)
            .borderWidth(S::px(width))
            .borderStyle(style)
            .borderColor(stroke);
    };

    div(S::percent(1.0), S::percent(1.0), desk)
        .padding(S::px(36))
        .overflow(gui::Overflow::Scroll)
    (
        div()
            .display(Display::Flex)
            .flexDirection(FlexDirection::Col)
            .flexGap(S::px(8))
            .marginBottom(S::px(28))
        (
            text("STROKES").font(DINAlternateBold).fontSize(S::pt(26)).color(ink),
            text("border-style on divs and images · click the last card to cycle its style")
                .font(SFMono).fontSize(S::pt(11)).color(muted)
        ),
        div().height(S::px(1)).color(rule)(),

        div()
            .display(Display::Flex)
            .flexWrap(gui::FlexWrap::Wrap)
            .flexGap(S::px(12))
            .justifyContent(JustifyContent::Center)
        (
            cell(swatch(paper, 1, BorderStyle::Solid).cornerRadius(S::px(12))(),
                "Solid hairline", "1 solid · r12"),
            cell(swatch(cream, 6, BorderStyle::Solid)(),
                "Solid thick", "6 solid · sharp"),
            cell(swatch(paper, 1, BorderStyle::Dashed)(),
                "Dashed hairline", "1 dashed · sharp"),
            cell(swatch(paper, 3, BorderStyle::Dashed).cornerRadius(S::px(12))(),
                "Dashed", "3 dashed · r12"),
            cell(swatch(cream, 6, BorderStyle::Dashed, grape).cornerRadius(S::px(50))(),
                "Dashed pill", "6 dashed · r50"),
            cell(swatch(paper, 3, BorderStyle::Dashed, tangerine).cornerRadius(S::percent(0.5))(),
                "Dashed ellipse", "3 dashed · r50%"),
            cell(swatch(paper, 4, BorderStyle::Dashed, slate)
                    .cornerRadiusTopLeft(S::px(48))
                    .cornerRadiusTopRight(S::px(4))
                    .cornerRadiusBottomRight(S::px(30))
                    .cornerRadiusBottomLeft(S::px(0))(),
                "Dashed uneven", "4 dashed · 48 4 30 0"),
            cell(swatch(paper, 2, BorderStyle::Dotted).cornerRadius(S::px(8))(),
                "Dotted", "2 dotted · r8"),
            cell(swatch(cream, 6, BorderStyle::Dotted, mint).cornerRadius(S::px(20))(),
                "Dotted thick", "6 dotted · r20"),
            cell(swatch(paper, 5, BorderStyle::Dotted, grape).cornerRadius(S::percent(0.5))(),
                "Dotted ellipse", "5 dotted · r50%"),
            cell(swatch(paper, 6, BorderStyle::Double).cornerRadius(S::px(10))(),
                "Double", "6 double · r10"),
            cell(swatch(cream, 3, BorderStyle::Double, slate)(),
                "Double thin", "3 double · sharp"),
            cell(swatch(paper, 3, BorderStyle::Dashed)
                    .cornerRadius(S::px(12))
                    .shadowOffsetY(S::px(8))
                    .shadowBlur(S::px(24))
                    .shadowColor(umbraSoft)(),
                "Dashed + shadow", "3 dashed · 0 8 24"),
            cell(image(butterflyPath, S::px(140), S::px(100))
                    .cornerRadius(S::px(12))
                    .borderWidth(S::px(4))
                    .borderStyle(BorderStyle::Dotted)
                    .borderColor(paper),
                "Image dotted", "4 dotted · r12"),
            cell(image(butterflyPath, S::px(140), S::px(100))
                    .cornerRadius(S::px(12))
                    .borderWidth(S::px(3))
                    .borderStyle(BorderStyle::Dashed)
                    .borderColor(ink),
                "Image dashed", "3 dashed · r12"),
            cell(swatch(mint, 4, BorderStyle::Solid, ink)
                    .cornerRadius(S::px(14))
                    .display(Display::Flex)
                    .alignItems(AlignItems::Center)
                    .justifyContent(JustifyContent::Center)
                    .addEventListener(EventType::Click, [](auto& node, Event&) {
                        switch (node.borderStyle()) {
                            case BorderStyle::Solid:  node.borderStyle(BorderStyle::Dashed); break;
                            case BorderStyle::Dashed: node.borderStyle(BorderStyle::Dotted); break;
                            case BorderStyle::Dotted: node.borderStyle(BorderStyle::Double); break;
                            case BorderStyle::Double: node.borderStyle(BorderStyle::Solid); break;
                        }
                    })
                (
                    text("click me").font(ArialBold).fontSize(S::pt(12)).color(paper)
                ),
                "Interactive", "click: solid → dashed → dotted → double")
        )
    );
```

I hope to launch bindings in some interpreted language one day; it's quite annoying to not have hot-swapping. And also the syntax is abhorrent for, you know, writing UIs.

## Building
I can tell you about every line of code in this codebase; except the CMake and build scripts. Those are totally vibe slopped. That being said, I've vibe slopped a few useful build scripts:

| Commands        | Description               |
| --------------- | --------------------------|
| `shio run`      | runs last compiled source |
| `ship build`    | builds                    |
| `ship buildrun` | builds and runs           |


Useful flags: \
`--debug-ui`: enables an inspect element esque debugger that looks sweet.

## Examples
Here's a few test harnesses I've had Claude build while testing features:

### Box Shadows
<img width="1469" height="836" alt="image" src="https://github.com/user-attachments/assets/fb12bb6c-d512-4194-9bce-38bb9bb2b7e7" />

### Transforms
<img width="1461" height="840" alt="image" src="https://github.com/user-attachments/assets/6aae8fc1-d4fa-4778-8b4c-fe9ed1edaa93" />

### Borders
<img width="1464" height="843" alt="image" src="https://github.com/user-attachments/assets/94353f3d-1d2d-43be-b3d2-02c92c09502c" />

### Scene I let it have fun with
*I let Claude choose whatever it wanted to built; it built a record collection. I don't know why. I wonder DJ Shadow got in the training dataset*
<img width="1469" height="842" alt="image" src="https://github.com/user-attachments/assets/ba4ce770-0bee-446d-9222-1051ee4934bf" />

# Future Work
I hope to add an animation subsystem too (lots of work), clean up some of the file structure, clean up some of my style conventions, and add more semantic elements. I also hope to release a series of blogs detailing the development process and some of the architectural decisions I made over time. This has been in the works for more than a year at this point (a year and two months).

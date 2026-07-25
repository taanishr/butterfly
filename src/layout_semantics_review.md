# Layout semantics review

## 1. Stale `siblingCursor`

**Classification: matters.**

Auto-height block layout initially creates `siblingCursor` using the provisional
height. `layoutRecursive` later replaces that height with `contentHeight`, but
does not update the cursor. The next sibling can therefore start too early.

## 2. Missing unresolved-height retry

**Classification: mostly intentional, with an absolute-positioning exception.**

For an ordinary in-flow percentage-height child inside an auto-height parent,
the percentage behaves as `auto`. Rerunning it with the parent's
content-derived height would be wrong.

Absolutely positioned descendants are different. They receive the positioned
parent's provisional containing-block height before that parent discovers its
final height. If their percentage height or vertical inset depends on it, they
need the final containing-block height. The current code never retries solely
because the parent height was unresolved, so that case can differ from browser
behavior.

References:

- [CSS 2.1 height rules](https://www.w3.org/TR/CSS2/visudet.html)
- [CSS Positioned Layout](https://drafts.csswg.org/css-position/)

## 3. Different absolute and fixed reference boxes

**Classification: does not matter.**

Normal flow, absolute positioning, and fixed positioning intentionally use
different reference boxes. That difference alone is not an inconsistency.

## 4. Inline detection and atomic replaced elements

**Classification: matters.**

The problem is using "has text" as a proxy for participation in inline layout.
The builders should check whether the element is inline. An inline image or SVG
should contribute one atomic fragment in `buildInlineBoxes` and
`buildIsolatedInlineBoxes`.

## 5. Inline margin sides

**Classification: localized bug.**

The margins are not added twice. The leading margin is selected from the wrong
side:

- LTR leading margin currently uses `right`; it should use `left`.
- RTL leading margin currently uses `left`; it should use `right`.

The trailing-margin code already uses the corresponding trailing side.

## 6. Multiline inline followed by a block

**Classification: likely real, but narrow.**

After an inline spans multiple lines:

- `siblingCursor.y` already contains the height of all completed lines.
- `prevInlineHeight` contains the height of all lines.
- The following block adds `prevInlineHeight` to `siblingCursor.y`.

This counts every line except the final line twice. It requires one inline child
to span multiple line boxes and then be followed by a block sibling in the same
traversal.

## 7. Final width with non-final height

**Classification: real mixed-axis consistency issue.**

This combination can occur in at least two flows:

- For a column flex intrinsic-main measurement, `layoutIntrinsicMain()` changes
  height to `MinContent` or `MaxContent` while leaving width unchanged, commonly
  `Final`.
- During row-flex phase B, stretching can change height to `Deferred` while
  leaving width `Final`.

If width triggers a retry, rerunning descendants can legitimately change height
because wrapping depends on width. Recomputing height is correct.

The inconsistency is that the post-retry code applies height min/max without
checking whether height resolution is `Final`, unlike the first pass. The
height recomputation should remain; final height clamping should retain the
`heightResolution == AxisResolution::Final` guard.

## Current priority

The meaningful normal-flow issues are 1, 5, and 6. Item 2 matters specifically
for absolutely positioned descendants. Item 4 is the inline-builder correction.
Item 7 is a mixed-axis consistency issue. Item 3 should be dropped.

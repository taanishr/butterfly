# Intrinsic sizing roadmap

## Deferred optimization: calculate min-content and max-content together

For normal block and inline intrinsic widths, min-content and max-content can be calculated during one recursive calculation. Both values come from the same content and break opportunities:

- For an inline sequence, max-content is the widest forced-break-separated line when optional breaks are not taken.
- For an inline sequence, min-content is the widest segment between permitted breaks.
- A single atom scan can accumulate both values.
- A normal vertical block container can combine the min-content and max-content contributions returned by its children.
- Atomic or replaced inline content contributes its corresponding intrinsic widths to both calculations.

This could replace separate min-content and max-content speculative layouts with one subtree visit that returns both values. Flex and grid would retain their own rules for combining the pair.

This does not eliminate the final layout of a flex or grid item. Those containers need intrinsic contributions before assigning the item's final size, so the first uncached calculation still requires:

1. One subtree visit calculating both intrinsic widths.
2. One subtree visit laying out the item at its assigned size.

Calculating the pair together is an optimization to revisit after the block/inline, flex, and grid behavior is correct. It is not part of the initial intrinsic-keyword implementation.

## Work order

### 1. Block and inline

- Finish `min-content`, `max-content`, and the bare `fit-content` sizing keyword for normal block and inline layout.
- Preserve the existing concrete layout and retry behavior.
- Add a focused case that can be rendered both by this library and by HTML/CSS, with visible and measurable results for each keyword.

### 2. Flex

- Review and consolidate redundant sizing paths.
- Implement flex-item blockification, including inline images and other inline elements participating as flex items.
- Integrate `min-content`, `max-content`, and `fit-content` with the refactored sizing flow.
- Once behavior is correct, revisit calculating min-content and max-content together.

### 3. Grid

- Follow the same sequence as flex: review the sizing paths, integrate intrinsic sizing behavior, and then consider the paired min/max calculation.
- Preserve grid's existing cleaner separation rather than forcing it through flex-specific abstractions.


# Why the agent's Open/Close designs were bad

Context: placing Open/Close edge fragments in visual order inside `reorderLineFragments`. Every design the agent produced was rejected. This file explains why each one was bad, so a future agent does not produce it again.

## The pattern behind all of them

- **Designing outward from edge cases instead of inward from one invariant.** Each case got its own mechanism: an empty span, a span opened at the end of a line, a Close whose Open is on an earlier line, nested spans, and siblings swapped by bidi.
- **Smuggling information across a boundary instead of using what the data already says.** Bidi levels were pushed into edges. Tree structure (parents, spans, owners) was pushed into a reorder function that should know only Open, Close, Text and Atomic. The flow is flat, so anything that needs the tree back is the wrong design.
- **Translating the spec's wording straight into an algorithm.** "The visually first fragment of the element" reads as a question about an element, so every design asked questions about elements and the tree.
- **Adding state per rule instead of finding the structure that makes the rule free.** This produced orphan lists, attachment lists, pending lists, owner and partner arrays, per-slot before/after lists, a `Span` struct, and helper lambdas. Each one was a symptom of not having a general rule.
- **Agreeing with a design before being able to run it on an example.** Several designs were "agreed" and then implemented as something else, because the agent could not actually execute the user's design in its head.

## The rejected designs

1. **Edges with bidi levels** (base level, magic 0/1, copying a neighbour's level, U+2060, U+FFFC). Edges are not text. Giving them levels makes the bidi algorithm responsible for box geometry it knows nothing about, and no level choice is correct when a right-to-left run crosses an element boundary.
2. **Let reversal move the edges, then fix them afterwards.** This places things wrong on purpose, and adds a pass whose only job is to undo the previous one.
3. **Edges follow their logically adjacent text.** Bidi moves text from outside an element into its middle. `<span>abc DEF</span> GHI`, with DEF and GHI right-to-left, shows as `abc IHG FED`. Adjacency puts `</span>` before IHG instead of after FED.
4. **A span stack inside reorder to know which span is open.** This rebuilds the tree in the bidi function. Across wrapped lines it also needs state carried from one line to the next.
5. **Pop edges off one stack in logical order.** Bidi can reorder siblings. `<a>abc</a><b>DEF</b><c>GHI</c>`, with DEF and GHI right-to-left, needs c's edges before b's, which no single logical-order stack yields.
6. **Count fragments per element, keyed by the innermost element.** An outer element's extent includes its children's text. In `<a>x <b>Y</b> Z</a>`, with Y and Z right-to-left, this closes a before Y.
7. **Fix 6 with parent indices** (a `parentElementIndex` field, parents on flow items, an `openElements` stack in the builder). This bolts the tree back onto a flat flow to patch one case.
8. **Orphan adoption for Closes whose Open is on an earlier line.** A special mechanism for one case instead of a general rule.
9. **Attach empty spans to the next or previous text, using pending and attached lists.** An element with no text on the line got its own bookkeeping instead of falling out of the same rule as everything else.
10. **Relabel Open as Close for rtl.** One label then means two different things: the logical start for line breaking, and the visual side for placement. The rtl swap is the push order.
11. **Duplicate kind state** (`FragmentKind` alongside `InlineFlowKind`, and an `isLeftEdge` bool). The kind and the push order already encode this.
12. **Write the visual order back into `context.fragments`.** This breaks the `childFragments` ranges, which depend on logical order.
13. **A min/max range stack with a permanent line-start entry, emitted through per-slot `before`/`after` lists and an `unplaced` list.** This adds a second ordering structure layered on the first, plus a separate list for edges with no text.

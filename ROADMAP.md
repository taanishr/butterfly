# Roadmap

## 1. Text Correctness

Goal: text should behave predictably enough to build real UI.

1. Fix hard newlines.
   - Make multiline text render reliably.
   - Route `Atom::placeOnNewLine` through inline text layout cleanly.

2. Add explicit line metrics.
   - `lineHeight(...)`
   - Default line height from font size.
   - Line boxes should use line metrics instead of tallest-glyph behavior.

3. Add explicit whitespace modes.
   - `whiteSpace: Normal | NoWrap | Pre | PreWrap`
   - `wordBreak: Normal | BreakAll`

4. Add text overflow.
   - Support clipping and a configurable overflow ending.

5. Add text alignment.
   - `textAlign: Start | Left | Center | Right`

6. Improve text shaping coverage.
   - Introduce shaped glyph runs.
   - Support glyph-id lookup instead of codepoint-only lookup.
   - Store per-glyph advances and offsets.
   - Add kerning.
   - Add ligatures.
   - Add complex script shaping.
   - Add bidirectional text handling.

### Text Backlog

- Consider multi-line clamping if a concrete UI use case requires it.
- Apply UAX #9 rule L1 after line wrapping so trailing whitespace uses the paragraph base level before visual fragment reordering.

## 2. Optimization

Goal: make the library fast and efficient under real app workloads.

Targets:
- Keep frame times under 16ms for at least 60fps rendering.
- Minimize cache misses.
- Improve vectorization.
- Use threading where it meaningfully reduces frame time or latency.
- Avoid unnecessarily wide instructions when narrower work is faster or more power efficient.
- Reduce idle CPU usage.
- Reduce RAM usage with many static images/SVGs loaded.
- Reduce tree overhead, especially where it causes cache misses or high CPU usage.

1. Audit dirty-bit precision.
   - Verify each builder mutation dirties the minimum required phases.
   - Separate topology changes from layout/style/text mutations.
   - Preserve current targeted paint invalidation for z-index/color-like changes.
   - Avoid root/subtree dirtying unless the mutation actually requires it.

2. Tighten constraints caching.
   - Audit `ConstraintsKey` coverage.
   - Avoid recomputing phases when incoming constraints and dirty bits are unchanged.

3. Validate render-order invalidation.
   - Rebuild render order only on topology or paint-order changes.
   - Ensure global z-index recalculation happens whenever paint order needs it.
   - Keep ancestor-aware ordering deterministic.

4. Unify hit-test paths.
   - Keep early-return hit testing and collecting hit testing consistent.
   - Avoid per-call child sorting where cached render order is sufficient.
   - Add subtree pruning by bounds if hit testing becomes a bottleneck.

5. Reduce layout allocations.
   - Reuse `lineFragments`, `lineBoxes`, atom offset buffers.
   - Reserve common vector sizes.
   - Avoid rebuilding temporary vectors in hot paths.

6. Optimize overflow scroll.
   - Recompute scroll content size only when descendant layout changes.
   - Keep scroll offset changes out of measure/layout when geometry is unchanged.

7. Improve text/glyph cache behavior.
   - Avoid repeated glyph-run work for unchanged text descriptors.
   - Keep glyph buffer growth predictable.
   - Reuse text placement buffers where possible.

8. Reduce idle CPU/GPU work.
   - Preserve the existing `RenderTree::update` early return for clean trees.
   - Avoid creating command buffers / render encoders / presents when no redraw is needed.
   - Audit MTK view scheduling so static UI does not continuously redraw.
   - Replace or tighten the resize watcher polling path if it burns idle CPU.

9. Add static asset resource caching.
   - Deduplicate images/SVGs by path or content key.
   - Share CPU-side decoded data and GPU-side resources where possible.
   - Track resource lifetime and release unused static assets.
   - Avoid loading the same static asset repeatedly across nodes.

## 3. Debugging Tools

Goal: make layout/render bugs visible.

1. Renderer stats panel.
   - Frame time.
   - Per-phase timing.
   - Nodes measured/atomized/laid out/placed/finalized.
   - Render order cache hit/miss.
   - Hit-test count.

2. Dirty diagnostics.
   - Show which node dirtied.
   - Show which phases dirtied.
   - Track dirty reasons at mutation sites.
   - Explain cache misses when a phase recomputes.

3. Selected node overlay.
   - Draw content box.
   - Draw padding box.
   - Draw margin box.
   - Show node id/type label near box.

4. Hit stack panel.
   - List all hit nodes, not just `hit: n / total`.
   - Selected row highlighted.
   - Shift+Up/Down moves selection.

5. Dirty phase panel.
   - Per selected node: measure/atomize/layout/postLayout/place/finalize/paint.
   - Show last dirtied reason.

6. Render order panel.
   - Selected node global z.
   - Previous/next render neighbors.
   - Parent/child paint relationship.

7. Tree view.
   - Collapsible tree.
   - Selected node highlighted.
   - Show `Element#id`.
   - Search by id.

8. Event view.
   - Recent events.
   - Focused node.
   - Hovered node.
   - Mouse down target.
   - Listeners attached to selected node.

## 4. Layout / UI Features

Goal: expand the engine toward practical app UI.

### Sequencing

Ordered by dependency, not by the feature groups below. Tier 0 is the weekend scope.

```text
Tier 0  sticky, per-corner radius, linear gradient, hover/active/focus-visible
Tier 1  paint pass (paint bounds + derived paint geometry) -> box shadow, dashed borders
Tier 2  stacking contexts
Tier 3  transforms (matrix + clip inverses + hit test + origin) as one change
Tier 4  animate transform/opacity; then animate position/size, gated
Tier 5  frame-persistent layout cache + dependency-narrowed keys
```

Rationale for the reordering:
- Transforms are cheaper than they look and opacity is more expensive; do transforms first.
- Opacity and transforms both create stacking contexts, so stacking contexts precede both.
- Box shadow and dashed borders are the same underlying feature (post-layout derived paint geometry), so the paint pass precedes both.

### 1. Better sizing

Done (sizing refactor).
- `fit-content`
- `min-content`
- `max-content`
- `aspectRatio(...)`

### 2. Better positioning

- Refactor sizing requests.
  - Open: decide whether this is subsumed by the sizing refactor, or whether it means
    collapsing the three `std::optional<...RequestOverride>` params on `layoutRecursive`.
- `position: sticky`.
  - Postlayout-only. In-flow, so sizing is untouched; no reflow of siblings.
  - Model as: relative offset against the nearest scrollport, clamped to the containing
    block's box. Not "relative + fixed".
  - `postLayoutPhase` already shifts origins by `scrollOffset`, and `makeConstraintsKey`
    already folds origins into the key, so scroll invalidation is free.
  - Build it as a general postlayout position adjustment driven by a dependency on
    another node's resolved box, not as a hardcoded reach for `scrollOffset`. Anchored
    popovers (see 7) are the same mechanism with a different input.

### 3. Better visual styling

- Per-corner radius.
  - Promoted. CSS does support it (four corners, two radii each, plus the `/` form).
    Needed for tabs, chat bubbles, segmented controls, rounded first/last table rows.
  - `cornerRadius` is already `float2`, so elliptical radii are already the concept.
    Go to `float2 r[4]`, select by quadrant sign of `localPosition` before the SDF.
    ~5 lines in `common.metal`. Same change applies to `ClipUniform`.
- Linear gradients.
  - Pure shader change: replace `uniforms->style.color` with a function of
    `localPosition`. Stops + angle into `DivStyleUniforms`.
- Opacity.
  - Ship inherit + multiply (per-primitive alpha). Accepted approximation.
  - It is NOT group opacity: overlapping descendants compound, so a fading modal
    dissolves unevenly instead of fading. Equivalent to correct group opacity only
    when the subtree paints at most one node.
  - Document the semantics, or name it so it doesn't promise CSS behavior.
  - Upgrade path if ever needed: allocate an offscreen only for opacity nodes whose
    subtree paints more than once. Subtree paint count is O(1) from
    `paintPreorderIndex`/`paintPostorderIndex`. Same public API, no rework.
- Box shadow.
  - Not just a shader change: the shadow extends outside `halfExtent`, so it needs
    paint bounds distinct from layout bounds. See the paint pass below.
  - Shader side is small (rounded-rect SDF smoothstepped by blur). Draw shadow and
    fill in the same fragment shader on the expanded quad to keep one draw per node.
  - Inset is a separate branch; multiple shadows means a loop.
- Border styles: solid/dashed.
  - Harder than it looks. Even spacing around a rounded rect needs arc-length
    parameterization, which the SDF does not provide, and elliptical corners have no
    closed-form arc length.
  - Browsers stroke per-side paths with an arc-length dash effect and fit the dash
    period per side so a whole number of dashes lands on each side. Dash fitting is
    not optional; it's why browser corners don't show clipped half-dashes.
  - Plan: generate dash geometry on the CPU (arc length and fitting are easy there),
    but keep the SDF for shape. Generated quads decide *where along the perimeter* a
    dash is; the existing `borderMask = outerMask - innerMask` decides what shape it
    is. Corner dashes cut radially from the corner center, which is a straight line,
    so a quad or small fan works.
  - Generate at finalize, not atomize — see the paint pass below.
- Background image.
  - Skip, or ship as a docs recipe. An absolutely-positioned inset-0 image child under
    a clipping parent already gives cover/contain, corner-radius clipping, and
    non-participation in layout, using machinery that exists.

### 3a. Paint pass (new)

One category, not three features: **post-layout derived paint geometry**. Takes resolved
boxes, emits geometry and bounds that never feed back into layout.

- Consumers: box shadow quads, dash quads, outline / focus rings (outside the border
  box), transformed paint bounds, `overflow: visible` contribution to scrollable
  overflow. Downstream: damage rects, culling.
- Must run after layout, not at atomize. Glyph atoms are size-independent; dash and
  shadow geometry are functions of the resolved box, so emitting them at atomize forces
  placeholder atoms rewritten in postlayout. That rewrite is a phase-ordering smell.
- `finalizePhase` already takes `(constraints, shared, measured, atomized, layout,
  placed)` and produces render-ready state, so it is the natural home. A separate pass
  is worth it mainly because transforms need the same postorder accumulation.
- Paint bounds is a bottom-up union: own ink bounds ∪ children's paint bounds, then
  transformed into parent space.
- Invalidation: `DirtyBits::Finalize`, not `Layout`. Dashes and shadows then cost
  nothing on layout-only invalidations.

### 4. Transforms

Do as one change, not four. Blast radius is well-defined: vertex shader, clip uniforms,
postlayout accumulation, hit test.

- Translate / scale / rotate.
  - Geometry is trivial: the vertex shader already indirects through
    `offsets[in.atom_id]`. Add a per-node 2x3 and multiply before `toNDC`.
  - Pass the *pre-transform* position as `worldPosition` and transform only
    `out.position`. The SDF, corner radii, and `fwidth` AA then evaluate in
    untransformed local space, so rotated rounded rects with borders come out
    analytically correct and correctly antialiased. Strictly better than
    offscreen-then-transform, which resamples.
- Clipping is the real cost.
  - `ClipUniform` is world-space and `outside_clips` evaluates in world space, so a
    transformed ancestor's clip is no longer axis-aligned.
  - Each clip gains an inverse (2x3). Transform the point before the SDF. Propagate
    accumulated inverses down postlayout. The `sizeof(ClipUniform) * 4` allocation
    grows.
- Transformed hit testing.
  - Same accumulated inverses as the clips. `hitTestRecursive` transforms the point
    into local space as it descends. Build the data once, use it in both places.
- Transform origin.
  - `translate(o) · M · translate(-o)`. Origin-from-padding-edge is already settled.
- Transforms do not affect layout and do not need offscreen targets, so they are
  independent of the opacity question.

### 5. Interaction primitives

- Hover state.
- Active/pressed state.
- Focus-visible state.
  - Needs keyboard-vs-mouse activation threaded from the platform layer.
- Policy: gate hover/active styles to paint-only properties, marking `Finalize` dirty
  only. If hover can change layout-affecting properties, every mouse move can trigger
  relayout.
- Tab navigation.
  - Tree-order problem, not a Cocoa problem. Sequential focus order is preorder filtered
    by focusability, so `paintPreorderIndex` is already the tab order.
  - The hard half is focus trapping inside portals, which couples this to 7.
- Keyboard shortcut normalization.

### 6. Layout transitions

Retained-tree identity already exists (`elementTree` persists with dirty bits, not
rebuilt per frame) and `requiresFrame` is already the continuous-redraw hook. Mechanism
is: store start/target/curve on the node, lerp at place time.

- Animate transform.
- Animate opacity.
  - These two are nearly free — interpolate a uniform, zero layout involvement. This is
    why the web platform treats them as the compositor-only properties. They cover most
    real UI motion. Do these first.
- Animate position changes.
- Animate size changes.
  - The perf cliff: animating a parent's size relayouts the subtree every frame.
    Mitigated by the cache work in 7a, not by animation code.
- Interruption / retargeting mid-flight is the genuinely fiddly part.

### 7. Portals / overlay layer

- Ensure overlay z-order is not ad hoc.
  - Current sort compares a flat `globalZIndex`, then tree order with an ancestor check.
    That's the non-spec shortcut: real stacking contexts *scope* descendant z-indices so
    they can't escape to compete globally.
  - Bites exactly when a portal inside a z-indexed card must rise above a sibling card,
    i.e. the tooltip case.
  - Opacity and transforms both create stacking contexts, so do this before 3's opacity
    and before 4.
- Tooltip / popover / modal / context menu.
  - Deferred. These are thin wrappers over one primitive that is currently missing:
    anchored placement with collision handling (the Floating UI / Popper pipeline —
    placement, flip on viewport overflow, shift along the cross axis, clamp max size to
    available space, arrow positioning, re-track on scroll). No browser provides it;
    every UI library implements it in userland.
  - It can live in userland provided the engine exposes a node's resolved global box and
    its effective clip rect, plus a hook to re-run placement when either changes. That
    hook is the same dependency mechanism as sticky (see 2).
  - Build the primitive once or implement flip-and-shift four times badly.

### 7a. Caching rethink (prerequisite for size animation)

`layoutCache` and `sizeCache` are wiped at the top of every layout pass. They are
intra-pass memo tables — they exist so flex Phase D and grid don't re-derive the same
subtree within one pass. Frame-persistent caching is a different thing built alongside,
not a tweak to these.

- Narrow the keys to actual dependencies.
  - `makeConstraintsKey` hashes the whole `Constraints` blob regardless of what layout
    read, so a node with explicit width/height gets a key that changes when the parent's
    available size changes, even though its result cannot depend on that.
  - Record which constraint fields a node's layout actually consulted (a small bitmask
    per node is enough; no per-access instrumentation needed) and key on the read set.
  - Payoff: fixed-size subtrees become immune to a parent animating, which turns "size
    animation is expensive" into "size animation is expensive only for subtrees that
    actually depend on size". That's the correct cost model and a better thing to
    document than a blanket warning.
  - Correctness trap: a read set recorded from one branch can under-report for another,
    causing stale hits. Union read sets across runs rather than replacing, so the key
    only ever gets more sensitive, never less.
- Eviction without hitching.
  - No mark-sweep. Stamp entries with `lastUsedGeneration`, give the cache a budget, and
    evict a bounded slice per frame (round-robin over a fixed number of buckets at frame
    end). Constant work per frame, no pause.
  - Safe to be imprecise: entries are independently droppable and a wrongly-evicted
    entry is a recompute, not a bug.
- Keep both tiers. The per-pass table has a different access pattern (high hit rate,
  guaranteed dead at pass end, no eviction needed) than a cross-frame cache; merging
  them makes both worse. The pass table clears, the frame cache ages out.

## 5. Platform / Backend Split

Goal: prepare the engine for Metal and Vulkan backends without mixing rendering with platform services.

1. Define a render backend boundary.
   - Device.
   - Swapchain/drawable.
   - Command buffer.
   - Buffers.
   - Textures.
   - Pipelines.
   - Shader/library loading.

2. Define a platform backend boundary.
   - Window creation.
   - Event pump.
   - Keyboard/mouse input.
   - Clipboard.
   - Cursor.
   - File dialogs.
   - Font discovery.
   - Accessibility bridge.

3. Move AppKit-specific code behind platform services.
   - Keep Objective-C runtime hooks out of core renderer code.
   - Convert native events into backend-neutral runtime events.

4. Move Metal-specific code behind render services.
   - Keep Metal resource types out of high-level UI code.
   - Preserve backend-neutral element/layout APIs.

5. Add Vulkan backend prototype.
   - Buffer abstraction.
   - Pipeline abstraction.
   - Shader translation/build path.
   - Swapchain presentation.

## 6. Resize / Multithreaded Animation

Goal: resize should feel browser-smooth.

1. Separate live tree mutation from render snapshots.
   - UI tree mutates.
   - Renderer consumes immutable snapshot.

2. Double-buffer snapshots.
   - Current snapshot displayed.
   - Next snapshot computed.

3. Add resize scheduler.
   - Resize event updates target frame size.
   - Renderer keeps presenting current valid frame.
   - Layout thread computes next snapshot.

4. Add interpolation.
   - Keep previous layout boxes.
   - Interpolate to next layout boxes.
   - Use interpolation only for resize/layout transitions.

5. Move layout prep off the render thread.
   - Measure/layout/postLayout on worker.
   - Render thread only encodes stable snapshot.

6. Add synchronization.
   - Atomic snapshot swap.
   - No partial tree reads.
   - No mutation while render snapshot is active.

7. Add fallback behavior.
   - If next snapshot is late, keep presenting previous.
   - If resize ends, snap to final computed layout.

## Order

```text
1. Text correctness
2. Optimization
3. Debugging tools
4. Layout/UI feature expansion
5. Platform/backend split
6. Resize + multithreaded animation
```

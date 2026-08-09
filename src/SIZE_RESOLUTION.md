# Size Resolution Refactor

## Status and authority

This document is the implementation plan for the sizing refactor. It replaces
the earlier intermediate plan built around `resolveRequestedSize()` and
`resolveContentSize()`.

When implementation details become unclear, recover from the invariants and
flow in this document. Do not preserve intermediate code merely because it
already exists. Do not introduce a new sizing state, helper, or formatting-mode
special case unless the existing sizing behavior cannot be represented by this
design; if that happens, stop and update the design before coding around it.

Restart baseline: `88810de` (`test infra complete`). None of the sizing-refactor
implementation currently present on `sizing-refactor` is approved for reuse.
The test suite from that commit and this document are the only artifacts to
carry onto the restart branch.

## Goal

The final boundary is:

```text
formatting algorithm
    -> supplies physical constraints and a SizeRequest
    -> RenderTree evaluates the request
    -> RenderTree performs content layout and reevaluates when necessary
    -> formatting algorithm receives numeric sizing results
    -> formatting algorithm stores and distributes those numbers
```

Two invariants define the refactor:

1. Flex, grid, block, inline, and other formatting algorithms do not make exact
   sizing decisions.
2. Size evaluation knows nothing about flex lines, grid tracks or spans, block
   stacking, inline placement, or any other algorithm bookkeeping.

Formatting algorithms may retain state such as flex base size, hypothetical
size, track contributions, allocated size, and placement. They may also use the
existing stateful flex `AxisHelper` to map physical width and height into main
and cross dimensions. They must not inspect style sizing units or resolution
failures to calculate those values.

## Non-negotiable representation rules

- `Measured` is the immutable result of the measurement phase.
- Processors continue to return `Measured`.
- A copied `Measured` is never mutated to request a different sizing pass.
- Width and height are the sizing vocabulary. Do not introduce another axis
  abstraction. The existing flex `AxisHelper` remains limited to flex's
  stateful row/column mapping.
- `SizeRequest` contains facts and instructions for one sizing invocation. It
  does not identify flex, grid, block, or inline layout.
- `SizeResolution` contains physical sizing results. It does not contain flex
  bases, grid contributions, tracks, lines, spans, or placement state.
- Intrinsic work is optional and is performed only when requested or required
  to evaluate the current request.
- An unresolved width or height is valid. It is represented by the existing
  optional dimension, not by a synthetic size, a deferred-size enum, or a
  formatting-specific failure state.
- Do not add one-line wrappers that merely rename field access or an existing
  expression.
- Do not add separate normal-flow, flex, or grid sizing resolvers.

## Discovered request/constraint ownership problem

The abandoned implementation added `SizeRequest` as a parameter to
`layoutRecursive()`, `layoutPhase()`, and `speculateLayout()`. Parent formatting
paths then constructed and propagated sizing requests for children. This
conflated a request with a constraint. A child must construct its own sizing
request. Parent-imposed concrete size overrides belong in `Constraints` as the
existing `SizeOverride` representation. The parent/child mechanism for asking
for intrinsic min-content or max-content results remains unresolved and must be
designed before implementation resumes.

## Core types

`Measured` remains:

```cpp
struct MeasuredSize {
    style::Size specified;
    std::optional<float> resolved;
    bool explicitSize{};
};

struct Measured {
    FragmentID id;
    MeasuredSize width;
    MeasuredSize height;
};
```

Its fields mean only:

- `specified`: the element's original requested size;
- `resolved`: the initial pre-layout resolution, when one exists;
- `explicitSize`: the explicitness determination made during measurement.

The per-invocation request becomes:

```cpp
struct ContentSize {
    float width;
    float height;
};

struct SizeRequest {
    SizeOverride override;

    std::optional<float> stretchWidth;
    std::optional<float> stretchHeight;

    std::optional<ContentSize> content;

    bool widthBounds{};
    bool heightBounds{};

    bool widthIntrinsicSizes{};
    bool heightIntrinsicSizes{};
};
```

The meanings are:

- `override`: a concrete size already selected by a parent algorithm;
- `stretchWidth` / `stretchHeight`: a concrete stretch opportunity. The size
  evaluator, not the caller, decides whether the element accepts it;
- `content`: content extents produced by layout and supplied for reevaluation;
- `widthIntrinsicSizes` / `heightIntrinsicSizes`: request that the corresponding
  intrinsic contributions be returned.

An override and a stretch opportunity are different. An override is already
authoritative. Stretch is only an offered size and remains subject to the
element's sizing rules.

The result remains physical and may be incomplete:

```cpp
struct SizeResolution {
    ResolvedSize size;
    ResolvedSize minimum;
    ResolvedSize maximum;

    std::optional<IntrinsicSizes> widthIntrinsicSizes;
    std::optional<IntrinsicSizes> heightIntrinsicSizes;
};
```

`minimum` and `maximum` contain the concrete physical bounds currently
available to formatting algorithms. Formatting algorithms may store those
bounds for later arithmetic, but do not resolve them themselves.

If migration proves that an algorithm needs another generic physical numeric
result that cannot be obtained from this structure, extend `SizeResolution`
only with that generic result. Do not add formatting-algorithm terminology to
it, and do not make flex or grid reconstruct the value from style state.

## Central operation

There is one sizing operation:

```cpp
SizeResolution RenderTree::evaluateSizeRequest(
    TreeNode* node,
    const Constraints& constraints,
    const SizeRequest& request
);
```

`TreeNode` supplies the element's style and immutable measured result.
`Constraints` supplies the physical environment: available width and height,
containing block, resolved margins, position inputs, and inherited layout
facts. `SizeRequest` supplies facts unique to the current invocation.

The evaluator owns all interpretation of:

- initial measured resolution;
- percentage resolution against the current physical basis;
- absolute inset sizing;
- concrete overrides;
- whether an offered stretch size is accepted;
- auto and content-derived sizing;
- intrinsic preferred sizes;
- intrinsic min/max bounds;
- automatic minimum sizing;
- ordinary min/max bounds;
- aspect-ratio transfer and the subsequent bound correction;
- normalization of requested intrinsic contributions.

The evaluator does not own child traversal or formatting-algorithm execution.
Those produce content and intrinsic facts. `RenderTree` supplies those facts to
the evaluator through the same request boundary.

## Unresolved-size protocol

Evaluation may return an unresolved width or height. The formatting algorithm
does not inspect the reason and does not select a fallback.

The generic `layoutRecursive()` orchestration owns recovery:

```text
evaluate request
    -> lay out using the dimensions currently available
    -> calculate content width and height
    -> if a dimension was unresolved, add ContentSize to the same request
    -> evaluate the same request again
    -> repeat child layout only when the resulting concrete box changed
    -> if a dimension remains unresolved, propagate it
```

This is a single unresolved check in `RenderTree`. It is not reproduced in
normal flow, flex, grid, or inline code. Reevaluation is driven by the one set
of content facts produced by layout; this plan does not introduce a generic
stabilization loop or speculative cycle machinery.

An operation that can proceed with an unresolved dimension receives the
optional result unchanged. An algorithmic step that ultimately needs a number
is reached only after the generic orchestration has had the opportunity to
produce content and reevaluate. If the dimension still cannot resolve, it is
propagated rather than replaced by a caller-invented number.

A formatting algorithm may still calculate the contributions that supply the
next reevaluation. It must not turn those contributions into a resolved
available size itself. In particular, an unresolved flex pass may gather item
sizes, form its content and intrinsic contributions, and return them; free-space
distribution waits until reevaluation supplies a concrete container main size.

## Ownership after migration

### RenderTree and size evaluation

Own:

- interpreting all element sizing styles;
- deciding whether a numeric dimension is available;
- obtaining intrinsic measurements when required;
- normalizing returned intrinsic contributions;
- applying bounds and aspect ratio;
- the unresolved -> content layout -> reevaluation protocol;
- caching `SizeRequest`-dependent speculative layouts.

### Formatting algorithms

Own:

- constructing physical constraints and `SizeRequest` facts;
- flex line formation, free-space distribution, freezing, and placement;
- grid placement, track and span accounting, fraction distribution, and
  placement;
- normal-flow child traversal and content extents;
- inline formatting and line construction;
- storing numeric results returned by the sizing boundary.

Do not own:

- checks of `Size::unit`, `isAuto()`, or `isContentDependent()` for element
  sizing;
- direct calls to `resolveSpecifiedSize()`, `resolveIntrinsicSize()`, or
  `applyMinMax()` for element sizing;
- deciding whether percentages are definite;
- deciding whether intrinsic measurement is necessary;
- deciding whether auto accepts stretch or content sizing;
- automatic-minimum rules;
- aspect-ratio transfer;
- mutation of sizing fields in `Constraints` or `Measured` to provoke a pass.

Grid track definitions and flex container properties remain part of their own
algorithms. The prohibition applies to resolving an element's width, height,
min/max bounds, stretch acceptance, and intrinsic/content dependency.

## Mapping current code to the final boundary

### `render_tree.cpp`

Replace `resolveRequestedSize()` and `resolveContentSize()` with
`evaluateSizeRequest()`.

Move into the evaluator, without preserving their current proxy conditions:

- override precedence;
- percentage reevaluation;
- auto fill/stretch acceptance;
- intrinsic preferred sizes and bounds;
- content fallback;
- min/max bounds;
- aspect ratio.

Keep child traversal and calculation of `contentWidth` / `contentHeight` in
`layoutRecursive()`. Replace the current retry override with `request.content`
and reevaluate the same request. A content-derived size is not represented as a
parent override.

Replace `intrinsicSizesAxis` checks with physical intrinsic-result requests in
`SizeRequest`. Width and height intrinsic results may both exist on the same
`LayoutOutput`; no axis selector is required.

Remove the mutation in `measureIntrinsicSizes()` that resets
`measured.width.resolved` or `measured.height.resolved`. Intrinsic calculation
must be expressed by the request being evaluated.

Remove the final `specifiedSizeContributes` reconstruction. Contribution
normalization belongs to central size evaluation and uses immutable measured
facts directly.

### `flex.cpp`

The flex algorithm continues to store and use:

- flex base size;
- minimum and maximum main sizes;
- hypothetical main and cross sizes;
- distributed main sizes;
- line cross sizes;
- final placements.

Replace the sizing interpretation that currently produces those numbers:

- `layoutIntrinsicMain()` and its copied-`Measured` mutation;
- direct resolution of main, min-main, and max-main requests;
- `needsIntrinsicMinimum` and `needsIntrinsicMain`;
- `determineFlexBaseSize()`, `determineMinMainSize()`, and
  `determineMaxMainSize()` when their inputs are reconstructed from style;
- direct cross-size/min-cross/max-cross resolution;
- `needsIntrinsicCross`;
- auto/stretch and shrink-to-fit decisions during final placement.

For each child, flex constructs a physical `SizeRequest` and obtains the
physical sizing result through the existing `speculateLayout()` /
`layoutPhase()` boundary. The existing `AxisHelper` selects width or height
from that result and flex stores values without erasing unresolved information.
Flex does not inspect why a value was initially unavailable and does not call
`resolve()` or `resolveOr()` to force an intrinsic result into a number.

Flex's calculation of line contributions from already-produced numbers remains
unchanged. Flex-container intrinsic contributions are still calculated from
the lines because that is a flex algorithm result; requesting and returning
those contributions is expressed through `SizeRequest`, not
`intrinsicSizesAxis`.

`determineAvailableMain()` is not preserved as a content fallback. If the
container main size is unresolved, flex produces the contributions needed by
the generic content reevaluation and does not run final free-space
distribution. When reevaluation resolves the container main size, the repeated
flex pass calls `resolveSizes()` with that concrete size. `resolveSizes()`
remains flex-owned; manufacturing its available size does not.

### `grid.cpp`

Grid continues to own placement, track sizing, span accounting, fraction
distribution, and final positioning.

Replace element sizing interpretation in contribution and placement passes:

- copied constraint mutations requesting max-content sizing;
- direct preferred/min/max width and height resolution;
- caller-side normalization of contributions using those resolved values;
- element-level `applyMinMax()` calls for final item sizes;
- checks of child width/height `isAuto()` when deciding stretch;
- shrink-to-fit mutations used to force content sizing.

Grid requests physical intrinsic results when collecting track contributions.
It stores those returned numbers in its existing contribution structures and
continues its track algorithm. During final placement, grid supplies the cell
allocation as an override or stretch opportunity and lets size evaluation
decide the child's used dimensions.

Grid track-definition unit handling remains in grid because it sizes tracks,
not element boxes.

### Ordinary and inline layout

Keep child traversal, cursor updates, margin behavior, inline box construction,
and calculation of content extents.

Remove sizing decisions based on explicitness, intrinsic-axis proxies, or
resolution modes. Ordinary block fill is represented as a physical stretch
opportunity. Inline/content sizing remains unresolved until content exists,
then uses the generic content reevaluation protocol.

`hadExplicitWidth` and `hadExplicitHeight`, or equivalent reconstruction from
current resolved values, must not survive. Any behavior that genuinely depends
on explicitness reads immutable `MeasuredSize::explicitSize` inside central
size evaluation.

## State to remove

The following are intermediate proxies and are removed after their call sites
use `SizeRequest`:

```cpp
Constraints::shrinkWidthToFit
Constraints::shrinkHeightToFit
Constraints::widthResolution
Constraints::heightResolution
Constraints::sizingAuthority
Constraints::intrinsicSizesAxis
SizingAuthority
ContentResolution
```

Also remove obsolete propagation, hashing, axis-helper accessors, and function
parameters associated with those fields. Do not replace them with renamed
boolean proxies.

Remove local element-sizing helpers once unused:

```text
layoutIntrinsicMain
FlexResolver::determineFlexBaseSize
FlexResolver::determineMinMainSize
FlexResolver::determineMaxMainSize
grid applyMinMax for element boxes
resolveRequestedSize
resolveContentSize
```

The low-level arithmetic helpers `resolveSpecifiedSize()` and
`resolveIntrinsicSize()` may remain as implementation details used by the
central evaluator or grid track sizing. They must not remain entry points for
formatting algorithms to resolve element sizes.

## Implementation order

Each step must leave the tree compiling. Visual verification belongs to the
user. Do not continue across a reported visual regression.

Current status: phases 1 and 2 are implemented and visually verified. Phase 3
now uses physical width/height intrinsic requests and results, and intrinsic
measurement no longer mutates `Measured`; it compiles cleanly and awaits visual
verification. Final contribution normalization and deletion of the intrinsic
proxy constraints remain coupled to the flex and grid migrations in phases
4–8. Phase 4 flex-main sizing now gathers child sizes through `SizeRequest`,
does not resolve element sizing in flex, and defers free-space distribution
until central evaluation supplies a concrete container main size. It compiles
cleanly. Phase 5 flex-cross sizing now obtains the child's cross size through
the same request path, requests cross contributions only when the parent needs
them, defers placement until both container dimensions are concrete, and offers
cross stretch without checking the child's style. The temporary duplicated
stretch-acceptance conditions remain part of the central-evaluator cleanup;
visual verification remains deferred until the flex and grid consumers are
migrated.

### 1. Establish the request and evaluation boundary

- Add `ContentSize` and the agreed request fields.
- Rename the central operation to `evaluateSizeRequest()`.
- Fold content-derived evaluation into that operation.
- Include every request field in speculative-layout hashing.
- Preserve current output behavior before deleting any caller branches.

Completion condition: ordinary layout still follows the current visual
behavior, and content-derived reevaluation no longer becomes a concrete
override.

### 2. Centralize generic unresolved recovery

- Make `layoutRecursive()` evaluate, lay out, calculate content, and reevaluate
  the same request when a dimension was unavailable.
- Keep the current single retry behavior; do not add a stabilization loop.
- Repeat child layout only when reevaluation changes the concrete box.
- Remove `resolveContentSize()`.

Completion condition: there is one unresolved/content recovery branch and it
contains no unit-, formatting-mode-, or intrinsic-policy checks.

### 3. Move intrinsic requests onto `SizeRequest`

- Request width and height intrinsic results with the physical request flags.
- Make intrinsic measurement use immutable `Measured`.
- Return optional width and height intrinsic results independently.
- Keep raw intrinsic measurement distinct from specified-size and bound
  normalization. Move that normalization into central evaluation as the
  ordinary, flex, and grid consumers migrate.
- Remove `intrinsicSizesAxis`, `widthResolution`, and `heightResolution` from
  ordinary recursive propagation once all immediate users are migrated.

Completion condition: no intrinsic pass mutates `Measured`, and callers do not
predict whether intrinsic measurement is needed by inspecting element style.

### 4. Migrate flex main sizing

Flex item sizing must preserve the working pre-refactor pass structure. The
existing child sizing/intrinsic pass already provides enough context to
produce the concrete inputs required by `FlexItem`; migration centralizes the
interpretation of that context and does not add speculative sizing passes.
Do not propagate an item size as unresolved merely because the migrated code
has failed to consume facts that the existing pass already returned.

In particular, do not recover the item inputs by probing arbitrary proposed
sizes, manufacturing numeric fallbacks, treating intrinsic contributions as
bounds, or invoking `RenderTree` from flex's distribution loop. Populate the
existing concrete flex bookkeeping from central evaluation of the context
gathered by the existing pass, then leave line formation and free-space
distribution unchanged.

- Replace the main-size/intrinsic preflight block in `FlexResolver::phaseB()`
  with physical sizing requests.
- Preserve unresolved returned values until generic content reevaluation has
  supplied enough context to produce the numeric flex-item inputs.
- Feed concrete returned sizing results into the existing flex item fields.
- Remove `determineAvailableMain()`; an unresolved container main size produces
  contributions for reevaluation rather than a caller-selected content
  fallback.
- Call `resolveSizes()` only when central evaluation has supplied a concrete
  container main size.
- Preserve line formation and free-space distribution unchanged.
- Delete the obsolete `determine*MainSize()` and `layoutIntrinsicMain()` paths.

Completion condition: flex main sizing contains no element `Size` resolution,
unit inspection, intrinsic-need prediction, automatic-minimum rule, or
copied-`Measured` mutation; it also performs no free-space distribution against
a main size manufactured from its own content contribution.

### 5. Migrate flex cross sizing and final placement

- Request physical cross sizing and optional cross contributions.
- Supply final main allocation as an override.
- Supply cross stretch as a stretch opportunity rather than checking auto.
- Preserve line cross aggregation, alignment, and placement unchanged.

Completion condition: flex cross/final placement contains no element-style
sizing decision and no sizing proxy mutation.

### 6. Migrate grid contribution sizing

- Request physical width contributions for column sizing.
- Request physical height contributions after column allocation establishes
  the child's available width.
- Store returned values in the existing grid contribution structures.
- Preserve track and spanning algorithms unchanged.

Completion condition: grid contribution collection contains no child preferred,
min/max, percentage, intrinsic, auto-minimum, or overflow sizing branches.

### 7. Migrate grid final allocation and stretch

- Supply grid cell dimensions as physical allocation facts.
- Represent alignment stretch as stretch opportunities.
- Let central evaluation apply element bounds and decide stretch acceptance.
- Preserve alignment offsets and placement unchanged.

Completion condition: grid final placement does not inspect child auto sizing or
apply element min/max sizing locally.

### 8. Delete superseded state and compatibility code

- Remove all proxy constraint fields listed above.
- Remove their propagation and cache hashing.
- Remove obsolete local sizing helpers and comments describing old failures.
- Remove unused `LayoutInput` sizing fields only if they are still genuinely
  unused after migration; do not expand cleanup beyond sizing.
- Update this document if the final names differ from the provisional names.

Completion condition: repository search shows that formatting algorithms do not
interpret element sizing styles and the removed proxy states have no remaining
references.

## Review checklist after every migration step

- Is `Measured` still immutable after measurement?
- Does the formatting algorithm only supply physical facts and store returned
  numbers?
- Is every element sizing unit/state check inside central evaluation?
- Does size evaluation remain unaware of formatting-algorithm bookkeeping?
- Is unresolved recovery still the single generic RenderTree path?
- Are intrinsic calculations still lazy?
- Did the change reuse existing recursive layout and result structures?
- Did it add a boolean or helper that merely hides an old condition? If so,
  remove it and represent the underlying fact instead.
- Did it preserve the formatting algorithm rather than rewriting it?

## Baseline sizing-decision audit

Call sites at restart commit `88810de`:

### Central orchestration: `render_tree.cpp`

- `render_tree.cpp:23` `applyContentResolution()`
- `render_tree.cpp:200–295` `makeConstraintsKey()` / `makeSpeculativeKey()`
- `render_tree.cpp:727–745` `measureIntrinsicSizes()`
- `render_tree.cpp:759–848` `layoutRecursive()` pre-layout sizing
- `render_tree.cpp:850–870` child constraint propagation
- `render_tree.cpp:900–950` `normalPass()` intrinsic sizing
- `render_tree.cpp:955–1005` `flexPass()` / `gridPass()` sizing bridges
- `render_tree.cpp:1015–1070` post-content sizing
- `render_tree.cpp:1080–1190` retry sizing
- `render_tree.cpp:1200–1220` final intrinsic contribution reconstruction

### Block and inline formatting: `new_arch.cpp`, `element.cpp`

- `new_arch.cpp:139–197` `resolveSize()`
- `new_arch.cpp:419–490` `LayoutEngine::layoutBlockNormalFlow()`
- `new_arch.cpp:500–750` inline normal/out-of-flow layout
- `element.cpp:288–301` `shouldTakeSoftBreak()`
- `element.cpp:570–630` prelayout/atomization sizing
- `element.cpp:635–715` `buildIsolatedInlineBoxes()`
- `element.cpp:720–815` `buildInlineBoxes()`

### Flex item sizing: `flex.cpp`, `flex.hpp`

- `flex.cpp:55–95` `layoutIntrinsicMain()`
- `flex.cpp:100–135` `determineFlexBaseSize()` / `determineMinMainSize()` /
  `determineMaxMainSize()`
- `flex.cpp:140–195` `determineAvailableMain()` / `determineAvailableCross()`
- `flex.cpp:225–285` `FlexResolver::phaseB()` main sizing
- `flex.cpp:289–303` intrinsic main aggregation
- `flex.cpp:305–360` cross sizing
- `flex.cpp:420–460` `phaseC()` final child sizing
- `flex.hpp:55–100` `AxisHelper` sizing accessors
- `flex.hpp:240–280` `FlexLine::resolve()`
- `flex.hpp:400–550` cross stretch and basis-definiteness sizing

### Grid item sizing: `grid.cpp`

- `grid.cpp:8` element `applyMinMax()`
- `grid.cpp:180–320` `resolveTracks()`
- `grid.cpp:400–415` width definiteness
- `grid.cpp:420–505` width contribution sizing
- `grid.cpp:515–630` height contribution sizing
- `grid.cpp:650–730` final item sizing

### Measurement boundary: `div.hpp`, `image.hpp`, `svg.hpp`, `text.hpp`

- `DivProcessor::measure()` around `div.hpp:220–265`.
- `ImageProcessor::measure()` around `image.hpp:280–320`.
- `SVGProcessor::measure()` around `svg.hpp:335–378`.
- `TextProcessor::measure()` around `text.hpp:250–265`.

### Shared representations and low-level helpers

- `new_arch.hpp`: `Measured`, `IntrinsicSizes`, `AxisResolution`, `Constraints`,
  `LayoutInput`, `LayoutResult`, and `resolveIntrinsicSize()`.
- `new_arch.cpp`: `resolveSize()`, `transferAspectRatio()`, block/inline layout,
  and auto-margin resolution.
- `sizing.hpp`: `Size::resolve()`, `resolveOr()`, `isAuto()`, and
  `isContentDependent()`.
- `element.hpp`: child constraint propagation in processor `layout()`.
- `render_tree.hpp`: recursive/speculative layout signatures.

## Validation

- Compilation is used only to catch implementation errors after substantive
  steps.
- The user performs visual comparison.
- Indefinite percentage, auto, intrinsic, flex, grid, aspect-ratio, absolute,
  and mixed cases must be checked before deleting their old branches.
- A step is not complete merely because code moved into a central function. It
  is complete only when the old caller-side sizing decisions are gone.

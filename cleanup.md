  My point it is that it is entirely duplicated normalization. Everywhere already checks and invalidates for percent sizing. This is backed by not finding a test case that actually
  causes this invariant to be true.

  "It is not obvious that all of those producers agree on whether margins have already been removed." - I do think it is double counting margins, but yes it is not entirely obviousl.

  "  So the algebra has a purpose. The fragility is that it assumes the child-height substitution is additive. It does not directly recompute sibling positions, margin collapse, or
  other
    layout behavior that might change when the child’s contribution changes.

    It is effectively using one concrete parent layout as a base and patching intrinsic child contributions into it. That is why it feels like slop: it is a shortcut around
  performing
    separate parent min/max-content layouts."

  No it does not really. I can read the code. I didnt need an explainer of what each specific term is doing. I just dont think it serves the purpose you describe whatsoever at all.
  It is stupid to me and worthless; it is not compensating for preventing an additional pass, rather it is entirel yuselses.

  "  I suspect the code currently gives different answers depending on the path." Obviously


• You’re right. I responded by inventing plausible purposes for suspicious code instead of asking whether those purposes are actually realized anywhere.

  For applyContentResolution(), your evidence is stronger than my hypothetical. If every relevant caller/path already invalidates percentage sizing and removing the helper changes no
  tested layout, then it is duplicated normalization. The burden should be on keeping it: produce a case where it changes a correct result. “Perhaps some descendant needs it” is not
  enough.

  The intrinsic adjustment is even more suspicious when followed recursively:

  parentAdjustment += childIntrinsicHeight - childComputedHeight

  For an ordinary normal-flow child during an intrinsic pass:

  childComputedHeight = childContentHeight
  childIntrinsicHeight = childContentHeight + childAdjustment

  Therefore:

  childIntrinsicHeight - childComputedHeight
      = childAdjustment

  So the parent adjustment merely propagates the descendant adjustment upward. Leaves contribute zero. Normal-flow containers do not originate meaningful adjustments themselves.

  A nonzero value must originate from some special producer—probably flex/grid returning an intrinsic height different from its concrete computed height. The normal-flow parent then
  assumes that difference can simply be added to its own concrete height. That is not “avoiding another pass” in any principled sense. It is either:

  - zero throughout the ordinary tree; or
  - importing a flex/grid difference using a questionable additive approximation.

  My previous explanation described the arithmetic without establishing that the arithmetic represents correct layout behavior. Your objection is valid.

  For intrinsicAvailableWidth, the only unit that actually observes the adjusted available size is fit-content; min-content and max-content ignore it. So the decisive case is narrow:

  definite available width
  + nonzero horizontal margins
  + fit-content width/min-width/max-width

  If the containing system already gave the node margin-reduced space, it is definitely a second subtraction. That should be settled with that case rather than more abstract
  discussion.

  More broadly, I think subtraction is the right first move—not a builder and not a new model:

  1. Remove one suspicious mechanism.
  2. Run the fixed sizing cases.
  3. If nothing changes, delete it.
  4. If something changes, isolate the smallest changed case.
  5. Decide whether the old or new result is correct.
  6. Only then identify the responsibility that must remain.

  That applies directly to:

  - applyContentResolution();
  - margin-adjusted intrinsic availability;
  - intrinsic height adjustments;
  - retry predicates;
  - duplicated post-retry clamping.

  You do not need to fully understand every overlapping state before doing this. Dead and duplicated mechanisms actively prevent understanding. Remove the ones with no observable
  responsibility first; whatever survives will reveal the actual irreducible sizing behavior.

  And yes, saying “different paths probably give different answers” contributed nothing. That is already the problem you described. The unanswered question is which differences are
  necessary and which are accumulated accidents.


› I think I basically understand every layout algorithm well enough. Some lines I have to spend 10 minutes thinking about, but my understanding hasn't evolved by looking at the code
  or tracing out a few examples of.

  The largest problem is that I have to spend 10 minutes thinking about it. This isnt really a documentation issue because the paths reveal themselves deep in recursive state.

  Subtraction is the right move, but it is hard to subtract without adding more state to augment the subtraction, particualrly dealing with measured. The problem of things like retry
  predicates is not that they are not specific enough; not a removal process.

  It is also hard to determine what is broken; I dont have a good test suite for this. Value inspection is largely uselss. I dont have playwright because this isnt a browser. My test
  cases sometimes dont catch all the beahavior, but are good **enough**

  It is largely just hard to figure out where to start and how to test this effectively, more so on the second part. The manual comment in/out thing is fine, but not fast enough nor
  comprehensive enough


• That reframes it correctly: you are not blocked on understanding the algorithms. You are blocked on changing them safely.

  The ten-minute pauses happen because the meaning of a local value depends on how the recursive call was entered, which copies were overridden, and whether this is an intrinsic,
  allocated, retry, or final pass. Reading more examples does not remove that cost.

  Also, subtraction cannot solve everything. With Measured, some implicit distinctions may need to become explicit before the overloaded state can be removed. Adding a field is not
  necessarily adding conceptual state if that distinction already exists invisibly through copies and mutation history. The useful metric is fewer roles per value, not immediately
  fewer struct members.

  The first project should probably be test infrastructure, not a sizing refactor.

  ## A practical native layout test runner

  Create a deterministic test mode that:

  1. Uses a fixed viewport.
  2. Selects a test scene by name or seed.
  3. Renders synchronously to completion.
  4. Exports the final observable layout tree.
  5. Exits.

  Each test element should have a stable test ID. The exported result only needs observable sizing information:

  test ID
  global x/y
  width/height
  child ordering
  possibly line count or fragment bounds

  Do not assert Measured, resolvedSize, retry flags, or other implementation details. Those are what you want freedom to change.

  For sizing, automated final-box comparison is much more valuable than manually inspecting values.

  ## Use three kinds of comparison

  ### 1. Current-output snapshots

  Save the GUI library’s current output for the full corpus.

  This does not prove correctness, but it makes subtraction fast:

  remove applyContentResolution
  run corpus
  show only fixtures whose final geometry changed

  If nothing changes, you know the deletion is behavior-preserving for the corpus. If twelve fixtures change, you inspect twelve cases rather than rereading the entire algorithm.

  ### 2. Browser comparison

  The native application does not need to be controlled by Playwright. Playwright would only run the React/browser reference.

  The two sides can run independently:

  React fixture → browser bounding boxes
  GUI fixture   → native bounding boxes
  comparison    → mismatches

  Your existing one-to-one React pages are already the beginning of this. A small coordinator can compare the exported results by test ID.

  Pixel-perfect screenshots are optional and potentially noisy because of text rasterization. Bounding rectangles and fragment/line geometry are the useful sizing oracle.

  ### 3. Explicit expected cases

  For behavior where browser parity is unavailable or intentionally different, keep a smaller set of hand-authored expectations:

  percentage child under 400px parent → 200px
  fit-content with margin → expected outer/inner width
  width clamp causes text reflow → expected height

  These should target contracts, not implementation state.

  ## Put many cases in one test atlas

  Rather than launching the GUI once per case, render hundreds of independent fixtures in one scene. Each fixture gets a fixed containing box and stable IDs:

  case-001: block / auto / definite parent
  case-002: block / percent / indefinite parent
  case-003: flex / min-content / allocation
  ...

  The React reference renders the same atlas.

  That gives you:

  - one native launch;
  - one browser launch;
  - one layout-tree comparison;
  - an individual diff for every fixture.

  The fixtures remain independent because each has its own fixed root container.

  ## Generate combinations systematically

  A complete Cartesian product will explode. Use pairwise combinations for factors such as:

  display:
      block, inline, flex, grid

  requested size:
      px, percent, auto, min-content, max-content, fit-content

  parent basis:
      definite, indefinite

  constraints:
      none, min, max, min+max

  allocation:
      none, stretch, flex allocation, grid allocation

  box additions:
      padding, margin, both

  content:
      empty, fixed child, text, nested container

  position:
      normal, absolute

  aspect ratio:
      absent, present

  Then add targeted three-way cases for interactions you already distrust:

  percent × intrinsic pass × parent allocation
  fit-content × margin × definite available size
  width clamp × text wrapping × retry
  intrinsic height × nested flex/grid × normal-flow parent

  A seeded generator can also create shallow recursive trees. Every failure retains its seed, so it becomes a permanent fixture after reduction.

  ## Measure whether the suite is actually useful

  Three signals answer different questions:

  - Branch coverage: did the corpus execute the questionable path?
  - Deliberate code mutation: would the suite notice if that path behaved differently?
  - Browser/expected comparison: which behavior is correct?

  For applyContentResolution(), for example:

  1. Confirm the helper’s percent branch is actually executed.
  2. Remove or invert that branch.
  3. Run the corpus.
  4. If every observable result remains identical, the suite has evidence that it is redundant.
  5. If something changes, the browser comparison tells you whether the change fixed or broke behavior.

  For intrinsicMinHeightAdjustment:

  1. Force the adjustment to zero.
  2. Run the intrinsic-height corpus.
  3. List only changed fixtures.
  4. If none change despite exercising the accumulation, remove it.
  5. If flex/grid-containing fixtures change, those are the only cases requiring semantic judgment.

  This is much faster and more informative than repeatedly commenting code and manually navigating the application.

  ## Where I would start

  I would not touch Measured first. Its refactor can alter many paths, and the current test feedback is too weak.

  I would first build the smallest possible automated loop:

  fixed native scene
  → final rectangles keyed by test ID
  → saved baseline
  → one-command comparison

  Then connect the existing React equivalents as a correctness oracle. After that, begin with the suspicious mechanisms that should be easy to subtract:

  1. applyContentResolution()
  2. intrinsic margin adjustment
  3. intrinsic height adjustments
  4. duplicated post-retry calculations

  Only after the suite can report the consequences in seconds would I touch the Measured distinction. That one may require temporarily making the already-existing “base measurement
  versus imposed attempt size” distinction explicit before anything can be deleted.
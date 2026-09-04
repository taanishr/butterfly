This is a systems codebase. Treat existing code as the source of truth.

Do not rely on remembered or summarized architecture. Inspect the relevant types,
callers, implementations, and tests before making a change.

For user-specified refactors:
- The requested target representation is authoritative.
- Do not add new state, flags, adapters, wrappers, compatibility paths, or alternate
  representations unless explicitly requested.
- Do not preserve a concept the user is removing under another name.
- Do not treat successful compilation/tests as the objective.
- Compiler/test failures are evidence about incomplete or incorrect migration.
- Reconsider your previous edit before introducing machinery to repair fallout.
- Prefer fewer representations of the same semantic fact.
- Do not modify unrelated code.

Before adding any state or representation, search for an existing representation of
the same semantic information.

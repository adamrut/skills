---
name: implement-prompt
description: Implement a numbered prompt document as written, stopping on material ambiguity and keeping docs/implementation/implementation-map.md accurate.
---

# Implement prompt

Implement a selected prompt document in the current workspace. Treat the prompt as the authority for requested behavior and scope. Do not ask implementation clarifying questions when the prompt is clear; stop and resolve the prompt itself if a material ambiguity prevents faithful implementation.

## 1. Select the prompt document

If the user supplied a prompt path or filename, use that document after verifying it exists. Otherwise:

1. Inspect `docs/prompts/` for Markdown files named `NNNN-prompt-description.md`, where `NNNN` is a four-or-more-digit number.
2. Select the file with the highest numeric prefix.
3. Tell the user which document is the default and ask whether to implement it or choose another prompt document. Do not begin implementation until the user confirms the selection.

If `docs/prompts/` does not exist or contains no matching prompt documents, ask the user to provide the prompt document; do not guess or implement a different source.

## 2. Check that the prompt is implementable

Read the selected prompt in full. Look for contradictions, unresolved questions, missing decisions, or acceptance criteria that permit materially different implementations. Also inspect the repository to resolve factual implementation details, such as existing interfaces, conventions, ownership, and test commands; do not treat repository discovery as a reason to ask about routine choices.

If a consequential behavior, scope boundary, or acceptance criterion remains ambiguous after repository inspection:

1. Stop before making implementation changes.
2. Explain the exact ambiguity and why it blocks faithful implementation. Ask the user for the missing decision; do not choose an outcome on their behalf.
3. After the user answers, propose the precise change to the selected prompt document and ask them to approve that update.
4. Only after approval, edit the prompt document, preserving its filename and existing confirmed requirements. Re-read it and repeat the readiness check before implementing. If approval is declined, leave the prompt unchanged and stop.

Do not silently edit the prompt, reinterpret contradictory requirements, or proceed with unresolved material decisions.

## 3. Inspect and implement

Before coding, inspect `docs/implementation/implementation-map.md` and any linked map files if they exist. Use the map as a navigation aid, not as the source of truth: verify relevant contracts, owners, and data flows against the current implementation. If the map is missing, continue repository inspection; create an accurate map after implementation.

Implement the selected prompt as written:

- Keep changes within its stated goal, requirements, and exclusions.
- Follow established repository patterns and preserve unrelated behavior.
- Resolve ordinary implementation or validation issues when they can be handled safely within the prompt. If a blocker requires a consequential decision outside the prompt, stop, explain the evidence and progress, and do not claim completion.
- Run the prompt's required validation and the relevant project-standard checks discovered in the repository. Fix regressions caused by the change; report checks that could not be completed and why.

## 4. Maintain the implementation map

Review and reconcile the map on every implementation run, including runs where the code change does not alter a mapped relationship. The map describes the repository as it exists after the implementation, not the requested design, a plan, or a history of changes.

Keep the map concise and factual. Use bullet points to capture durable relationships that help an agent navigate the codebase, especially:

- **Contracts**: interfaces, schemas, invariants, and boundaries between components.
- **Ownership**: which component or module owns a responsibility or source of truth.
- **Data flows**: where important data originates, how it is transformed, and where it goes.

Ground each point in the actual implementation and include relevant repository paths or symbol references where useful. Correct or remove stale, duplicated, speculative, or overly detailed entries. Do not add facts solely because they appear in the prompt; include them only after verifying them in code.

Maintain `docs/implementation/implementation-map.md` as the concise bullet-point entry point. Create `docs/implementation/` and this file if needed. If one file no longer remains concise and navigable for an agent, split details along logical ownership or data-flow boundaries into files named `implementation-map-<description>.md`. Keep the main file as a concise bullet-point index linking to those files; keep every split file concise, factual, and organized as bullet points. Use lowercase kebab-case descriptions. When splitting, remove details from the main file that now belong in a topic file, and when merging or pruning, remove obsolete split files only after checking that no valid information would be lost.

After updating the map, verify its statements and links against the resulting code. Do not leave entries that describe intended future behavior or claim validation that was not performed.

## 5. Finish with an accurate report

Before reporting completion, confirm that the selected prompt was implemented, the implementation map was checked and reconciled, and the required validation was attempted. Summarize the changes, report validation results and any remaining blockers, and link to the selected prompt, changed implementation-map files, and other important changed files. Never report success for work that was stopped or only partially completed.
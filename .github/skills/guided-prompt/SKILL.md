---
name: guided-prompt
description: Turn a rough request into a clear, implementation-ready prompt through rounds of clarifying questions, then save it in docs/prompts/.
---

# Guided prompt

Help the user and a future implementing AI agent reach a shared, explicit understanding of a requested feature, fix, update, or other change. Ask questions before finalizing; do not fill important gaps with assumptions.

## 1. Start with the user's goal

Begin by asking what the user wants to do:

> What would you like to do? Describe the feature, fix, update, or other change in your own words.

If the user already supplied a task with the skill invocation, briefly reflect that task and ask whether they have any additional context. Treat the supplied task as the starting request; do not make them repeat it.

## 2. Clarify in rounds

After the user describes the task:

1. Identify what is known, what is unclear, and which unclear points could materially change the implementation or its expected behavior.
2. Ask a focused first round of questions. Group related questions together, ask several at once when practical, and make each question easy to answer. Explain briefly why a consequential choice matters when that is not obvious.
3. Ask only questions that cannot be answered from information already provided or safely discovered by the implementing agent in the repository. Do not ask the user to decide routine implementation details that the agent can resolve by following existing project patterns.
4. After each answer, update the understanding and ask another round only for remaining material ambiguities or new questions revealed by the answers. Avoid repeating answered questions. Continue until the important decisions, intended behavior, scope, and success criteria are clear enough to implement without guessing.

Use the request's context to select relevant topics. Consider:

- Who or what the change is for and the problem it addresses.
- Expected behavior and acceptance criteria, including important edge cases and failure behavior.
- Scope boundaries and explicit exclusions.
- Compatibility requirements, affected interfaces, data changes, and migration needs.
- Constraints such as performance, accessibility, security, privacy, or supported platforms.
- How the change should be validated, if the user has requirements beyond the project's established tests.

Do not force questions from this list when they do not apply. If an important answer is unknown or the user cannot decide, state the uncertainty plainly in the prompt and instruct the implementing agent to investigate or stop for clarification rather than silently choose a behavior. Keep asking about unresolved decisions that would make implementation risky or materially different.

## 3. Maintain a draft during clarification

After the user answers the first round, create an initial draft in `docs/prompts/`. Update that same draft after each subsequent round so it reflects the latest shared understanding. The draft must distinguish confirmed requirements from unresolved questions; never present a guess as a confirmed decision.

If the request is already sufficiently clear after the first answer, write the completed prompt directly. Do not require a separate approval step before saving or finalizing. When the agent concludes that the material questions are resolved, finish the prompt and tell the user where it was saved. The user can still request changes afterward.

## 4. Name and write the document safely

- Save Markdown documents in the current workspace's `docs/prompts/` directory. Create the directory if needed.
- Use the filename format `NNNN-prompt-description.md`, for example `0001-prompt-add-export-button.md`.
- Choose the next number by finding the highest existing four-or-more-digit number in filenames beginning with `NNNN-prompt-` in that directory, then adding one. Start at `0001` if none exist. Use at least four digits; do not truncate numbers above `9999`.
- Before writing, check that the chosen path does not already exist. If it does, recheck the directory and select the next unused number; never overwrite an existing prompt.
- Make `description` a concise, lowercase, ASCII kebab-case summary of the request. Omit characters that are not valid in a filename.
- Keep the working draft and final prompt at the same path. Do not create a second numbered file just to finalize it.
- If the workspace location or file operation is unavailable, explain the specific problem and provide the completed prompt in the conversation instead. Do not claim a file was saved if it was not.

## 5. Write an implementation-ready prompt

Use a concise title and organize the document into relevant sections. Include:

- **Goal**: the problem and desired outcome.
- **Confirmed requirements**: only behavior and decisions established by the user.
- **Acceptance criteria**: observable conditions that define completion.
- **Scope and exclusions**: what to change and what not to change.
- **Constraints and compatibility**: applicable requirements and known limits.
- **Open questions or investigation**: only unresolved items, explicitly labeled; say what the implementing agent must verify or ask before choosing.
- **Validation**: expected tests or other checks, including project-standard checks the implementing agent should discover and run.

Adapt or omit sections that do not apply. Use ASD-STE100 as a guide for the prompt document:

- Write in clear, concise, controlled English.
- Use short sentences with one idea per sentence.
- Use the imperative mood for instructions and actions.
- Use consistent terms for the same concept, requirement, component, and behavior.
- Prefer simple, direct constructions and avoid ambiguity, jargon, hedging, and unnecessary qualification.
- Make requirements specific and verifiable; separate distinct requirements into separate bullets or sentences.

Do not invent technologies, file paths, designs, behavior, test results, or project conventions. Instruct the implementing agent to inspect the repository, follow its established patterns, and validate the actual acceptance criteria. Make clear that the agent should ask before making any remaining consequential choice that cannot be resolved through repository investigation.

Before finishing, check that the prompt is self-contained, internally consistent, and specific enough to guide implementation; that every important ambiguity is resolved or explicitly surfaced; and that the document is saved under the required name and location. Then give the user a brief summary and link to the saved document.
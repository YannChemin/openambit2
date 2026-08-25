Subject: Re: Re: openambit2 — the guided-workout screen itself

Hi André,

Good to hear the orbital-data confirmation lines up — nice that you've
got independent confirmation from two directions now.

Glad the last email helped crack part of it. On the remaining piece
(the native interval-name screen, "Accelerare" in your photo): I had a
subagent search the *entire* decompiled tree, not just
`WorkoutSyncer.java`, specifically chasing the `Binary` /
`TargetVirtualMachine` fields I'd noticed but not followed up on the SML
`Rule` object. Here's what came back — a real clue, but not a full
answer, and I want to be straight about which part is which.

## The clue: "Rule" is App-Zone bytecode, confirmed independently

`com.suunto.movescount.model.UserDeviceDisplayedRule` — the class behind
`getDisplayedRules()`, used by the custom-mode display-metric picker in
`AmbitSportModesActivity` — carries:

```java
public ArrayList<Integer> Binary;               // compiled bytecode, one int per byte
public String TargetVirtualMachineVersion;
```

Same shape as `WorkoutSyncer.Sml...Rule`'s `Binary`/`TargetVirtualMachine`
fields I mentioned before. That's not a coincidence — it means "Rule" is
one unified concept across this whole app: **source text compiled
server-side into bytecode targeting a specific VM version**, used both
for sport-mode display-metric rules and for guidance/workout rules
alike. That's independent confirmation, from a completely different
code path than your USBPcap captures, of the exact thing you already
found for the Apps region: a Rule is a compiled App-Zone-style program,
not raw data the firmware interprets directly.

## The dead end: the phone never moves the Binary anywhere

Here's the part that matters for what you're trying to do: I grepped the
whole tree for every place `.Binary` gets touched, and every BLE/SML
call in `com/suunto/komposti/`. **Nothing in this app ever writes a
Rule's Binary to the watch.** The mobile app only ever reads `RuleID`
and `Source` — never `Binary`, never `TargetVirtualMachineVersion`. Those
fields get populated on the model (presumably straight from the server's
JSON response) and then just... sit there, unused, dead-ended.

So the Movescount *phone* app's role in a guided workout was narrower
than either of us assumed: author/target the workout server-side, tell
the watch which Rule IDs it should have (`sml.DeviceSettings.Rules`),
and stop. The actual compile-and-flash step — taking that `Binary` and
physically writing it into the watch, the step that would make the
"Accelerare" screen appear — never happens in this codebase. Given what
you already know about SuuntoLink doing a structurally different,
directory-based Apps-region write (the real format you decoded from real
captures), my read is that step lived entirely on the **desktop**, in
SuuntoLink, not the phone app. This decompile simply can't tell us
anything about that binary transfer, because it was never here.

## What this does and doesn't get you

- **Does**: strengthens the case that a guided workout is *not* a
  separate native firmware concept from a Suunto App — it's the same
  compiled-bytecode mechanism you're already driving through the
  community compiler + `DISP_FIELD_SHORTCUT` slots. That's good news:
  you're not missing a whole separate subsystem, you're missing
  **opcodes/UI primitives** Movescount's own (private) compiler had
  access to that the public community compiler apparently doesn't
  expose — most plausibly whatever draws that bounded live target graph,
  and possibly a richer per-phase text label than a plain `prefix`
  string.
- **Doesn't**: give me a way to hand you a real compiled guidance-Rule
  `Binary` to diff against community-compiler output, which is the
  concrete next step I'd actually try if I had hardware. I don't own an
  Ambit and have no workout living on a watch to pull — I can't test
  this side of it myself, only point at where the evidence would have
  to come from.

## One concrete thing worth trying, if you can get the bytes

If you ever get your hands on a real `Rule.Binary` for a guidance-type
rule — from a live account that still has one cached, an old SuuntoLink
capture, or anyone else's dusty backup — diffing it byte-for-byte against
what the community compiler produces for an equivalent plain App Zone
script (same phase structure, same `SUUNTO_HR`/etc. targets) would tell
you exactly which opcodes are extra. That's the same "diff a real
capture against a clean pre-state" method that cracked the Apps-region
directory format (Finding 44) and the checksum scope (Finding 29) — it's
just that I don't have a source for that particular capture; you'd know
better than me whether anything like that survives anywhere (an old
Movescount API response cached in a browser devtools log, a phone's app
data backup from before the shutdown, anything).

Short version: the mystery isn't "what mechanism does this," it's "what
extra opcodes did Movescount's private compiler support" — narrower than
before, but I've run out of decompile to search on this one. If you find
a real Binary sample, I'm happy to help pick it apart.

Kind regards,
Yann

--
Dr. Yann Chemin
dr.yann.chemin@gmail.com

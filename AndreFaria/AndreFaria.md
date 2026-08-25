Subject: Re: openambit2 — training plans / scheduled workouts in Movescount

Hi André,

Thanks for reaching out, and for pointing me at Sommet. I went and cloned
it before replying — and I'm glad I did, because my first draft of this
email was going to hand you a decompiled protocol and a sketch patch for
"training plans," on the premise that neither of us had this solved yet.
You clearly have. `tools/training_plan.py`, `workout.py` and
`workout_install.py`, plus the Qt `TrainingProgramService`/QML page and
the matching TypeScript layer on Android, are a genuinely complete,
end-to-end revival of the feature — and a cleverer one than what I was
about to propose. So: scrapping that draft, this is the honest version.

## What you built vs. what I was about to suggest

I was going to point you at the old Movescount app's mechanism: workouts
uploaded to the cloud as a "Rule" of `Type=guidance`, an ID handed back,
then that ID written into `sml.DeviceSettings.Rules` to push it onto the
watch, keyed on a `TrainingProgram`-shaped flash region. Your own
findings (30-32, per the comments in `training_plan.py`) already went
down exactly that native road and hit the real wall: the flash region is
writable, but nothing in the firmware surfaces what's written there
without ground truth only a live Movescount could have given you. You
pivoted to something that doesn't need the dead cloud at all — date-gated
Suunto Apps, driving off `SUUNTO_DAYS_AFTER_1_1_2000`, so the watch
schedules itself from its own clock. That's not a workaround, that's a
better architecture than the original — no phone needed on the day.

For what it's worth, since I did dig through that old SML path in
detail: it's a plausible *explanation* for why 30-32 hit a wall, even
though it's not a route worth taking. The old app's upload step
(`WorkoutSyncer.uploadWorkout()` in the decompiled 1.5.11 build) sends
the workout to Movescount first and only gets an ID back from the cloud
*before* that ID is ever written to the watch — so whatever the firmware
needed to recognize and display a `TrainingProgram` entry may well have
depended on state the cloud round-trip established (server-side
validation, a signature, cross-referencing the account), not just "any
watch-side rule ID." If that's right, the native route was never fully
reproducible offline even in principle, which matches your own
conclusion — I just have a slightly more specific guess at *why* it's
walled off, for whatever that's worth to the writeup.

## The schema comparison — probably the one useful thing left

`workout.py`'s docstring says your JSON schema came from
`openambitproject/openambit#257`, wanarun.net, and Suunto's own French
tutorial. I got mine independently, straight from the official Android
app's own Java model classes
(`com.suunto.movescount.model.Workout`/`WorkoutStep`/`WorkoutStepType`/
`WorkoutStepDuration`/`WorkoutStepTarget`, decompiled from v1.5.11). The
core shape matches exactly — `name`/`steps`, `type.typeName` with
`warmup`/`interval`/`recovery`/`cooldown`/`repeatStart`/`repeatEnd`,
`duration`/`target` with a `valueRange{min,max}` — good independent
triangulation, three unrelated sources and a straight decompile all
agreeing is about as confirmed as this gets without a live watch.

Two small deltas worth you double-checking, since my source is the
literal client-side enum constants rather than examples:

- Your `duration.durationName` set is `time`/`distance`/`ascent`/`lap`.
  The Android app's `WorkoutStepDuration` constants also include
  `energy` (kcal) and two heart-rate-threshold triggers, `hr_below` /
  `hr_above` (step ends when HR crosses that bpm value). No idea if
  there's a live App Zone built-in that could drive `energy` or an
  HR-threshold trigger the way `SUUNTO_DURATION`/`SUUNTO_DISTANCE`/
  `SUUNTO_ASCENT` drive yours — if there isn't, that's plausibly *why*
  they're missing from your list rather than an oversight, but worth a
  five-minute check against the manual.
- Your `target.targetName` set is `none`/`hr`/`pace`/`speed`/
  `vertical_speed`/`power`. The Android app's `WorkoutStepTarget` also
  has `cadence`. Same caveat — only worth adding if `SUUNTO_CADENCE` (or
  equivalent) is a real built-in.

Neither of these contradicts what you've got; they're just enum values
that existed in the client Suunto shipped but that apparently never
showed up in any example workout the community's examined, which is
exactly the kind of gap a decompile catches and worked examples don't.

Your `notify{beep,light}` per-step field isn't in the Android app's
model at all — that doesn't mean it's wrong, it likely just means the
Android client never read/wrote it even if the real backend schema
carried it (Gson silently ignores JSON keys a class doesn't declare). If
anything that's a mild point in favor of it being a real, independently
legitimate field rather than something to second-guess.

## One more thing, unrelated to training plans

Since I had the decompile open anyway: I also went through how the
official app handles EON, Spartan/Suunto9, and the Spartan Trainer
specifically (device classification, pairing flow — spoiler, only
Ambit3/Traverse ever got a real manual-pairing walkthrough, everything
else auto-pairs — and per-family settings screens). Sommet's scope is
explicitly Ambit1/2/3/Traverse/Kailash/Etrex only per your FAQ, so this
is probably not useful to you now, but shout if that ever changes and
I'll send the notes over.

Good luck with the rest of it — Sommet is a lot of ground covered for a
project you describe as your first. Let me know if the `energy`/
`hr_below`/`hr_above`/`cadence` question turns up anything once you check
it against the App Zone manual.

Kind regards,
Yann

--
Dr. Yann Chemin
dr.yann.chemin@gmail.com

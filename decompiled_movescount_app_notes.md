# Notes from the decompiled Suunto Movescount Android app (v1.5.11)

Reference document for LLM assistants working on **openambit2**. Extracted
by decompiling the official Suunto Movescount Android app, build
**v1.5.11** (`versionCode 129`, built 2017-07-24), found at
`~/dev/movescount/movescount-1-5-11/` (`sources/` = jadx-decompiled Java,
`resources/` = extracted APK resources). That tree also has its own
`readme_for_LLM.md` (general repo map) and `watch_instructions.md` (full
per-family detail this document distills for openambit2's purposes).

**Read this critically**: the Movescount Android app talks to watches over
**Bluetooth LE** (phone companion app). **openambit2/libambit talks to
watches over USB HID** (`src/libambit/`, vendor ID `0x1493`,
`device_support.c`). These are two different transports with two
different pairing models — BLE needs an on-watch passkey pairing dance;
USB HID is just "plug in and it enumerates." Sections below are marked
**[transferable]** (protocol/data-model knowledge that applies regardless
of transport) or **[BLE-only, informational]** (describes the phone app's
Bluetooth pairing UX, not directly portable to openambit2's USB flow,
included for completeness / in case BLE support is ever added).

---

## 1. Device identification — cross-validates `device_support.c` **[transferable]**

The decompiled app's device-codename table
(`com.suunto.movescount.manager.m`, static field `h`) is effectively the
**same codename set** already hard-coded in openambit2's
`src/libambit/device_support.c`:

| Product name | Codename | In `device_support.c`? |
|---|---|---|
| Suunto Ambit | Bluebird | yes (PID `0x0010`) |
| Suunto Ambit2 | Duck | yes (PID `0x0019`) |
| Suunto Ambit2 R | Greentit | yes (PID `0x001d`) |
| Suunto Ambit2 S | Colibri | yes (PID `0x001a`) |
| Suunto Ambit3 Peak | Emu | yes (PID `0x001b`) |
| Suunto Ambit3 Run | Ibisbill | yes (PID `0x001e`) |
| Suunto Ambit3 Sport | Finch | yes (PID `0x001c`) |
| Suunto Ambit3 Vertical | Kaka | yes (PID `0x002c`) |
| Suunto Traverse | Jabiru | yes (PID `0x002b`) |
| Suunto Traverse Alpha | Loon | yes (PID `0x002d`) |
| Suunto EON Steel / EON Core | EON Steel / EON Core | not present — EON uses a different sync path in the phone app too (see §5) |
| Suunto Spartan Ultra/Sport/Sport HR/Trainer | Amsterdam/Brighton/Cairo/Dubai | not present — out of openambit2's current USB-HID scope |
| Suunto 9 Sport / Lima, Suunto 3 Fitness | Ibiza / Lima / Helsinki | not present — same as above |

This is a useful **independent cross-check**: two unrelated codebases
(Suunto's own Android app, and the third-party openambit reverse-engineering
effort) agree on the same product↔codename mapping, which increases
confidence the `device_support.c` table is complete and correct for the
Ambit/Ambit2/Ambit3/Traverse family. No new codenames were found in the
Android app that are missing from `device_support.c`.

**One gap worth knowing about**: the Android app's own *live* BLE
device-name matcher (`com.suunto.movescount.suuntoconnectivity.b.c`,
matching by advertised Bluetooth name prefix, e.g. `"suunto ambit3
peak"`) only recognizes the **four Ambit3 variants + Traverse +
TraverseAlpha** — it has no matcher branch for plain "Ambit" or "Ambit2"
at all, even though the codename table (above) still lists them. In other
words: by 2017 (this build's date), even Suunto's own official app could
no longer *pair* with an original Ambit or Ambit2 over Bluetooth — only
recognize/sync Ambit3-and-newer over BLE. This has no bearing on
openambit2's USB path (openambit2 talks USB HID directly to Ambit/Ambit2/
Ambit3 alike, sidestepping BLE compatibility entirely — this is in fact
exactly the kind of thing that makes openambit2 valuable for older-watch
owners).

### Pseudocode: identical concept already implemented in `device_support.c`

```c
// device_support.c — libambit_device_support_find() (existing, for reference)
const ambit_known_device_t *find_device(vendor_id, product_id, model_name, fw_version) {
    for each row in known_devices[] {
        if row.vid == vendor_id
           and row.pid == product_id
           and row.model == model_name
           and fw_version_number(fw_version) >= fw_version_number(row.min_sw_version)
        {
            return &row.public_info;   // driver, feature flags, komposti_version
        }
    }
    return NULL;   // unrecognized device
}
```

No change needed here — just documenting that the Movescount app's own
device table independently confirms this logic/table is right.

---

## 2. Per-model capability flags **[transferable — useful for feature-gating the GUI/settings]**

The Android app's `ISuuntoDeviceCapabilityInfo` interface
(`manager/deviceid/*.java`, one implementation class per device type) is a
**richer capability model** than the vendor/product/min-fw struct in
`ambit_known_device_t` — it encodes per-feature support booleans that
openambit2 could use to decide which settings/UI controls to expose per
connected model (today `device_driver_ambit.c` / `device_driver_ambit3.c`
likely branch ad hoc on driver type; this table is a cleaner reference for
what *should* differ by model). Key figures pulled from the decompiled
classes, all for devices in openambit2's current scope:

| Model | GPS hours (best/good/OK)* | Barometric altitude | Storm alarm | Multisport mode | Workouts | Sunrise/sunset alarm | Vibration |
|---|---|---|---|---|---|---|---|
| Ambit3 Peak/Sport/Run/Vertical | 20 / 30 / 200 | yes | yes | yes | firmware-version-gated | no | no |
| Traverse / Traverse Alpha | 20 / 30 / 200 | yes | yes | yes | firmware-version-gated | no | no |

*("best/good/OK" = GPS accuracy modes, in hours of battery life — these
are static per-model UI constants in the Android app, not something read
live from the watch; if openambit2 wants to show similar battery-life
estimates it would need to hardcode the same kind of table, not query the
device for it.)

Ambit1/Ambit2-specific capability figures were **not found** in the
Android app — as noted in §1, the app's BLE-era capability classes only
cover Ambit3-and-newer; Ambit/Ambit2 capability differences (if any beyond
what `device_support.c`'s per-PID `min_sw_version`/feature struct already
encodes) would need to come from another source (e.g. the original
`openambit` project's protocol notes, or Suunto's older desktop
Moveslink software) — not from this decompiled app.

### Pseudocode: capability-flag lookup pattern (for possible adoption)

```c
// Sketch of a capability table openambit2 could add alongside device_support.c,
// modeled on the Android app's per-model ISuuntoDeviceCapabilityInfo classes.
typedef struct {
    const char *model_name;
    int gps_hours_best, gps_hours_good, gps_hours_ok;
    bool supports_barometric_altitude;
    bool supports_storm_alarm;
    bool supports_multisport_mode;
    bool supports_workouts;          // may itself be firmware-version-gated
    bool supports_sunrise_sunset_alarm;
    bool supports_vibration;
} device_capabilities_t;

static const device_capabilities_t capabilities[] = {
    { "Suunto Ambit3 Peak",   20, 30, 200, true, true, true, true /*fw-gated*/, false, false },
    { "Suunto Traverse",      20, 30, 200, true, true, true, true /*fw-gated*/, false, false },
    // ... Ambit / Ambit2 rows would need independent verification, not sourced from this app
};

const device_capabilities_t *get_capabilities(const char *model_name) {
    for each row in capabilities[] {
        if strcmp(row.model_name, model_name) == 0 { return &row; }
    }
    return NULL;
}
```

---

## 3. Sport-mode / custom-mode sync model **[transferable — directly relevant to openambit2's Sport Modes tab]**

openambit2's README advertises a working Sport Mode editor (19 presets,
read/write to watch) — the Android app's equivalent screen
(`AmbitSportModesActivity`) reveals a couple of implementation details
worth cross-checking against openambit2's own sport-mode code
(`src/movescount/` likely, or wherever sport-mode serialization lives —
see `src/libambit/sport_mode_serialize.c`):

- Sport-mode configuration is modeled as **`CustomModeGroup`** (a named
  group, potentially "multisport" i.e. containing several
  `CustomMode`s selectable during one activity) each containing one or
  more **`CustomMode`** entries (a single sport profile: activity type,
  display screens/metrics, sensor pairing, autolap, etc.) — a
  group→modes hierarchy, not a flat list of modes.
- **New, not-yet-synced modes/groups are assigned negative integer IDs**
  locally (`private static int nextId = 0; int allocate() { return
  --nextId; }`), as a marker that "this ID hasn't been assigned by the
  server/watch yet, don't treat it as a real persisted ID." If
  openambit2 needs to create new sport modes locally before writing them
  to the watch (rather than always mutating an existing on-watch mode),
  this convention (negative-ID = pending/local-only) is a simple,
  proven way to distinguish "new" from "existing" entries without a
  separate boolean flag.
- Each metric-display screen within a mode has a `Type` field (seen set
  to `4` for a newly created default display) and is edited via a
  sub-screen listing 2 rows of up to some N metrics each (the Android UI
  passes `(customMode, display, rowCount=2, colCount/selectedIndex=0)`
  when opening the metric picker) — i.e. **the watch face's data screens
  are modeled as a small grid of metric slots per display**, not a single
  metric per screen.

### Pseudocode: local-ID allocation + group/mode hierarchy

```c
// Sport mode data model (matches AmbitSportModesActivity's shape)
typedef struct {
    int custom_mode_id;          // negative = pending local creation, not yet synced
    char *name;
    int activity_id;
    display_config_t displays[MAX_DISPLAYS_PER_MODE];
} custom_mode_t;

typedef struct {
    int custom_mode_group_id;    // negative = pending local creation
    char *name;
    int activity_id;
    bool is_visible;
    bool is_multisport;          // true => group holds >1 selectable custom_mode
    custom_mode_t *modes;        // one mode if !is_multisport, else several
    int mode_count;
} custom_mode_group_t;

static int next_local_id = 0;
int allocate_pending_id(void) {
    return --next_local_id;      // -1, -2, -3, ... marks "not yet synced"
}

// When pushing to the watch/server: only entries with id < 0 are "new";
// after a successful write, replace the negative placeholder with the
// real id the watch/server assigns.
void sync_custom_mode_groups(custom_mode_group_t *groups, int count) {
    for (i = 0; i < count; i++) {
        if (groups[i].custom_mode_group_id < 0) {
            create_new_group_on_device(&groups[i]);   // assigns a real id back
        } else {
            update_existing_group_on_device(&groups[i]);
        }
    }
}
```

### Verified against openambit2's actual source (2026-08-19)

Checked `src/movescount/sportmode.h`/`sportmodegroup.h` and
`src/openambit/sportmodeeditorwidget.cpp` directly:

- **Group/mode hierarchy: already implemented, matches well.**
  `CustomModeGroup` (list of `customModeIds`, `isVisible`, `activityId`)
  containing `CustomMode` entries, and `CustomModeDisplay` already uses
  the *same* row/type encoding seen in the decompiled app
  (`MOVESCOUNT_SINGLE_ROW_DISPLAY_TYPE = 4`, double = 6, triple = 5).
  Nothing to change here.

- **Negative/pending-ID convention: NOT implemented — and this is a real,
  reachable bug, not just a stylistic gap.**
  `SportModeEditorWidget::onAddMode()` (`sportmodeeditorwidget.cpp:282-296`)
  assigns new-mode IDs as:
  ```cpp
  nm["CustomModeID"] = m_modes.size();   // plain running count, not unique
  ```
  `SportModeStorage::factoryDefaults()` ships **19 presets with
  `CustomModeID` 1–19** (Running=1 … "Other"=19). Load the defaults (list
  size 19), click **Add** → the new mode gets `CustomModeID = 19`, a
  direct duplicate of the existing "Other" preset. That value is written
  straight into the on-device record: `CustomMode::toAmbitSettings()`
  (`sportmode.cpp:206`) does
  `settings->sport_mode_id = (uint16_t)sportmodeId;` with **no
  uniqueness check anywhere in `src/libambit/`** (grepped — the field is
  just a plain struct member, `libambit.h:480`). So this writes two sport
  modes with the same on-watch ID, silently.
  **Minimal fix**: don't derive the new ID from `m_modes.size()`; use
  `max(existing CustomModeIDs) + 1` (or adopt the negative-placeholder
  convention above if a "pending vs. synced" distinction becomes useful
  later, e.g. for round-tripping IDs the watch/server assigns back).

---

## 4. Ambit3/Traverse BLE manual-pairing flow **[BLE-only, informational — not applicable to openambit2's USB HID path]**

Included for completeness in case openambit2 ever grows a BLE/mobile
companion mode. The Android app's only real pairing-instructions screen
(`AmbitPairingInstructionsActivity`, a 3-page video walkthrough) exists
specifically because Ambit3/Traverse BLE pairing requires **manual
watch-side interaction**:

1. Connect the watch to USB power to wake/unlock it.
2. On the watch, run its startup wizard; after choosing a language,
   answer "Yes" to pair with the mobile app.
3. The watch displays a passkey; the user types that passkey into the
   phone app and taps "Pair."

This has **no analogue in USB HID** (openambit2's transport) — plugging
in over USB is inherently "paired" the moment the OS enumerates the
device (subject to the udev rule already shipped in openambit2's Debian
packaging). No action needed; noted only so this distinction doesn't get
conflated if BLE support is ever explored.

---

## 5. EON / Spartan / Suunto9 / Trainer — confirmed out of scope **[informational]**

The decompiled app confirmed these families use an **entirely different
generation/transport** than Ambit/Ambit2/Ambit3/Traverse:

- **EON Steel/Core**: despite being newer hardware, they actually share
  Ambit3/Traverse's legacy BLE/GATT transport (`KOMPOSTI` generation in
  the app's terms) — but are absent from `device_support.c`, so
  openambit2 doesn't talk to them today. If ever added, note EON uses a
  **live SML-based settings protocol** (query/set individual settings by
  XML-ish SML tags over the connection, not a monolithic settings blob)
  — different in character from how Ambit/Ambit2/Ambit3 settings appear
  to be handled in `src/libambit/personal.c`/`protocol.c`.
- **Spartan (all variants) and Suunto9**: a separate `WHITEBOARD`
  generation/`ng_ble` transport, with generally lower estimated GPS
  battery life (6–15h "best" mode vs. Ambit3/EON's 20h) and — notably —
  **no workout-planner support** in several Spartan capability classes
  even in the official app (`supportsWorkouts()` hard-`false` for
  SpartanSport/SpartanUltra/SpartanTrainer). Not relevant to
  openambit2's current Ambit-family focus; recorded here only so a future
  contributor doesn't assume EON/Spartan support is "just a small
  extension" of the existing Ambit USB driver — it would be a materially
  different protocol implementation (BLE, `ng_ble`/`KOMPOSTI` framing,
  SML settings model), likely a separate driver module.

Full detail (per-state UI screens, settings field lists, etc.) is in
`~/dev/movescount/movescount-1-5-11/watch_instructions.md` if ever needed.

---

## 6. Summary for openambit2 contributors

- **Do**: treat §1's codename cross-check as confirmation
  `device_support.c` is complete/correct for Ambit/Ambit2/Ambit3/Traverse;
  consider §2's capability-flag pattern if openambit2 wants richer
  per-model feature gating; consider §3's negative-local-ID convention if
  extending the sport-mode editor to create brand-new custom modes rather
  than only editing existing ones.
- **Don't**: port §4's BLE pairing-passkey flow into the USB codepath —
  it doesn't apply. Don't assume EON/Spartan/Suunto9 support (§5) is a
  quick addition — the Movescount app itself treats them as a distinct
  generation with a distinct transport and settings model.
- **Caveat**: all of this is reverse-engineered from one specific,
  Proguard-obfuscated Android build (v1.5.11, mid-2017) of a since-
  discontinued app. Cross-check anything load-bearing against
  `device_support.c`'s own `min_sw_version` gating and, where possible,
  against real hardware, before relying on it.

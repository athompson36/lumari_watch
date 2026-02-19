# 🚀 LUMARI DEVELOPMENT ROADMAP

## 🧭 Overall strategy

We build in **5 layers**. Each phase should produce something **usable**.

| Layer | Focus |
|-------|--------|
| 1. **Foundation** | Alive on screen |
| 2. **Core Loop** | Quest + XP + Inventory |
| 3. **Engagement** | Mini-games + Cutscenes |
| 4. **Ecosystem** | iPhone App + Sync |
| 5. **Expansion** | HR, Haptics, Apple Watch |

---

## 🟢 PHASE 0 — Hardware bring-up (1–2 weeks)

**Goal:** Prove the board + toolchain.

**Deliverables:**

| Item | Status |
|------|--------|
| ESP-IDF project builds cleanly | ✅ |
| AMOLED working (410×502) | 🚧 HAL in place; validate on hardware |
| Touch input working | ✅ (FT3168 I2C) |
| Buttons mapped | ✅ (BOOT/PWR) |
| IMU readable | ✅ (QMI8658) |
| RTC readable | ✅ (PCF85063) |
| Basic power sleep/wake | ✅ (display_hal_sleep/wake) |

**Visible output (no Lumari yet — just control):**

- [x] Black screen
- [x] Simple floating circle
- [x] Tap changes color
- [x] Button toggles brightness
- [x] Shake prints debug (IMU stub; wire real IMU for shake)

---

## 🟢 PHASE 1 — Lumari core (2–4 weeks)

**Goal:** Creature exists.

**Build:** Render engine, layer stack, idle animation, expression layer, basic state machine.

**Deliver:**

- [x] Lumari Seedling (pastel)
- [x] Simple XP counter
- [x] Long press → system menu (tap to close)
- [x] Sleep/wake wrist rotation (IMU face-down/up)
- [x] Step counter working (XP stub)

**At end of Phase 1:** A living creature on your wrist. No quests, no inventory — just alive.

---

## 🟢 PHASE 2 — Core loop (4–6 weeks)

**Goal:** Real progression.

**Build:** Quest engine, XP math, momentum meter, inventory, basic crafting, loadout switching, save/load JSON, pixel mode (first alternate skin).

**Deliver:**

- Add quest → complete quest → XP increases
- Unlock & equip accessory
- Craft simple aura
- Momentum meter animates

**Milestone:** “It works.”

---

## 🟢 PHASE 3 — Lore + cutscenes (3–5 weeks)

**Goal:** Myth layer activated.

**Build:** Cutscene engine (8-bit + 16-bit style), dialogue parser, lore unlock flags, codex screen.

**Deliver:**

- First Lumari evolution cutscene
- First Aetheron intro cutscene
- Pixel mode cutscene
- Replayable in lore menu

**At this stage:** Feels like a real RPG world.

---

## 🟢 PHASE 4 — Mini-games + IMU (4–6 weeks)

**Goal:** Engagement layer.

**Build:** Aura Alignment, Orbit Control (tilt), Particle Garden, Energy Sync, shake-to-celebrate, adaptive mode logic.

**Deliver:**

- Calm mode auto-switch
- Celebration surge mini-game
- Step-based micro XP (capped)
- Bond increases via Energy Sync

**Outcome:** System is emotionally dynamic.

---

## 🟢 PHASE 5 — iPhone app v1 (6–8 weeks)

**Goal:** Companion ecosystem.

**Build:** BLE pairing, quest creation, XP/inventory sync, Creature Forge, parent mode (calendar only), optional parent Aetheron.

**Deliver:**

- Add quest from phone
- See Lumari animate in app
- Equip skin from phone
- Parent can add calendar quests; teen full control

**Milestone:** First “ecosystem moment.”

---

## 🟡 PHASE 6 — Polishing & stability (4 weeks)

- Power optimization, sleep/wake tuning
- Frame rate & memory profiling
- BLE stability, crash recovery, data integrity

**Outcome:** Wearable, not just demoable.

---

## 🔵 PHASE 7 — Expansion (future)

Optional, after stable v1:

- **Heart rate module** — Hardware redesign, arousal index, biofeedback mini-game
- **Haptic module** — LRA + DRV2605, breathing pulse patterns
- **Apple Watch version** — Lightweight Lumari, native HR + haptics, simplified mini-games, shared lore

---

## 🧠 Time reality

| Cadence | Phases 0–2 | +3–4 | +5 | Full ecosystem MVP |
|---------|------------|------|----|--------------------|
| Part-time (nights/weekends) | ~3 months | +2 months | +2 months | **~6–8 months** |
| Tightly scoped | — | — | — | Faster |

---

## 🎯 Critical advice

**Do NOT start with:** Crafting, pixel mode, multi-style cutscenes, parent species, HR.

**Start with:**

1. ✔ Display  
2. ✔ Idle Lumari  
3. ✔ Quest completion → XP  

*Momentum builds motivation for you too.*

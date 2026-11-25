# HK6B — RadioFoundry DM-32UV: cumulative firmware development (NXDN, DMR and CPS)

**Author / contributor:** [@thediex](https://github.com/thediex) · amateur-radio callsign **HK6B**  
**Reference device:** Baofeng DM-32UV  
**Public contribution milestone:** **V05 P117** · [Upstream discussion #1087](https://github.com/RadioFoundry/RadioFoundry/issues/1087)

> **Important:** HK6B is a **cumulative firmware customization**, not a standalone Motorola Basic Privacy feature. The working P117 firmware retains the earlier NXDN and user-interface work. **This public repository contains a 32-file BP/CPS-oriented review subset, NOT the complete cumulative firmware, full CPS website, or a ready-to-merge upstream patch.** The complete development repository remains private.

## What has been implemented in the cumulative firmware

The operator reports physically verifying the cumulative functions on their own DM-32UV. The milestones build on one another rather than replacing prior functionality.

| Development milestone | Capability |
| --- | --- |
| Early V01–V03 | Receiver audio and display/interface refinements |
| **V03E — NXDN/OAA** | NXDN reception with source identification and reconstruction/display of over-the-air alias (OAA); numeric identification remains available |
| **V04 — NXDN recent contacts** | Separate in-memory list of the **last three NXDN interlocutors**, including alias/ID and call context, with deduplication for continuing transmissions |
| **V05 P115** | Motorola Basic Privacy with **four shared key slots**, per-channel selection, on-radio key management and persistence |
| **V05 P116** | On-radio editing of per-channel BP assignments and shared-key warning; tested across restarts |
| **V05 P117** | Browser-based CPS / Web Serial interface for reading and writing the four BP key slots, including read-back |

**Retained capabilities:** the full cumulative firmware includes the RadioFoundry baseline and earlier NXDN 4800/9600-related development, identification, alias display, recent-caller presentation and other HK6B work. This is not a promise that every capability is implemented by the 32 files in this repository.

### Physical validation reported by the operator

On their DM-32UV, the operator reports verifying that the cumulative NXDN features, last-three-caller view, per-channel BP settings, persistence after reboot, and CPS BP read/write operate in the installed firmware. This is **single-device field validation**, not a cross-hardware compatibility certification or evidence that every combination of Dual Watch and in-call transitions has passed automated tests.

For the NXDN OAA/history implementation in the complete firmware, relevant code includes `core/nxdn/nxdn_call.c`, `core/radio/radio_nxdn.c`, `core/ui/ui_app.c`, `core/ui/ui_screens.c` and `core/ui/ui_screens.h`. **Those specific files are not all included here.** Their omission from this review subset must not be interpreted as removal from P117.

## What is actually published in this repository

This is a **selected 32-file source snapshot** focusing on the P115–P117 Motorola BP and CPS work, with:

- `core/radio/hk6b_privacy_*.h` — map, parser, cache, effective slot resolution and guards.
- `core/radio/radio_sm.c`, `core/settings/`, `core/ui/` — relevant integration files from the P117 source snapshot.
- `site/static/js/codeplug/` and `site/static/js/editor/` — selected CPS source modules.
- `v05-tests/` — HK6B privacy-related tests.
- `HK6B-V05-P117.md` — detailed snapshot scope and development milestones.

**Not included:** the complete RadioFoundry tree, all NXDN/OAA/history sources, the built website, flashable firmware/codec, codeplugs, personal radio IDs or settings, BP key values, memory backups, serial credentials and private development history. **Do not attempt to flash or build this selection as a complete firmware.**

## How this relates to upstream RadioFoundry

- **Official project:** [RadioFoundry/RadioFoundry](https://github.com/RadioFoundry/RadioFoundry)
- **Contribution proposal:** [Issue #1087 — NXDN UI and Motorola BP/CPS](https://github.com/RadioFoundry/RadioFoundry/issues/1087)
- **Current public review material:** this repository; **not** an upstream pull request.
- **Development reference:** cumulative private branch `hk6b-v05-p117`, baseline commit `5a666b04a346f113c7f6f8d59886a6a54a2e2a4f`.

The intent is to work with maintainers on **separate, clean, reviewable upstream contributions** (for example NXDN/OAA UI and history, BP per-channel controls, CPS/serial protocol), including provenance/licensing review and appropriate tests. Upstream acceptance is not assumed.

## Security, licensing and limitations

The RadioFoundry MIT license is preserved in [LICENSE](LICENSE); upstream author credit and project clean-room requirements must be respected. **Motorola Basic Privacy is weak obfuscation, not strong contemporary encryption.** This repository contains no real BP key values and is not a release package. Verify regulatory requirements and use only authorized communications.

_Repository name retained for existing links: `HK6B-RadioFoundry-P117-Contribution` denotes the P117 public **submission snapshot**, not the complete scope of the HK6B cumulative firmware._

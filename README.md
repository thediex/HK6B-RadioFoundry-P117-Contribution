# HK6B P117 â€” Motorola Basic Privacy for RadioFoundry DM-32UV

Technical contribution candidate from **thediex**. This repository contains selected source files only, without historical branches, private codeplugs, keys, firmware binaries, or the full generated website.

Features tested on physical hardware: four shared BP key slots; per-channel slot selection; on-radio key and slot editing; persistent settings; and Web Serial CPS reading, writing and read-back.

Implementation reference: `thediex/HK6B-RadioFoundry-DM32UV`, branch `hk6b-v05-p117`, commit `5a666b04a346f113c7f6f8d59886a6a54a2e2a4f` (private development history).

## Review notes

This is a **selected source snapshot**, not a standalone firmware build or a ready-to-merge PR. Maintainers can advise how to split patches for upstream integration. The original RadioFoundry license remains in `LICENSE`; upstream authorship must be preserved. The full built CPS and firmware are deliberately excluded from this public review candidate.

Outstanding validation: exhaustive dual-watch and all in-call configuration transitions. Motorola Basic Privacy is not modern strong encryption.
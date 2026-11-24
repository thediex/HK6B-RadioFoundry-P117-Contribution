// SPDX-License-Identifier: MIT
// HK6B P117 - Motorola Basic Privacy slot management over the local serial debug link.
// Distinct from the channel-to-slot codeplug KV map. Values are read from the live radio.
import { CMD } from "../flasher/link.js";
const GET = 1, SET = 2;

export async function readBpKeys(link) {
    const raw = await link.request(CMD.HK6B_BP, new Uint8Array([GET]));
    if (raw.length !== 12) throw new Error(`P117: expected 12 BP bytes, got ${raw.length}`);
    return Array.from({ length: 4 }, (_, i) => {
        const type = raw[3 * i], key = raw[3 * i + 1] | (raw[3 * i + 2] << 8);
        return { slot: i + 1, type, key, valid: (type === 0 && key === 0) || (key >= 1 && key <= 255) };
    });
}
export async function writeBpKey(link, slot, key) {
    if (!Number.isInteger(slot) || slot < 1 || slot > 4 ||
        !Number.isInteger(key) || key < 0 || key > 255)
        throw new RangeError("BP slot 1..4, key 0..255 required");
    await link.request(CMD.HK6B_BP, new Uint8Array([SET, slot, key]));
    const readback = await readBpKeys(link);
    const got = readback[slot - 1];
    if (got.key !== key || (key === 0 && got.type !== 0))
        throw new Error("P117: BP readback does not match; check the radio");
    return readback;
}

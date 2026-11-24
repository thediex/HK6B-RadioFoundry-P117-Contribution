// HK6B V05 - DMR Basic Privacy per channel
// KV record: version, count, then (channel u16 LE, slot u8).
// No encryption keys are stored in this record.

export const HK6B_PRIVACY_KEY = 0x1E00;
export const HK6B_PRIVACY_VERSION = 1;
export const HK6B_PRIVACY_MAX = 4;

export function validPrivacySlot(slot) {
    return Number.isInteger(slot) &&
        ((slot >= 1 && slot <= 4) || slot === 255);
}

export function encodePrivacyMap(mappings) {
    if (!Array.isArray(mappings) ||
        mappings.length > HK6B_PRIVACY_MAX) {
        throw new RangeError("Invalid privacy mapping count");
    }

    const seen = new Set();
    const bytes = new Uint8Array(2 + mappings.length * 3);
    bytes[0] = HK6B_PRIVACY_VERSION;
    bytes[1] = mappings.length;

    mappings.forEach(({ channel, slot }, i) => {
        if (!Number.isInteger(channel) ||
            channel < 1 || channel > 1024 ||
            !validPrivacySlot(slot) ||
            seen.has(channel)) {
            throw new RangeError("Invalid privacy mapping");
        }

        seen.add(channel);

        const offset = 2 + i * 3;
        bytes[offset] = channel & 0xFF;
        bytes[offset + 1] = channel >> 8;
        bytes[offset + 2] = slot;
    });

    return bytes;
}

export function decodePrivacyMap(bytes) {
    if (bytes == null) return [];

    if (!(bytes instanceof Uint8Array) ||
        bytes.length < 2 ||
        bytes[0] !== HK6B_PRIVACY_VERSION ||
        bytes[1] > HK6B_PRIVACY_MAX ||
        bytes.length !== 2 + bytes[1] * 3) {
        throw new RangeError("Invalid privacy record");
    }

    const mappings = [];

    for (let i = 0; i < bytes[1]; i++) {
        const offset = 2 + i * 3;
        mappings.push({
            channel: bytes[offset] | (bytes[offset + 1] << 8),
            slot: bytes[offset + 2]
        });
    }

    // Reuse the encoder to validate duplicates and ranges.
    encodePrivacyMap(mappings);

    return mappings;
}
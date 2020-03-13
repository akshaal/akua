// This file is auto-generated by src/avr/maintain-protocol script! DON'T EDIT!

export const avrProtocolVersion = 0x8e;

export interface AvrData {
    "u32 uptime_deciseconds": number,
    "u8 debug_overflow_count": number,
    "u8 usart0_rx_overflow_count": number,
    "u32 main_loop_iterations_in_last_decisecond": number,
    "u32 ((u32)last_drift_of_clock_deciseconds_since_midnight)": number,
    "u32 clock_corrections_since_protection_stat_reset": number,
    "u32 clock_deciseconds_since_midnight": number,
    "u8 ds18b20_aqua.get_crc_errors()": number,
    "u8 ds18b20_aqua.get_disconnects()": number,
    "u16 ds18b20_aqua.get_temperatureX16()": number,
    "u8 ds18b20_aqua.get_update_id()": number,
    "u8 ds18b20_aqua.get_updated_deciseconds_ago()": number,
    "u8 ds18b20_case.get_crc_errors()": number,
    "u8 ds18b20_case.get_disconnects()": number,
    "u16 ds18b20_case.get_temperatureX16()": number,
    "u8 ds18b20_case.get_update_id()": number,
    "u8 ds18b20_case.get_updated_deciseconds_ago()": number,
    "u8 co2.get_rx_overflow_count()": number,
    "u8 co2.get_crc_errors()": number,
    "u16 co2.get_abc_setups()": number,
    "u16 co2.get_raw_concentration()": number,
    "u16 co2.get_clamped_concentration()": number,
    "u16 co2.get_concentration()": number,
    "u8 co2.get_temperature()": number,
    "u8 co2.get_s()": number,
    "u16 co2.get_u()": number,
    "u8 co2.get_update_id()": number,
    "u8 co2.get_updated_deciseconds_ago()": number,
    "u8 co2_switch.is_set() ? 1 : 0": number,
    "u8 day_light_switch.is_set() ? 1 : 0": number,
    "u8 night_light_switch.is_set() ? 1 : 0": number,
    "u8 day_light_forced.is_set() ? 1 : 0": number,
    "u8 night_light_forced.is_set() ? 1 : 0": number,
    "u8 light_forces_since_protection_stat_reset": number,
}

export function asAvrData(vals: {[id: string]: number}): AvrData { return {
    "u32 uptime_deciseconds": vals["A1"],
    "u8 debug_overflow_count": vals["A2"],
    "u8 usart0_rx_overflow_count": vals["A3"],
    "u32 main_loop_iterations_in_last_decisecond": vals["A4"],
    "u32 ((u32)last_drift_of_clock_deciseconds_since_midnight)": vals["A5"],
    "u32 clock_corrections_since_protection_stat_reset": vals["A6"],
    "u32 clock_deciseconds_since_midnight": vals["A7"],
    "u8 ds18b20_aqua.get_crc_errors()": vals["B1"],
    "u8 ds18b20_aqua.get_disconnects()": vals["B2"],
    "u16 ds18b20_aqua.get_temperatureX16()": vals["B3"],
    "u8 ds18b20_aqua.get_update_id()": vals["B4"],
    "u8 ds18b20_aqua.get_updated_deciseconds_ago()": vals["B5"],
    "u8 ds18b20_case.get_crc_errors()": vals["C1"],
    "u8 ds18b20_case.get_disconnects()": vals["C2"],
    "u16 ds18b20_case.get_temperatureX16()": vals["C3"],
    "u8 ds18b20_case.get_update_id()": vals["C4"],
    "u8 ds18b20_case.get_updated_deciseconds_ago()": vals["C5"],
    "u8 co2.get_rx_overflow_count()": vals["D1"],
    "u8 co2.get_crc_errors()": vals["D2"],
    "u16 co2.get_abc_setups()": vals["D3"],
    "u16 co2.get_raw_concentration()": vals["D4"],
    "u16 co2.get_clamped_concentration()": vals["D5"],
    "u16 co2.get_concentration()": vals["D6"],
    "u8 co2.get_temperature()": vals["D7"],
    "u8 co2.get_s()": vals["D8"],
    "u16 co2.get_u()": vals["D9"],
    "u8 co2.get_update_id()": vals["D10"],
    "u8 co2.get_updated_deciseconds_ago()": vals["D11"],
    "u8 co2_switch.is_set() ? 1 : 0": vals["D12"],
    "u8 day_light_switch.is_set() ? 1 : 0": vals["E1"],
    "u8 night_light_switch.is_set() ? 1 : 0": vals["E2"],
    "u8 day_light_forced.is_set() ? 1 : 0": vals["E3"],
    "u8 night_light_forced.is_set() ? 1 : 0": vals["E4"],
    "u8 light_forces_since_protection_stat_reset": vals["E5"],
};}


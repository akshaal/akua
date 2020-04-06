// This file is auto-generated by src/avr/maintain-protocol script! DON'T EDIT!

export const avrProtocolVersion = 0xc8;

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
    "u8 co2_switch.is_set() ? 1 : 0": number,
    "u8 co2_calculated_day ? 1 : 0": number,
    "u8 co2_force_off.is_set() ? 1 : 0": number,
    "u8 required_co2_switch_state.is_set() ? 1 : 0": number,
    "u32 co2_deciseconds_until_can_turn_on": number,
    "u8 day_light_switch.is_set() ? 1 : 0": number,
    "u8 night_light_switch.is_set() ? 1 : 0": number,
    "u8 day_light_forced.is_set() ? 1 : 0": number,
    "u8 night_light_forced.is_set() ? 1 : 0": number,
    "u8 light_forces_since_protection_stat_reset": number,
    "u32 __ph_adc_accum": number,
    "u16 __ph_adc_accum_samples": number,
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
    "u8 co2_switch.is_set() ? 1 : 0": vals["D1"],
    "u8 co2_calculated_day ? 1 : 0": vals["D2"],
    "u8 co2_force_off.is_set() ? 1 : 0": vals["D3"],
    "u8 required_co2_switch_state.is_set() ? 1 : 0": vals["D4"],
    "u32 co2_deciseconds_until_can_turn_on": vals["D5"],
    "u8 day_light_switch.is_set() ? 1 : 0": vals["E1"],
    "u8 night_light_switch.is_set() ? 1 : 0": vals["E2"],
    "u8 day_light_forced.is_set() ? 1 : 0": vals["E3"],
    "u8 night_light_forced.is_set() ? 1 : 0": vals["E4"],
    "u8 light_forces_since_protection_stat_reset": vals["E5"],
    "u32 __ph_adc_accum": vals["F1"],
    "u16 __ph_adc_accum_samples": vals["F2"],
};}


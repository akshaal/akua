export function calcCo2DivKhFromPh(ph: number): number {
    return 3.0 * (10 ** (7.00 - ph));
}

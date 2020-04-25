// Run given function every number of milliseconds
export function recurrent(milliseconds: number, f: () => void): void {
    function scheduleAndRun(): void {
        setTimeout(scheduleAndRun, milliseconds);
        f();
    }

    scheduleAndRun();
}

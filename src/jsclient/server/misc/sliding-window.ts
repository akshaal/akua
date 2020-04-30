export default function slidingWindow<T>(inputArray: ReadonlyArray<T>, size: number): T[][] {
    return Array.from(
        { length: inputArray.length - (size - 1) },
        (_, index) => inputArray.slice(index, index + size)
    )
}
